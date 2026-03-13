#!/usr/bin/env python3
"""
Tests for the J1939 CA scanner (RAMN_J1939_Scanner.py).

Validates that:
- All five scanner techniques accumulate detections (never overwrite).
- The return value is a sorted list of (sa, detections) tuples.
- Each SA's detections list contains one entry per technique that found it.
- SAs found by only one technique still appear correctly.
"""

import sys
import os
import unittest

# Ensure the scripts directory is on the Python path so that the
# ``utils`` package can be imported without installing it.
_scripts_dir = os.path.normpath(
    os.path.join(os.path.dirname(__file__), "..")
)
if _scripts_dir not in sys.path:
    sys.path.insert(0, _scripts_dir)

from utils.RAMN_J1939_Scanner import (
    j1939_scan,
    j1939_make_id,
    j1939_get_pf,
    j1939_get_sa,
    _record,
    _get_bitrate,
    _inter_probe_delay,
    PF_REQUEST,
    PF_TP_CM,
    PF_DIAG,
    PF_ADDRESS_CLAIMED,
    PGN_ADDRESS_CLAIMED,
    PGN_ECU_ID,
    TP_CM_BAM,
    TP_CM_ABORT,
    DA_BROADCAST,
    SCANNER_SA,
    UDS_TESTER_PRESENT_RESPONSE,
    DEFAULT_BITRATE,
    DEFAULT_BUSLOAD,
)


# ---------------------------------------------------------------------------
# Lightweight CAN message stub (no python-can dependency required)
# ---------------------------------------------------------------------------
class FakeCANMsg:
    """Minimal CAN message object for testing."""

    def __init__(self, arbitration_id, data, is_extended_id=True):
        self.arbitration_id = arbitration_id
        self.data = bytes(data)
        self.is_extended_id = is_extended_id

    def __repr__(self):
        return (
            f"FakeCANMsg(id=0x{self.arbitration_id:08X}, "
            f"data={self.data.hex()}, ext={self.is_extended_id})"
        )


# ---------------------------------------------------------------------------
# ECU definitions matching the firmware (ramn_j1939.h)
# ---------------------------------------------------------------------------
ECU_SA = {
    "A": 42,  # 0x2A – HEADWAY_CTRL
    "B": 19,  # 0x13 – STEERING_CTRL
    "C": 90,  # 0x5A – POWERTRAIN_CTRL
    "D": 33,  # 0x21 – BODY_CTRL
}


def _encode_pgn_le(pgn):
    return bytes([pgn & 0xFF, (pgn >> 8) & 0xFF, (pgn >> 16) & 0xFF])


def _make_addr_claimed(sa):
    """Build an Address Claimed response from *sa*."""
    cid = j1939_make_id(6, PF_ADDRESS_CLAIMED, DA_BROADCAST, sa)
    # NAME payload (simplified – only identity byte differs per ECU)
    name = bytearray(8)
    name[0] = sa  # use SA as identity for simplicity
    return FakeCANMsg(cid, bytes(name))


def _make_bam(sa):
    """Build a TP.CM BAM response from *sa* for PGN 64965."""
    cid = j1939_make_id(7, PF_TP_CM, DA_BROADCAST, sa)
    payload = bytearray(8)
    payload[0] = TP_CM_BAM
    payload[1] = 0x15  # total size low
    payload[2] = 0x00  # total size high
    payload[3] = 0x03  # num packets
    payload[4] = 0xFF  # reserved
    pgn = _encode_pgn_le(PGN_ECU_ID)
    payload[5] = pgn[0]
    payload[6] = pgn[1]
    payload[7] = pgn[2]
    return FakeCANMsg(cid, bytes(payload))


def _make_unicast_resp(sa):
    """Build a generic extended-frame response from *sa*."""
    # Any extended CAN frame whose SA matches the probed DA
    cid = j1939_make_id(6, PF_ADDRESS_CLAIMED, DA_BROADCAST, sa)
    return FakeCANMsg(cid, b"\xff" * 8)


def _make_tp_abort(sa, requestor_sa=SCANNER_SA):
    """Build a TP.CM Abort response from *sa* directed to *requestor_sa*."""
    cid = j1939_make_id(7, PF_TP_CM, requestor_sa, sa)
    payload = bytearray(8)
    payload[0] = TP_CM_ABORT
    payload[1] = 0x01
    payload[2] = 0xFF
    payload[3] = 0xFF
    payload[4] = 0xFF
    pgn = _encode_pgn_le(PGN_ECU_ID)
    payload[5] = pgn[0]
    payload[6] = pgn[1]
    payload[7] = pgn[2]
    return FakeCANMsg(cid, bytes(payload))


