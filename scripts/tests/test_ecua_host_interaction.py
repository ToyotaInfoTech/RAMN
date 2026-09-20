import unittest
from ramn_firmware_bus import RAMNFirmwareBus

# ---------------------------------------------------------------------------
# Constants matching firmware/RAMNV1/Core/Inc/ramn_canfd.h
# ---------------------------------------------------------------------------
RAMN_CAN_ORIGIN_BUS      = 0x00
RAMN_CAN_ORIGIN_HOST     = 0xA1
RAMN_CAN_ORIGIN_INTERNAL = 0x02

UDS_PHYS_REQ_ID  = 0x7E0
UDS_PHYS_RESP_ID = 0x7E8
UDS_FUNC_REQ_ID  = 0x7DF

J1939_PGN_REQUEST = 59904      # 0xEA00
J1939_PGN_ADDR_CLAIMED = 60928 # 0xEE00
J1939_PGN_COMP_ID = 65259      # 0xFEEB
ECUA_J1939_SA = 42             # Headway Controller (0x2A)


class TestECUAHostInteraction(unittest.TestCase):
    """
    Validates host PC interaction with ECU A over slcan / gs_usb software routing:
    - Host-originated frames (origin=RAMN_CAN_ORIGIN_HOST) reach ECU A's diagnostic
      and custom protocol stacks.
    - Responses from ECU A are marked as internal transmissions (marker != HOST)
      so they are forwarded to the USB host.
    - Anti-storm behavior: local transmissions do not trigger runaway recursive loops.
    """

    def test_uds_physical_host_interaction(self):
        """Host sends UDS Tester Present to ECU A (0x7E0) with ORIGIN_HOST."""
        bus = RAMNFirmwareBus('A', mode='std')
        # UDS Tester Present (SF)
        req_data = [0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

        responses = bus.process_msg(UDS_PHYS_REQ_ID, req_data, is_extended=False, origin=RAMN_CAN_ORIGIN_HOST)
        self.assertEqual(len(responses), 1, "ECU A did not respond to physical UDS request injected by host")

        resp = responses[0]
        self.assertEqual(resp['id'], UDS_PHYS_RESP_ID)
        self.assertFalse(resp['is_extended'])
        # Positive response: 0x02, 0x7E, 0x00...
        self.assertEqual(resp['data'][0], 0x02)
        self.assertEqual(resp['data'][1], 0x7E)
        # MessageMarker must NOT be RAMN_CAN_ORIGIN_HOST so it gets forwarded to USB
        self.assertNotEqual(resp['marker'], RAMN_CAN_ORIGIN_HOST,
                            "Firmware response should not be marked as ORIGIN_HOST")

    def test_uds_functional_host_interaction_all_ecus(self):
        """Host sends functional UDS Tester Present (0x7DF) with ORIGIN_HOST to network."""
        req_data = [0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

        # ECU A processes via software loopback
        bus_a = RAMNFirmwareBus('A', mode='std')
        responses_a = bus_a.process_msg(UDS_FUNC_REQ_ID, req_data, is_extended=False, origin=RAMN_CAN_ORIGIN_HOST)
        self.assertEqual(len(responses_a), 1, "ECU A did not respond to functional request")
        self.assertEqual(responses_a[0]['id'], 0x7E8)
        self.assertNotEqual(responses_a[0]['marker'], RAMN_CAN_ORIGIN_HOST)

        # ECU B, C, D receive from physical bus
        expected_ids = {'B': 0x7E9, 'C': 0x7EA, 'D': 0x7EB}
        for letter, resp_id in expected_ids.items():
            bus = RAMNFirmwareBus(letter, mode='std')
            resps = bus.process_msg(UDS_FUNC_REQ_ID, req_data, is_extended=False, origin=RAMN_CAN_ORIGIN_BUS)
            self.assertEqual(len(resps), 1, f"ECU {letter} did not respond to functional request")
            self.assertEqual(resps[0]['id'], resp_id)

    def test_j1939_address_claim_request_by_host(self):
        """Host sends Request for Address Claimed (PGN 60928) to ECU A (DA=42)."""
        bus = RAMNFirmwareBus('A', mode='j1939')
        # PGN Request: PDU Format 234 (0xEA), PDU Specific DA=42 (0x2A), SA=Host (e.g. 0xF9)
        # Priority 6 -> CAN ID 0x18EA2AF9
        req_id = (6 << 26) | (0xEA << 16) | (ECUA_J1939_SA << 8) | 0xF9
        req_data = [0x00, 0xEE, 0x00]  # PGN 60928 in little-endian

        responses = bus.process_msg(req_id, req_data, is_extended=True, origin=RAMN_CAN_ORIGIN_HOST)
        self.assertTrue(len(responses) >= 1, "ECU A did not respond to J1939 Address Claimed request from host")

        # Response must be Address Claimed (PGN 60928 / 0xEE00) from SA 42
        addr_claim = next((r for r in responses if ((r['id'] >> 16) & 0xFF) == 0xEE), None)
        self.assertIsNotNone(addr_claim, "Address Claimed response missing")
        self.assertEqual(addr_claim['id'] & 0xFF, ECUA_J1939_SA, "Response SA mismatch")
        self.assertNotEqual(addr_claim['marker'], RAMN_CAN_ORIGIN_HOST)

    def test_anti_storm_no_recursive_echo(self):
        """Feeding ECU A's own response back into ECU A must not generate recursive frames."""
        bus = RAMNFirmwareBus('A', mode='std')
        # ECU A's physical response ID
        resp_data = [0x02, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        # Feed response back with INTERNAL origin
        responses = bus.process_msg(UDS_PHYS_RESP_ID, resp_data, is_extended=False, origin=RAMN_CAN_ORIGIN_INTERNAL)
        self.assertEqual(len(responses), 0, "ECU A must not respond to its own response frame (anti-storm violation)")

    def test_host_origin_suppression_tag(self):
        """Verify host origin marker tag distinction."""
        self.assertNotEqual(RAMN_CAN_ORIGIN_HOST, RAMN_CAN_ORIGIN_BUS)
        self.assertNotEqual(RAMN_CAN_ORIGIN_HOST, RAMN_CAN_ORIGIN_INTERNAL)


if __name__ == '__main__':
    unittest.main()
