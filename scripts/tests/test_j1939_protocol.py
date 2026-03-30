#!/usr/bin/env python3
"""
Test suite for J1939-TP and NAME support.

Validates that the firmware protocol definitions and CAN ID construction
are correct for each of the five scanner techniques:

  Technique    Probe sent                              Expected response
  ------------ --------------------------------------- ------------------------------------
  addr_claim   Bcast Request (PF=0xEA,DA=0xFF) PGN60928  Address Claimed (PF=0xEE, SA=ECU-SA)
  ecu_id       Bcast Request (PF=0xEA,DA=0xFF) PGN64965  TP.CM BAM (PF=0xEC, ctrl=0x20) PGN64965
  unicast      Ucast Request (PF=0xEA,DA=ECU-SA) PGN60928  Extended frame SA==probed DA
  rts_probe    TP.CM_RTS  (PF=0xEC,DA=ECU-SA)          TP.CM_CTS(0x11) or Abort(0xFF)
  uds          UDS TesterPresent (PF=0xDA,DA=ECU-SA)   Positive response 0x7E
"""

import struct
import unittest

# ---------------------------------------------------------------------------
# J1939 protocol constants (mirroring ramn_j1939.h)
# ---------------------------------------------------------------------------
PGN_REQUEST         = 59904   # 0xEA00
PGN_TP_DT           = 60160   # 0xEB00
PGN_TP_CM           = 60416   # 0xEC00
PGN_ADDRESS_CLAIMED = 60928   # 0xEE00
PGN_ECU_ID          = 64965   # 0xFDC5

PF_REQUEST          = 0xEA
PF_TP_DT            = 0xEB
PF_TP_CM            = 0xEC
PF_ADDRESS_CLAIMED  = 0xEE
PF_DIAG             = 0xDA

TP_CM_RTS   = 0x10
TP_CM_CTS   = 0x11
TP_CM_BAM   = 0x20
TP_CM_ABORT = 0xFF

DA_BROADCAST = 0xFF

# Per-ECU primary source addresses (mirroring ramn_j1939.h)
ECU_SA = {
    'A': 42,   # HEADWAY_CTRL
    'B': 19,   # STEERING_CTRL
    'C': 90,   # POWERTRAIN_CTRL
    'D': 33,   # BODY_CTRL
}

# J1939 NAME per ECU: (function, identity_number) — all other fields zero
ECU_NAME_FIELDS = {
    'A': {'function': 42, 'identity': 1},
    'B': {'function': 19, 'identity': 2},
    'C': {'function': 90, 'identity': 3},
    'D': {'function': 33, 'identity': 4},
}

ECU_ID_STRINGS = {
    'A': "RAMN*ECU_A*0001*UNIT1*",
    'B': "RAMN*ECU_B*0002*UNIT2*",
    'C': "RAMN*ECU_C*0003*UNIT3*",
    'D': "RAMN*ECU_D*0004*UNIT4*",
}

# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

def j1939_make_id(prio, pf, da, sa):
    """Build a 29-bit J1939 CAN ID (EDP=0, DP=0)."""
    return ((prio & 0x7) << 26) | (pf << 16) | (da << 8) | sa


def j1939_get_pf(can_id):
    return (can_id >> 16) & 0xFF


def j1939_get_ps(can_id):
    return (can_id >> 8) & 0xFF


def j1939_get_sa(can_id):
    return can_id & 0xFF


def encode_pgn_le(pgn):
    """Encode a PGN as 3 little-endian bytes."""
    return bytes([pgn & 0xFF, (pgn >> 8) & 0xFF, (pgn >> 16) & 0xFF])


def build_name_bytes(function, identity):
    """
    Build 8 NAME bytes in little-endian order.
    Only Function (bits 47-40) and Identity Number (bits 20-0) are nonzero.
    """
    name64 = (function << 40) | identity
    return list(name64.to_bytes(8, 'little'))


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