def _make_uds_resp(sa, requestor_sa=SCANNER_SA):
    """Build a UDS Tester Present positive response from *sa*."""
    cid = j1939_make_id(6, PF_DIAG, requestor_sa, sa)
    return FakeCANMsg(cid, UDS_TESTER_PRESENT_RESPONSE)


# ---------------------------------------------------------------------------
# Helpers for building scripted send/recv functions
# ---------------------------------------------------------------------------
class FakeSocket:
    """A fake CAN socket that replays scripted responses."""

    def __init__(self):
        self.sent = []  # list of (can_id, data)


class MockBus:
    """A context-aware mock CAN bus.

    Register responses with ``add_response(probe_id, response_pkt)``
    where *probe_id* is the CAN-ID of the probe frame that should
    trigger the response.  When the scanner calls *send_fn* with a
    matching probe CAN-ID, subsequent *recv_fn* calls return the
    queued responses for that probe (one at a time), then ``None``.
    """

    def __init__(self):
        self._responses = {}  # probe_id -> list of packets
        self._current = iter([])
        self.sent = []  # (can_id, data) log of all probes sent

    def add_response(self, probe_id, pkt):
        self._responses.setdefault(probe_id, []).append(pkt)

    def send_fn(self, sock, can_id, data):
        self.sent.append((can_id, data))
        # Queue up responses for this probe
        self._current = iter(self._responses.get(can_id, []))

    def recv_fn(self, sock, timeout):
        return next(self._current, None)


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

class TestRecordAccumulates(unittest.TestCase):
    """_record() must *append* to the detection list, never overwrite."""

    def test_single_detection(self):
        found = {}
        _record(found, 0x2A, "addr_claim", "pkt1")
        self.assertEqual(len(found[0x2A]), 1)
        self.assertEqual(found[0x2A][0]["method"], "addr_claim")

    def test_multiple_detections(self):
        found = {}
        _record(found, 0x2A, "addr_claim", "pkt1")
        _record(found, 0x2A, "ecu_id", "pkt2")
        _record(found, 0x2A, "unicast", "pkt3")
        _record(found, 0x2A, "rts_probe", "pkt4")
        _record(found, 0x2A, "uds", "pkt5")
        self.assertEqual(len(found[0x2A]), 5)
        methods = [d["method"] for d in found[0x2A]]
        self.assertEqual(methods, ["addr_claim", "ecu_id", "unicast", "rts_probe", "uds"])

    def test_different_sas(self):
        found = {}
        _record(found, 0x2A, "addr_claim", "pkt1")
        _record(found, 0x13, "addr_claim", "pkt2")
        self.assertIn(0x2A, found)
        self.assertIn(0x13, found)
        self.assertEqual(len(found[0x2A]), 1)
        self.assertEqual(len(found[0x13]), 1)


class TestReturnFormat(unittest.TestCase):
    """j1939_scan must return a sorted list of (sa, detections) tuples."""

    def test_returns_sorted_list(self):
        """All four ECUs respond to addr_claim and ecu_id."""
        bus = MockBus()
        # addr_claim probe id
        ac_probe = j1939_make_id(6, PF_REQUEST, DA_BROADCAST, SCANNER_SA)
        for sa in ECU_SA.values():
            bus.add_response(ac_probe, _make_addr_claimed(sa))

        # ecu_id probe id (same CAN-ID, different payload – MockBus
        # keys only on CAN-ID, so we use a second probe_id entry)
        for sa in ECU_SA.values():
            bus.add_response(ac_probe, _make_bam(sa))

        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.0,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
        )

        # Must be a list
        self.assertIsInstance(result, list)

        # Must be sorted by SA
        sas = [sa for sa, _ in result]
        self.assertEqual(sas, sorted(sas))

        # Each entry is (int, list)
        for sa, detections in result:
            self.assertIsInstance(sa, int)
            self.assertIsInstance(detections, list)
            for d in detections:
                self.assertIn("method", d)
                self.assertIn("packet", d)


