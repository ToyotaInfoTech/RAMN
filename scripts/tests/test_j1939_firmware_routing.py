import unittest
from ramn_firmware_bus import RAMNFirmwareBus

# ---------------------------------------------------------------------------
# J1939 protocol constants
# ---------------------------------------------------------------------------
PF_UDS_PHYS     = 0xDA
PF_UDS_FUNC     = 0xDB
PF_PROPA        = 0xEF

ECU_SAS = {
    'A': 42,
    'B': 19,
    'C': 90,
    'D': 33
}

def j1939_make_id(prio, pf, da, sa):
    return ((prio & 0x7) << 26) | (pf << 16) | (da << 8) | sa

def j1939_parse_id(can_id):
    prio = (can_id >> 26) & 0x7
    pf = (can_id >> 16) & 0xFF
    da = (can_id >> 8) & 0xFF
    sa = can_id & 0xFF
    return prio, pf, da, sa

class TestJ1939FirmwareRouting(unittest.TestCase):
    """
    Verifies the actual C firmware logic for J1939 diagnostic routing.
    This uses the ECU-specific shared libraries.
    """

    def test_uds_routing_all_ecus(self):
        """Verify UDS physical routing for all ECUs."""
        tsa = 0xF9
        for letter, ecu_sa in ECU_SAS.items():
            bus = RAMNFirmwareBus(letter, mode='j1939')
            # Priority 7 UDS physical request
            req_id = j1939_make_id(7, PF_UDS_PHYS, ecu_sa, tsa)
            # UDS Tester Present (SF)
            data = [0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
            
            responses = bus.process_msg(req_id, data)
            self.assertEqual(len(responses), 1, f"ECU {letter} did not respond to UDS physical")
            
            resp = responses[0]
            prio, pf, da, sa = j1939_parse_id(resp['id'])
            self.assertEqual(prio, 7, f"ECU {letter} did not match priority")
            self.assertEqual(pf, PF_UDS_PHYS)
            self.assertEqual(da, tsa)
            self.assertEqual(sa, ecu_sa)

    def test_uds_functional_routing(self):
        """Verify UDS functional routing (broadcast)."""
        tsa = 0xF1
        # Test on ECUD
        bus = RAMNFirmwareBus('D', mode='j1939')
        req_id = j1939_make_id(6, PF_UDS_FUNC, 0xFF, tsa)
        data = [0x02, 0x3E, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00] # suppressPosResponse=True
        
        # In our mock, SF TesterPresent might still trigger a callback for the response
        # even if subfunction has suppress bit, but let's check routing.
        responses = bus.process_msg(req_id, data)
        # If the firmware correctly handles functional, it should respond physically.
        if responses:
            resp = responses[0]
            _, pf, da, sa = j1939_parse_id(resp['id'])
            self.assertEqual(pf, PF_UDS_PHYS)
            self.assertEqual(da, tsa)
            self.assertEqual(sa, ECU_SAS['D'])

    def test_kwp_multiplexing(self):
        """Verify KWP2000 multiplexing on PropA (TSA 0xF1-0xFA)."""
        ecu_letter = 'D'
        ecu_sa = ECU_SAS[ecu_letter]
        bus = RAMNFirmwareBus(ecu_letter, mode='j1939')
        
        # Valid KWP TSA
        tsa = 0xF1
        req_id = j1939_make_id(6, PF_PROPA, ecu_sa, tsa)
        data = [0x02, 0x3E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = bus.process_msg(req_id, data)
        self.assertEqual(len(responses), 1, "KWP response failed on PropA")
        
        _, pf, da, sa = j1939_parse_id(responses[0]['id'])
        self.assertEqual(pf, PF_PROPA)
        self.assertEqual(da, tsa)
        self.assertEqual(sa, ecu_sa)
        self.assertEqual(responses[0]['data'][1], 0x7E) # Positive response

    def test_xcp_multiplexing(self):
        """Verify XCP multiplexing on PropA (TSA 0x3F or 0x5A)."""
        ecu_letter = 'D'
        ecu_sa = ECU_SAS[ecu_letter]
        bus = RAMNFirmwareBus(ecu_letter, mode='j1939')
        
        # Valid XCP TSA
        tsa = 0x3F
        req_id = j1939_make_id(3, PF_PROPA, ecu_sa, tsa)
        # XCP Connect
        data = [0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = bus.process_msg(req_id, data)
        self.assertEqual(len(responses), 1, "XCP response failed on PropA")
        
        prio, pf, da, sa = j1939_parse_id(responses[0]['id'])
        self.assertEqual(prio, 3)
        self.assertEqual(pf, PF_PROPA)
        self.assertEqual(da, tsa)
        self.assertEqual(sa, ecu_sa)

    def test_invalid_tsa_ignored(self):
        """Verify that invalid TSAs on PropA are ignored."""
        bus = RAMNFirmwareBus('D', mode='j1939')
        req_id = j1939_make_id(6, PF_PROPA, ECU_SAS['D'], 0x20) # TSA 0x20 is not diagnostic
        data = [0x02, 0x3E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = bus.process_msg(req_id, data)
        self.assertEqual(len(responses), 0, "Firmware responded to invalid TSA on PropA")

    def test_standard_mode_routing(self):
        """Verify routing in standard (non-J1939) mode."""
        # ECUD in standard mode
        bus = RAMNFirmwareBus('D', mode='std')
        
        # Standard UDS ID for ECUD is 0x7E3
        req_id = 0x7E3
        data = [0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = bus.process_msg(req_id, data, is_extended=False)
        self.assertEqual(len(responses), 1)
        self.assertEqual(responses[0]['id'], 0x7E3 + 8)
        self.assertFalse(responses[0]['is_extended'])

if __name__ == '__main__':
    unittest.main()