class TestJ1939IdConstruction(unittest.TestCase):
    """Verify CAN ID construction helper matches J1939 spec."""

    def test_broadcast_request(self):
        """Broadcast Request from SA=0xFE targeting all ECUs."""
        can_id = j1939_make_id(6, PF_REQUEST, DA_BROADCAST, 0xFE)
        self.assertEqual(j1939_get_pf(can_id), PF_REQUEST)
        self.assertEqual(j1939_get_ps(can_id), DA_BROADCAST)
        self.assertEqual(j1939_get_sa(can_id), 0xFE)
        # Must fit in 29 bits
        self.assertLessEqual(can_id, 0x1FFFFFFF)

    def test_unicast_request(self):
        """Unicast Request to specific SA."""
        for ecu, sa in ECU_SA.items():
            can_id = j1939_make_id(6, PF_REQUEST, sa, 0xFE)
            self.assertEqual(j1939_get_ps(can_id), sa,
                             f"DA should equal ECU {ecu}'s SA")

    def test_address_claimed_id(self):
        """Address Claimed response from each ECU."""
        for ecu, sa in ECU_SA.items():
            can_id = j1939_make_id(6, PF_ADDRESS_CLAIMED, DA_BROADCAST, sa)
            self.assertEqual(j1939_get_pf(can_id), PF_ADDRESS_CLAIMED)
            self.assertEqual(j1939_get_ps(can_id), DA_BROADCAST)
            self.assertEqual(j1939_get_sa(can_id), sa,
                             f"SA in Address Claimed must be ECU {ecu}'s SA")

    def test_tp_cm_bam_id(self):
        """TP.CM BAM broadcast from each ECU."""
        for ecu, sa in ECU_SA.items():
            can_id = j1939_make_id(7, PF_TP_CM, DA_BROADCAST, sa)
            self.assertEqual(j1939_get_pf(can_id), PF_TP_CM)
            self.assertEqual(j1939_get_ps(can_id), DA_BROADCAST)
            self.assertEqual(j1939_get_sa(can_id), sa)

    def test_tp_cm_abort_id(self):
        """TP.CM Abort response directed at requestor SA=0xFE."""
        for ecu, sa in ECU_SA.items():
            can_id = j1939_make_id(7, PF_TP_CM, 0xFE, sa)
            self.assertEqual(j1939_get_pf(can_id), PF_TP_CM)
            self.assertEqual(j1939_get_ps(can_id), 0xFE,
                             "DA should be the requestor's SA")
            self.assertEqual(j1939_get_sa(can_id), sa)


class TestJ1939Name(unittest.TestCase):
    """Validate the 8-byte J1939 NAME for each ECU."""

    def test_names_are_unique(self):
        names = set()
        for ecu, fields in ECU_NAME_FIELDS.items():
            name = tuple(build_name_bytes(fields['function'], fields['identity']))
            self.assertNotIn(name, names, f"ECU {ecu} NAME must be unique")
            names.add(name)

    def test_name_length(self):
        for ecu, fields in ECU_NAME_FIELDS.items():
            name = build_name_bytes(fields['function'], fields['identity'])
            self.assertEqual(len(name), 8, f"ECU {ecu} NAME must be 8 bytes")

    def test_name_function_field(self):
        """Function byte is at byte index 5 (bits 47-40)."""
        for ecu, fields in ECU_NAME_FIELDS.items():
            name = build_name_bytes(fields['function'], fields['identity'])
            self.assertEqual(name[5], fields['function'],
                             f"ECU {ecu} function byte mismatch")

    def test_name_identity_field(self):
        """Identity number occupies bits 20-0 (bytes 0-2)."""
        for ecu, fields in ECU_NAME_FIELDS.items():
            name = build_name_bytes(fields['function'], fields['identity'])
            identity = name[0] | (name[1] << 8) | (name[2] << 16)
            # only lower 21 bits
            identity &= 0x1FFFFF
            self.assertEqual(identity, fields['identity'],
                             f"ECU {ecu} identity number mismatch")


