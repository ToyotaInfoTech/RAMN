#!/usr/bin/env python3
"""
Verifies the actual C firmware logic for J1939 diagnostic routing.
This uses the ECU-specific shared libraries via RAMNFirmwareBus.
"""

import unittest
from ramn_firmware_bus import RAMNFirmwareBus

# ---------------------------------------------------------------------------
# J1939 protocol constants
# ---------------------------------------------------------------------------
PF_UDS_PHYS     = 0xDA
PF_UDS_FUNC     = 0xDB
PF_PROPA        = 0xEF

ECU_SA_D = 33  # Body Control (ECUD)

def j1939_make_id(prio, pf, da, sa):
    return ((prio & 0x7) << 26) | (pf << 16) | (da << 8) | sa

def j1939_parse_id(can_id):
    prio = (can_id >> 26) & 0x7
    pf = (can_id >> 16) & 0xFF
    da = (can_id >> 8) & 0xFF
    sa = can_id & 0xFF
    return prio, pf, da, sa

class TestJ1939DiagRouting(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bus = RAMNFirmwareBus('D', mode='j1939')

    def test_uds_physical_prio7(self):
        """UDS Physical with Priority 7 should respond with Priority 7."""
        tsa = 0xF9
        req_id = j1939_make_id(7, PF_UDS_PHYS, ECU_SA_D, tsa)
        # Service 0x3E (Tester Present), Subfunction 0x00
        data = [0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = self.bus.process_msg(req_id, data)
        self.assertEqual(len(responses), 1, "No response to UDS physical request")
        
        rx = responses[0]
        prio, pf, da, sa = j1939_parse_id(rx['id'])
        self.assertEqual(prio, 7)
        self.assertEqual(pf, PF_UDS_PHYS)
        self.assertEqual(da, tsa)
        self.assertEqual(sa, ECU_SA_D)
        self.assertEqual(rx['data'][1], 0x7E) # Positive response (0x3E + 0x40)

    def test_uds_functional(self):
        """UDS Functional with DA=0xFF should respond physically."""
        tsa = 0xF1
        req_id = j1939_make_id(6, PF_UDS_FUNC, 0xFF, tsa)
        data = [0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = self.bus.process_msg(req_id, data)
        self.assertEqual(len(responses), 1, "No response to UDS functional request")
        
        rx = responses[0]
        prio, pf, da, sa = j1939_parse_id(rx['id'])
        self.assertEqual(pf, PF_UDS_PHYS) # Response is physical
        self.assertEqual(da, tsa)
        self.assertEqual(sa, ECU_SA_D)

    def test_kwp_propa_f1(self):
        """KWP2000 on PropA (PF 0xEF) with TSA 0xF1."""
        tsa = 0xF1
        req_id = j1939_make_id(6, PF_PROPA, ECU_SA_D, tsa)
        # KWP Tester Present (0x3E 0x01)
        data = [0x02, 0x3E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = self.bus.process_msg(req_id, data)
        self.assertEqual(len(responses), 1, "No response to KWP request on PropA")
        
        rx = responses[0]
        _, pf, da, sa = j1939_parse_id(rx['id'])
        self.assertEqual(pf, PF_PROPA)
        self.assertEqual(da, tsa)
        self.assertEqual(sa, ECU_SA_D)
        self.assertEqual(rx['data'][1], 0x7E) # Positive response

    def test_xcp_propa_3f(self):
        """XCP on PropA (PF 0xEF) with TSA 0x3F."""
        tsa = 0x3F
        req_id = j1939_make_id(3, PF_PROPA, ECU_SA_D, tsa)
        # XCP Connect (0xFF 0x00)
        data = [0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = self.bus.process_msg(req_id, data)
        self.assertEqual(len(responses), 1, "No response to XCP request on PropA")
        
        rx = responses[0]
        prio, pf, da, sa = j1939_parse_id(rx['id'])
        self.assertEqual(prio, 3)
        self.assertEqual(pf, PF_PROPA)
        self.assertEqual(da, tsa)
        self.assertEqual(sa, ECU_SA_D)
        self.assertEqual(rx['data'][0], 0xFF) # Positive response (Connect)

    def test_propa_ignored_tsa(self):
        """PropA (PF 0xEF) with an invalid TSA (e.g., 0x20) should be ignored."""
        tsa = 0x20
        req_id = j1939_make_id(6, PF_PROPA, ECU_SA_D, tsa)
        data = [0x02, 0x3E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = self.bus.process_msg(req_id, data)
        self.assertEqual(len(responses), 0, "Should not have received a response for invalid TSA on PropA")

    def test_xcp_no_self_response(self):
        """ECU should not respond to XCP requests where TSA == its own SA."""
        tsa = ECU_SA_D
        req_id = j1939_make_id(3, PF_PROPA, ECU_SA_D, tsa)
        data = [0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        
        responses = self.bus.process_msg(req_id, data)
        self.assertEqual(len(responses), 0, "ECU responded to its own SA as TSA")

if __name__ == '__main__':
    unittest.main()
