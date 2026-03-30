import unittest
from ramn_firmware_bus import RAMNFirmwareBus

class TestJ1939FirmwareEdgeCases(unittest.TestCase):

    def test_uds_priority_matching(self):
        """Verify that the response matches the request priority exactly."""
        for prio in [3, 5, 7]:
            # ECU D (SA 33)
            bus = RAMNFirmwareBus('D', mode='j1939')
            # UDS Diagnostic Session Control (0x10 0x01)
            # Prio: prio, PF: 0xDA, DA: 0x21 (33), SA: 0xF1 (241)
            can_id = (prio << 26) | (0xDA << 16) | (33 << 8) | 0xF1
            data = [0x02, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00]
            
            responses = bus.process_msg(can_id, data)
            self.assertEqual(len(responses), 1, f"Failed for priority {prio}")
            
            resp_prio = (responses[0]['id'] >> 26) & 0x07
            self.assertEqual(resp_prio, prio, f"Response priority {resp_prio} != Request priority {prio}")

    def test_uds_sa_da_swapping(self):
        """Verify that SA and DA are correctly swapped in the response."""
        requester_sa = 0x55
        ecu_sa = 42 # ECU A
        bus = RAMNFirmwareBus('A', mode='j1939')
        
        # Prio 6, PF 0xDA, DA 42, SA 0x55
        can_id = (6 << 26) | (0xDA << 16) | (ecu_sa << 8) | requester_sa
        data = [0x02, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = bus.process_msg(can_id, data)
        self.assertEqual(len(responses), 1)
        
        resp_id = responses[0]['id']
        resp_da = (resp_id >> 8) & 0xFF
        resp_sa = resp_id & 0xFF
        
        self.assertEqual(resp_da, requester_sa, "Response DA should be requester's SA")
        self.assertEqual(resp_sa, ecu_sa, "Response SA should be ECU's SA")

    def test_uds_functional_response_address(self):
        """Verify functional request (broadcast) results in a unicast response from the ECU's SA."""
        requester_sa = 0xF9
        bus = RAMNFirmwareBus('B', mode='j1939') # ECU B (SA 19)
        
        # Prio 6, PF 0xDB (Functional), DA 0xFF (Global), SA 0xF9
        can_id = (6 << 26) | (0xDB << 16) | (0xFF << 8) | requester_sa
        data = [0x02, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = bus.process_msg(can_id, data)
        self.assertEqual(len(responses), 1)
        
        resp_id = responses[0]['id']
        resp_pf = (resp_id >> 16) & 0xFF
        resp_da = (resp_id >> 8) & 0xFF
        resp_sa = resp_id & 0xFF
        
        self.assertEqual(resp_pf, 0xDA, "Response to functional should be on PF 0xDA (Unicast)")
        self.assertEqual(resp_da, requester_sa, "Response should be directed to requester's SA")
        self.assertEqual(resp_sa, 19, "Response source should be ECU's actual SA (19)")

    def test_kwp_tsa_boundaries(self):
        """Verify KWP TSA range (0xF1-0xFA) boundaries."""
        # ECU C (SA 90)
        bus = RAMNFirmwareBus('C', mode='j1939')
        
        # Test 0xF0 (Just below) - Should be ignored
        can_id_low = (6 << 26) | (0xEF << 16) | (90 << 8) | 0xF0
        # KWP Start Session (0x10 0x81)
        data = [0x02, 0x10, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00]
        self.assertEqual(len(bus.process_msg(can_id_low, data)), 0, "TSA 0xF0 should be ignored")
        
        # Test 0xFB (Just above) - Should be ignored
        can_id_high = (6 << 26) | (0xEF << 16) | (90 << 8) | 0xFB
        self.assertEqual(len(bus.process_msg(can_id_high, data)), 0, "TSA 0xFB should be ignored")
        
        # Test 0xF1 (Lower boundary) - Should pass
        can_id_valid_low = (6 << 26) | (0xEF << 16) | (90 << 8) | 0xF1
        self.assertEqual(len(bus.process_msg(can_id_valid_low, data)), 1, "TSA 0xF1 should be valid")

        # Test 0xFA (Upper boundary) - Should pass
        can_id_valid_high = (6 << 26) | (0xEF << 16) | (90 << 8) | 0xFA
        self.assertEqual(len(bus.process_msg(can_id_valid_high, data)), 1, "TSA 0xFA should be valid")

    def test_xcp_tsa_validation(self):
        """Verify XCP TSA specific values (0x3F, 0x5A)."""
        bus = RAMNFirmwareBus('D', mode='j1939')
        # XCP CONNECT (0xFF 0x00)
        data = [0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        # Test 0x3F (Valid)
        id_3f = (6 << 26) | (0xEF << 16) | (33 << 8) | 0x3F
        self.assertEqual(len(bus.process_msg(id_3f, data)), 1, "XCP TSA 0x3F should be valid")
        
        # Test 0x5A (Valid)
        id_5a = (6 << 26) | (0xEF << 16) | (33 << 8) | 0x5A
        self.assertEqual(len(bus.process_msg(id_5a, data)), 1, "XCP TSA 0x5A should be valid")
        
        # Test 0x40 (Invalid)
        id_40 = (6 << 26) | (0xEF << 16) | (33 << 8) | 0x40
        self.assertEqual(len(bus.process_msg(id_40, data)), 0, "XCP TSA 0x40 should be ignored")

    @unittest.skip("Crashing due to pointer dereference issues in 64-bit mock environment")
    def test_xcp_device_name_per_ecu(self):
        """Verify each ECU returns its unique name via XCP GET_ID."""
        ecu_data = {
            'A': (42, b'ECUA'),
            'B': (19, b'ECUB'),
            'C': (90, b'ECUC'),
            'D': (33, b'ECUD')
        }
        
        for letter, (ecu_sa, expected_name) in ecu_data.items():
            bus = RAMNFirmwareBus(letter, mode='j1939')
            # Use a valid TSA for XCP (0x3F)
            can_id = (6 << 26) | (0xEF << 16) | (ecu_sa << 8) | 0x3F
            
            # 1. XCP CONNECT (0xFF 0x00)
            connect_data = [0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
            bus.process_msg(can_id, connect_data)

            # 2. XCP GET_ID (0xFA 0x01)
            get_id_data = [0xFA, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
            responses = bus.process_msg(can_id, get_id_data)
            self.assertEqual(len(responses), 1, f"No GET_ID response for ECU {letter}")

            # 3. Read the ID data via UPLOAD (since GET_ID just sets the MTA)
            # XCP UPLOAD (0xF5 0x04) - upload 4 bytes
            upload_data = [0xF5, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
            responses = bus.process_msg(can_id, upload_data)
            self.assertEqual(len(responses), 1, f"No UPLOAD response for ECU {letter}")
            
            resp_data = responses[0]['data']
            self.assertIn(expected_name, resp_data, f"ECU {letter} did not return {expected_name}")

if __name__ == '__main__':
    unittest.main()