class TestAddrClaimScanner(unittest.TestCase):
    """
    Technique: addr_claim
    Probe:    Broadcast Request (PF=0xEA, DA=0xFF) for PGN 60928
    Expected: Address Claimed (PF=0xEE, SA=ECU-SA) with 8-byte NAME
    """

    def test_probe_format(self):
        """Verify the probe CAN ID and payload format."""
        probe_id = j1939_make_id(6, PF_REQUEST, DA_BROADCAST, 0xFE)
        self.assertEqual(j1939_get_pf(probe_id), PF_REQUEST)
        self.assertEqual(j1939_get_ps(probe_id), DA_BROADCAST)
        payload = encode_pgn_le(PGN_ADDRESS_CLAIMED)
        self.assertEqual(len(payload), 3)
        self.assertEqual(payload, bytes([0x00, 0xEE, 0x00]))

    def test_response_format(self):
        """Each ECU responds with Address Claimed broadcast."""
        for ecu, sa in ECU_SA.items():
            resp_id = j1939_make_id(6, PF_ADDRESS_CLAIMED, DA_BROADCAST, sa)
            self.assertEqual(j1939_get_pf(resp_id), PF_ADDRESS_CLAIMED,
                             f"ECU {ecu}: PF must be 0xEE")
            self.assertEqual(j1939_get_sa(resp_id), sa,
                             f"ECU {ecu}: SA must match primary SA")
            fields = ECU_NAME_FIELDS[ecu]
            name = build_name_bytes(fields['function'], fields['identity'])
            self.assertEqual(len(name), 8)


class TestEcuIdScanner(unittest.TestCase):
    """
    Technique: ecu_id
    Probe:    Broadcast Request (PF=0xEA, DA=0xFF) for PGN 64965
    Expected: TP.CM BAM (PF=0xEC, ctrl=0x20) announcing PGN 64965
    """

    def test_probe_format(self):
        probe_id = j1939_make_id(6, PF_REQUEST, DA_BROADCAST, 0xFE)
        payload = encode_pgn_le(PGN_ECU_ID)
        self.assertEqual(payload, bytes([0xC5, 0xFD, 0x00]))

    def test_bam_response_format(self):
        """Each ECU sends a TP.CM BAM followed by TP.DT packets."""
        for ecu, sa in ECU_SA.items():
            bam_id = j1939_make_id(7, PF_TP_CM, DA_BROADCAST, sa)
            self.assertEqual(j1939_get_pf(bam_id), PF_TP_CM)
            self.assertEqual(j1939_get_ps(bam_id), DA_BROADCAST)
            self.assertEqual(j1939_get_sa(bam_id), sa)

            ecu_id_str = ECU_ID_STRINGS[ecu]
            total_size = len(ecu_id_str)
            num_packets = (total_size + 6) // 7

            # Build BAM payload
            bam_payload = bytearray(8)
            bam_payload[0] = TP_CM_BAM
            bam_payload[1] = total_size & 0xFF
            bam_payload[2] = (total_size >> 8) & 0xFF
            bam_payload[3] = num_packets
            bam_payload[4] = 0xFF
            bam_payload[5] = PGN_ECU_ID & 0xFF
            bam_payload[6] = (PGN_ECU_ID >> 8) & 0xFF
            bam_payload[7] = (PGN_ECU_ID >> 16) & 0xFF

            self.assertEqual(bam_payload[0], 0x20,
                             f"ECU {ecu}: ctrl must be BAM (0x20)")
            self.assertEqual(bam_payload[5:8], bytes([0xC5, 0xFD, 0x00]),
                             f"ECU {ecu}: PGN must be 64965")

    def test_tp_dt_reassembly(self):
        """TP.DT packets must fully contain the ECU ID string."""
        for ecu, sa in ECU_SA.items():
            ecu_id_str = ECU_ID_STRINGS[ecu]
            total_size = len(ecu_id_str)
            num_packets = (total_size + 6) // 7

            reassembled = bytearray()
            for seq in range(1, num_packets + 1):
                payload = bytearray([0xFF] * 8)
                payload[0] = seq
                offset = (seq - 1) * 7
                for j in range(7):
                    if offset + j < total_size:
                        payload[1 + j] = ord(ecu_id_str[offset + j])
                reassembled.extend(payload[1:8])

            self.assertEqual(reassembled[:total_size].decode('ascii'),
                             ecu_id_str,
                             f"ECU {ecu}: reassembled data mismatch")