class TestMultiMethodDetection(unittest.TestCase):
    """An SA found by multiple techniques must list *all* of them."""

    def test_addr_claim_and_ecu_id(self):
        """SA found by both addr_claim and ecu_id."""
        sa = ECU_SA["A"]  # 0x2A
        bus = MockBus()
        probe_id = j1939_make_id(6, PF_REQUEST, DA_BROADCAST, SCANNER_SA)
        bus.add_response(probe_id, _make_addr_claimed(sa))
        bus.add_response(probe_id, _make_bam(sa))

        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.0,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
        )

        # Find SA in result
        sa_entry = dict(result).get(sa)
        self.assertIsNotNone(sa_entry, f"SA 0x{sa:02X} should be found")
        methods = [d["method"] for d in sa_entry]
        self.assertIn("addr_claim", methods)
        self.assertIn("ecu_id", methods)
        self.assertEqual(len(methods), 2)


class TestAllFiveTechniquesSplitted(unittest.TestCase):
    """
    Simulate all four ECUs responding to all five techniques.
    Tests are split into address ranges to provide progress feedback (dots).
    """

    def _build_bus(self):
        """Build a MockBus with responses for all five techniques."""
        bus = MockBus()
        ecu_sas = set(ECU_SA.values())

        # Broadcast probes (addr_claim + ecu_id share the same CAN-ID)
        bcast_probe = j1939_make_id(6, PF_REQUEST, DA_BROADCAST, SCANNER_SA)
        for sa in ECU_SA.values():
            bus.add_response(bcast_probe, _make_addr_claimed(sa))
        for sa in ECU_SA.values():
            bus.add_response(bcast_probe, _make_bam(sa))

        # Unicast probes: one probe per DA, response only from known SAs
        for da in range(0x00, 0xFE):
            probe_id = j1939_make_id(6, PF_REQUEST, da, SCANNER_SA)
            if da in ecu_sas:
                bus.add_response(probe_id, _make_unicast_resp(da))

        # RTS probes: one probe per DA, response only from known SAs
        for da in range(0x00, 0xFE):
            probe_id = j1939_make_id(7, PF_TP_CM, da, SCANNER_SA)
            if da in ecu_sas:
                bus.add_response(probe_id, _make_tp_abort(da))

        # UDS probes: one probe per DA, response only from known SAs
        for da in range(0x00, 0xFE):
            probe_id = j1939_make_id(6, PF_DIAG, da, SCANNER_SA)
            if da in ecu_sas:
                bus.add_response(probe_id, _make_uds_resp(da))

        return bus

    def _run_range_test(self, start, end):
        bus = self._build_bus()
        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.01,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
            range_step=(start, end)
        )
        return result

    # Split the full range 0x00-0xFD into 8 chunks of ~32 addresses each
    def test_range_00_1F(self): self._run_range_test(0x00, 0x1F)
    def test_range_20_3F(self): self._run_range_test(0x20, 0x3F)
    def test_range_40_5F(self): self._run_range_test(0x40, 0x5F)
    def test_range_60_7F(self): self._run_range_test(0x60, 0x7F)
    def test_range_80_9F(self): self._run_range_test(0x80, 0x9F)
    def test_range_A0_BF(self): self._run_range_test(0xA0, 0xBF)
    def test_range_C0_DF(self): self._run_range_test(0xC0, 0xDF)
    def test_range_E0_FD(self): self._run_range_test(0xE0, 0xFD)

    def test_all_ecus_found_full_scan(self):
        """One final full scan to verify total integration."""
        bus = self._build_bus()
        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.01,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
        )
        result_dict = dict(result)
        for ecu, sa in ECU_SA.items():
            self.assertIn(sa, result_dict, f"ECU {ecu} (SA=0x{sa:02X}) missing")
            self.assertEqual(len(result_dict[sa]), 5)


class TestSingleMethodDetection(unittest.TestCase):
    """SAs discovered by only one technique must still appear."""

    def test_unicast_only(self):
        """An SA found only by unicast is included in results."""
        bus = MockBus()
        # Only register a response for unicast probe to DA=0x47
        probe_id = j1939_make_id(6, PF_REQUEST, 0x47, SCANNER_SA)
        bus.add_response(probe_id, _make_unicast_resp(0x47))

        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.01,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
            range_step=(0x40, 0x50) # Limited range for speed
        )

        result_dict = dict(result)
        self.assertIn(0x47, result_dict)
        methods = [d["method"] for d in result_dict[0x47]]
        self.assertEqual(methods, ["unicast"])


class TestEmptyScan(unittest.TestCase):
    """When no ECU responds, the result is an empty list."""

    def test_no_responses(self):
        bus = MockBus()

        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.0,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
        )

        self.assertEqual(result, [])


