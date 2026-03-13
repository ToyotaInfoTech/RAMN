import unittest
from ramn_firmware_bus import RAMNFirmwareBus

# ---------------------------------------------------------------------------
# Standard mode constants (matching ramn_vehicle_specific.h mocks)
# ---------------------------------------------------------------------------
UDS_PHYS_REQ_IDS = {
    'A': 0x7E0,
    'B': 0x7E1,
    'C': 0x7E2,
    'D': 0x7E3
}

UDS_PHYS_RESP_IDS = {
    'A': 0x7E8,
    'B': 0x7E9,
    'C': 0x7EA,
    'D': 0x7EB
}

UDS_FUNC_REQ_ID = 0x7DF

class TestStdFirmwareRouting(unittest.TestCase):
    """
    Verifies the actual C firmware logic for Standard diagnostic routing.
    This uses the ECU-specific shared libraries in 'std' mode.
    """

    def test_uds_routing_all_ecus(self):
        """Verify UDS physical routing for all ECUs in Standard mode."""
        for letter in ['A', 'B', 'C', 'D']:
            bus = RAMNFirmwareBus(letter, mode='std')
            req_id = UDS_PHYS_REQ_IDS[letter]
            # UDS Tester Present (SF)
            data = [0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
            
            responses = bus.process_msg(req_id, data, is_extended=False)
            self.assertEqual(len(responses), 1, f"ECU {letter} did not respond to UDS physical")
            
            resp = responses[0]
            self.assertEqual(resp['id'], UDS_PHYS_RESP_IDS[letter], f"ECU {letter} response ID mismatch")
            self.assertFalse(resp['is_extended'])

    def test_uds_functional_routing(self):
        """Verify UDS functional routing (broadcast) in Standard mode."""
        for letter in ['A', 'B', 'C', 'D']:
            bus = RAMNFirmwareBus(letter, mode='std')
            req_id = UDS_FUNC_REQ_ID
            data = [0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
            
            responses = bus.process_msg(req_id, data, is_extended=False)
            self.assertEqual(len(responses), 1, f"ECU {letter} did not respond to UDS functional")
            
            resp = responses[0]
            self.assertEqual(resp['id'], UDS_PHYS_RESP_IDS[letter], f"ECU {letter} should respond physically to functional")

if __name__ == '__main__':
    unittest.main()
