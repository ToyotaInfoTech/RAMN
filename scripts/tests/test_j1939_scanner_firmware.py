import unittest
import os
import sys

# Add the directory containing RAMN_J1939_Scanner.py to path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'utils'))

from RAMN_J1939_Scanner import j1939_scan
from ramn_firmware_bus import RAMNFirmwareBus

class FirmwareMockBus:
    """
    A mock bus that routes scanner probes to multiple ECU shared libraries
    and collects their actual responses.
    """
    def __init__(self, mode='j1939'):
        self.ecus = {
            letter: RAMNFirmwareBus(letter, mode=mode)
            for letter in ['A', 'B', 'C', 'D']
        }
        self.pending_responses = []

    def send_fn(self, sock, can_id, data, is_extended_id=True):
        # Route the probe to all ECUs (like a real bus)
        for bus in self.ecus.values():
            resps = bus.process_msg(can_id, data, is_extended=is_extended_id)
            for r in resps:
                # Convert to a format j1939_scan expects (object with arbitration_id and data)
                self.pending_responses.append(type('Msg', (), {
                    'arbitration_id': r['id'],
                    'data': r['data'],
                    'is_extended_id': r['is_extended']
                }))

    def recv_fn(self, sock, timeout=0.1):
        if self.pending_responses:
            return self.pending_responses.pop(0)
        return None

class FakeSocket:
    def __init__(self, bitrate=500000):
        self.bitrate = bitrate

class TestScannerWithFirmware(unittest.TestCase):
    """
    Tests the J1939 scanner logic against the actual C firmware diagnostic routing.
    """

    def test_scan_all_ecus_j1939(self):
        """Scanner should find all 4 ECUs in J1939 mode using firmware responses."""
        mock_bus = FirmwareMockBus(mode='j1939')
        sock = FakeSocket()
        
        # We expect j1939_scan to find ECUs by sending UDS Tester Present probes
        # and getting responses from the firmware.
        found = j1939_scan(
            sock,
            timeout=0.1,
            timeout_per_da=0.01,
            send_fn=mock_bus.send_fn,
            recv_fn=mock_bus.recv_fn
        )
        
        # Verify all ECUs were found. found is a list of (sa, detections)
        found_sas = [sa for sa, detections in found]
        expected_sas = [42, 19, 90, 33]
        for sa in expected_sas:
            self.assertIn(sa, found_sas, f"Scanner failed to find ECU with SA {sa}")
        
        self.assertEqual(len(found), 4)

    def test_scan_all_ecus_std(self):
        """Standard mode ECUs respond to 11-bit probes."""
        # Since j1939_scan only looks for J1939 frames, we test the firmware routing 
        # for standard mode directly using our mock bus.
        mock_bus = FirmwareMockBus(mode='std')
        
        expected_ids = [0x7E0, 0x7E1, 0x7E2, 0x7E3]
        for eid in expected_ids:
            # Send standard UDS Tester Present
            mock_bus.send_fn(None, eid, [0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], is_extended_id=False)
            
            # Check for response
            resp = mock_bus.recv_fn(None)
            self.assertIsNotNone(resp, f"No response from standard ECU 0x{eid:03X}")
            self.assertEqual(resp.arbitration_id, eid + 8)
            self.assertFalse(resp.is_extended_id)

if __name__ == '__main__':
    unittest.main()