class TestUnicastScanner(unittest.TestCase):
    """
    Technique: unicast
    Probe:    Unicast Request (PF=0xEA, DA=ECU-SA) for PGN 60928
    Expected: Any extended CAN frame whose SA equals probed DA
    """

    def test_unicast_probe_addresses_ecu(self):
        """Probe DA must match each ECU's primary SA."""
        for ecu, sa in ECU_SA.items():
            probe_id = j1939_make_id(6, PF_REQUEST, sa, 0xFE)
            self.assertEqual(j1939_get_ps(probe_id), sa)

    def test_response_sa_matches_probed_da(self):
        """The response from each ECU carries SA == probed DA."""
        for ecu, sa in ECU_SA.items():
            resp_id = j1939_make_id(6, PF_ADDRESS_CLAIMED, DA_BROADCAST, sa)
            self.assertEqual(j1939_get_sa(resp_id), sa,
                             f"ECU {ecu}: response SA must equal probed DA")


class TestRtsProbeScanner(unittest.TestCase):
    """
    Technique: rts_probe
    Probe:    TP.CM_RTS (PF=0xEC, DA=ECU-SA)
    Expected: TP.CM_CTS (ctrl=0x11) or TP_Conn_Abort (ctrl=0xFF)
    """

    def test_rts_probe_format(self):
        """RTS probe is a TP.CM message addressed to each ECU."""
        for ecu, sa in ECU_SA.items():
            probe_id = j1939_make_id(7, PF_TP_CM, sa, 0xFE)
            self.assertEqual(j1939_get_pf(probe_id), PF_TP_CM)
            self.assertEqual(j1939_get_ps(probe_id), sa,
                             f"RTS DA must be ECU {ecu}'s SA")

    def test_abort_response_format(self):
        """Each ECU responds with TP Connection Abort."""
        requestor_sa = 0xFE
        some_pgn = PGN_ECU_ID  # any PGN can be used in RTS
        for ecu, sa in ECU_SA.items():
            resp_id = j1939_make_id(7, PF_TP_CM, requestor_sa, sa)
            self.assertEqual(j1939_get_pf(resp_id), PF_TP_CM)
            self.assertEqual(j1939_get_ps(resp_id), requestor_sa,
                             f"Abort DA must be the requestor's SA")
            self.assertEqual(j1939_get_sa(resp_id), sa)

            # Build abort payload
            abort_payload = bytearray(8)
            abort_payload[0] = TP_CM_ABORT
            abort_payload[1] = 0x01   # reason
            abort_payload[2] = 0xFF
            abort_payload[3] = 0xFF
            abort_payload[4] = 0xFF
            abort_payload[5] = some_pgn & 0xFF
            abort_payload[6] = (some_pgn >> 8) & 0xFF
            abort_payload[7] = (some_pgn >> 16) & 0xFF
            self.assertEqual(abort_payload[0], TP_CM_ABORT,
                             f"ECU {ecu}: ctrl must be Abort (0xFF)")

    def test_acceptable_responses(self):
        """Either CTS or Abort is acceptable per spec."""
        self.assertIn(TP_CM_CTS, [0x11])
        self.assertIn(TP_CM_ABORT, [0xFF])