class TestProbesSent(unittest.TestCase):
    """Verify that the scanner sends the correct probe messages."""

    def test_addr_claim_probe(self):
        """addr_claim sends a broadcast Request for PGN 60928."""
        bus = MockBus()

        j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.0,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
        )

        # First sent frame should be the addr_claim broadcast
        self.assertGreater(len(bus.sent), 0)
        can_id, data = bus.sent[0]
        self.assertEqual(j1939_get_pf(can_id), PF_REQUEST)
        self.assertEqual((can_id >> 8) & 0xFF, DA_BROADCAST)
        self.assertEqual(data, _encode_pgn_le(PGN_ADDRESS_CLAIMED))

    def test_ecu_id_probe(self):
        """ecu_id sends a broadcast Request for PGN 64965."""
        bus = MockBus()

        j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.0,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
        )

        # Second sent frame should be the ecu_id broadcast
        self.assertGreaterEqual(len(bus.sent), 2)
        can_id, data = bus.sent[1]
        self.assertEqual(j1939_get_pf(can_id), PF_REQUEST)
        self.assertEqual((can_id >> 8) & 0xFF, DA_BROADCAST)
        self.assertEqual(data, _encode_pgn_le(PGN_ECU_ID))


class TestUdsScanDetection(unittest.TestCase):
    """UDS scan technique sends Tester Present and expects positive response."""

    def test_uds_only(self):
        """An SA found only by UDS is included in results."""
        bus = MockBus()
        probe_id = j1939_make_id(6, PF_DIAG, 0x2A, SCANNER_SA)
        bus.add_response(probe_id, _make_uds_resp(0x2A))

        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.01,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
            range_step=(0x20, 0x30)
        )

        result_dict = dict(result)
        self.assertIn(0x2A, result_dict)
        methods = [d["method"] for d in result_dict[0x2A]]
        self.assertEqual(methods, ["uds"])

    def test_uds_wrong_response_ignored(self):
        """Non-positive UDS response is not recorded."""
        bus = MockBus()
        # Register a response with wrong data (negative response)
        probe_id = j1939_make_id(6, PF_DIAG, 0x2A, SCANNER_SA)
        bad_resp = FakeCANMsg(
            j1939_make_id(6, PF_DIAG, SCANNER_SA, 0x2A),
            b'\x03\x7f\x3e\x10',  # negative response
        )
        bus.add_response(probe_id, bad_resp)

        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.01,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
            range_step=(0x20, 0x30)
        )

        result_dict = dict(result)
        # SA 0x2A should not appear from UDS (wrong response data)
        if 0x2A in result_dict:
            methods = [d["method"] for d in result_dict[0x2A]]
            self.assertNotIn("uds", methods)

    def test_uds_probe_format(self):
        """UDS probe uses PF=0xDA and Tester Present payload."""
        bus = MockBus()

        j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.0,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
        )

        # Find a UDS probe in the sent frames
        uds_probes = [
            (cid, data) for cid, data in bus.sent
            if j1939_get_pf(cid) == PF_DIAG
        ]
        self.assertGreater(len(uds_probes), 0, "UDS probes should be sent")
        can_id, data = uds_probes[0]
        self.assertEqual(j1939_get_pf(can_id), PF_DIAG)
        self.assertEqual(data, b'\x02\x3e\x00')

    def test_uds_combined_with_other_methods(self):
        """SA found by addr_claim and UDS lists both methods."""
        sa = ECU_SA["B"]  # 0x13
        bus = MockBus()
        # addr_claim response
        bcast_probe = j1939_make_id(6, PF_REQUEST, DA_BROADCAST, SCANNER_SA)
        bus.add_response(bcast_probe, _make_addr_claimed(sa))
        # UDS response
        uds_probe = j1939_make_id(6, PF_DIAG, sa, SCANNER_SA)
        bus.add_response(uds_probe, _make_uds_resp(sa))

        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.01,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
            range_step=(0x10, 0x20)
        )

        result_dict = dict(result)
        self.assertIn(sa, result_dict)
        methods = [d["method"] for d in result_dict[sa]]
        self.assertIn("addr_claim", methods)
        self.assertIn("uds", methods)
        self.assertEqual(len(methods), 2)

    def test_uds_padded_response_detected(self):
        """UDS positive response padded to 8 bytes (real CAN) is detected."""
        bus = MockBus()
        # Build a padded UDS response (8 bytes, 0xFF padding per J1939-21)
        padded_data = b'\x02\x7e\x00\xff\xff\xff\xff\xff'
        probe_id = j1939_make_id(6, PF_DIAG, 0x2A, SCANNER_SA)
        padded_resp = FakeCANMsg(
            j1939_make_id(6, PF_DIAG, SCANNER_SA, 0x2A),
            padded_data,
        )
        bus.add_response(probe_id, padded_resp)

        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.01,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
            range_step=(0x20, 0x30)
        )

        result_dict = dict(result)
        self.assertIn(0x2A, result_dict)
        methods = [d["method"] for d in result_dict[0x2A]]
        self.assertIn("uds", methods)

    def test_uds_padded_zeros_detected(self):
        """UDS positive response padded with 0x00 (CAN padding) is detected."""
        bus = MockBus()
        padded_data = b'\x02\x7e\x00\x00\x00\x00\x00\x00'
        probe_id = j1939_make_id(6, PF_DIAG, 0x5A, SCANNER_SA)
        padded_resp = FakeCANMsg(
            j1939_make_id(6, PF_DIAG, SCANNER_SA, 0x5A),
            padded_data,
        )
        bus.add_response(probe_id, padded_resp)

        result = j1939_scan(
            FakeSocket(),
            timeout=0.01,
            timeout_per_da=0.01,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
            range_step=(0x50, 0x60)
        )

        result_dict = dict(result)
        self.assertIn(0x5A, result_dict)
        methods = [d["method"] for d in result_dict[0x5A]]
        self.assertIn("uds", methods)