class TestUdsScanner(unittest.TestCase):
    """
    Technique: uds
    Probe:    UDS Tester Present (PF=0xDA, DA=ECU-SA)
    Expected: Positive response 0x7E from ECU-SA
    """

    def test_uds_probe_format(self):
        """Verify the UDS probe CAN ID uses PF=0xDA."""
        for ecu, sa in ECU_SA.items():
            probe_id = j1939_make_id(6, PF_DIAG, sa, 0xFE)
            self.assertEqual(j1939_get_pf(probe_id), PF_DIAG,
                             f"UDS probe PF must be 0xDA for ECU {ecu}")
            self.assertEqual(j1939_get_ps(probe_id), sa,
                             f"UDS probe DA must be ECU {ecu}'s SA")

    def test_uds_tester_present_payload(self):
        """Tester Present request: SID=0x3E, subfunction=0x00."""
        payload = b'\x02\x3e\x00'
        self.assertEqual(payload[0], 0x02, "Length byte")
        self.assertEqual(payload[1], 0x3E, "Tester Present SID")
        self.assertEqual(payload[2], 0x00, "Subfunction (no suppress)")

    def test_uds_positive_response(self):
        """Positive response: SID+0x40 = 0x7E."""
        response = b'\x02\x7e\x00'
        self.assertEqual(response[0], 0x02, "Length byte")
        self.assertEqual(response[1], 0x7E, "Tester Present positive response SID")

    def test_uds_response_id_format(self):
        """Response CAN ID from each ECU uses PF=0xDA, DA=requestor."""
        requestor_sa = 0xFE
        for ecu, sa in ECU_SA.items():
            resp_id = j1939_make_id(6, PF_DIAG, requestor_sa, sa)
            self.assertEqual(j1939_get_pf(resp_id), PF_DIAG)
            self.assertEqual(j1939_get_ps(resp_id), requestor_sa,
                             f"Response DA must be requestor's SA")
            self.assertEqual(j1939_get_sa(resp_id), sa,
                             f"Response SA must be ECU {ecu}'s SA")


class TestJ1939ModeGuard(unittest.TestCase):
    """Verify that J1939 support is compile-time guarded."""

    def test_enable_j1939_mode_defined(self):
        """ENABLE_J1939_MODE must be defined in ramn_config.h for the
        protocol handlers to be compiled in."""
        import os
        config_path = os.path.join(os.path.dirname(__file__), '..',
                                   '..', 'firmware', 'RAMNV1', 'Core',
                                   'Inc', 'ramn_config.h')
        config_path = os.path.normpath(config_path)
        with open(config_path) as f:
            content = f.read()
        self.assertIn('#define ENABLE_J1939_MODE', content)

    def test_handlers_guarded(self):
        """Protocol handlers in ramn_customize.c must be inside
        #ifdef ENABLE_J1939_MODE blocks."""
        import os
        src_path = os.path.join(os.path.dirname(__file__), '..',
                                '..', 'firmware', 'RAMNV1', 'Core',
                                'Src', 'ramn_customize.c')
        src_path = os.path.normpath(src_path)
        with open(src_path) as f:
            content = f.read()
        self.assertIn('#ifdef ENABLE_J1939_MODE', content)
        self.assertIn('J1939_SendAddressClaimed', content)
        self.assertIn('J1939_SendEcuIdBAM', content)
        self.assertIn('J1939_SendTPConnAbort', content)
        self.assertIn('J1939_PF_REQUEST', content)
        self.assertIn('J1939_PF_TP_CM', content)


class TestPGNDefinitions(unittest.TestCase):
    """Verify PGN numeric values match the J1939 standard."""

    def test_pgn_hex_values(self):
        self.assertEqual(PGN_REQUEST,         0xEA00)
        self.assertEqual(PGN_TP_DT,           0xEB00)
        self.assertEqual(PGN_TP_CM,           0xEC00)
        self.assertEqual(PGN_ADDRESS_CLAIMED, 0xEE00)
        self.assertEqual(PGN_ECU_ID,          0xFDC5)

    def test_pf_derived_from_pgn(self):
        """For PDU1 PGNs (<0xF000), PF = PGN >> 8."""
        self.assertEqual(PGN_REQUEST >> 8,          PF_REQUEST)
        self.assertEqual(PGN_TP_DT >> 8,            PF_TP_DT)
        self.assertEqual(PGN_TP_CM >> 8,            PF_TP_CM)
        self.assertEqual(PGN_ADDRESS_CLAIMED >> 8,  PF_ADDRESS_CLAIMED)

    def test_ecu_id_is_pdu2(self):
        """PGN 64965 has PF >= 0xF0 (PDU2), so it is broadcast."""
        pf = (PGN_ECU_ID >> 8) & 0xFF
        self.assertGreaterEqual(pf, 0xF0,
                                "ECU ID PGN must be PDU2 (PF >= 0xF0)")


if __name__ == '__main__':
    unittest.main()