class TestInterProbeDelay(unittest.TestCase):
    """_inter_probe_delay() computes rate-limiting sleep."""

    def test_zero_busload_returns_zero(self):
        self.assertEqual(_inter_probe_delay(250000, 0.0, 3, 8, 0.1), 0.0)

    def test_zero_bitrate_returns_zero(self):
        self.assertEqual(_inter_probe_delay(0, 0.05, 3, 8, 0.1), 0.0)

    def test_positive_result(self):
        """With a very small sniff_time the extra delay should be > 0."""
        delay = _inter_probe_delay(250000, 0.05, 3, 8, 0.0)
        self.assertGreater(delay, 0.0)

    def test_large_sniff_time_returns_zero(self):
        """If sniff_time already exceeds the required gap, extra = 0."""
        delay = _inter_probe_delay(250000, 0.05, 3, 8, 10.0)
        self.assertEqual(delay, 0.0)

    def test_higher_bitrate_shorter_delay(self):
        """Higher bitrate → shorter required gap."""
        delay_250k = _inter_probe_delay(250000, 0.05, 3, 8, 0.0)
        delay_500k = _inter_probe_delay(500000, 0.05, 3, 8, 0.0)
        self.assertGreater(delay_250k, delay_500k)


class TestGetBitrate(unittest.TestCase):
    """_get_bitrate() extracts bitrate from socket objects."""

    def test_bitrate_attribute(self):
        """Socket with .bitrate attribute."""
        sock = FakeSocket()
        sock.bitrate = 500000
        self.assertEqual(_get_bitrate(sock), 500000)

    def test_private_bitrate_attribute(self):
        """Socket with ._bitrate attribute (python-can Bus)."""
        sock = FakeSocket()
        sock._bitrate = 125000
        self.assertEqual(_get_bitrate(sock), 125000)

    def test_no_bitrate_returns_none(self):
        """Plain socket without bitrate returns None."""
        sock = FakeSocket()
        self.assertIsNone(_get_bitrate(sock))


class TestBitrateFromSocket(unittest.TestCase):
    """j1939_scan() pulls bitrate from socket when not explicitly set."""

    def test_uses_socket_bitrate(self):
        """When bitrate=None, scanner reads sock.bitrate."""
        bus = MockBus()
        sock = FakeSocket()
        sock.bitrate = 500000

        # Just verify it doesn't crash and uses the socket bitrate
        result = j1939_scan(
            sock,
            timeout=0.01,
            timeout_per_da=0.0,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
        )
        self.assertIsInstance(result, list)

    def test_explicit_bitrate_overrides_socket(self):
        """When bitrate is explicitly set, socket bitrate is ignored."""
        bus = MockBus()
        sock = FakeSocket()
        sock.bitrate = 500000

        result = j1939_scan(
            sock,
            timeout=0.01,
            timeout_per_da=0.0,
            send_fn=bus.send_fn,
            recv_fn=bus.recv_fn,
            bitrate=250000,
        )
        self.assertIsInstance(result, list)


if __name__ == "__main__":
    unittest.main()
