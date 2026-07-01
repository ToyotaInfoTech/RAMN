import unittest
from ramn_firmware_bus import RAMNFirmwareBus

# ---------------------------------------------------------------------------
# Standard mode constants (matching ramn_vehicle_specific.h mocks)
# ---------------------------------------------------------------------------
UDS_PHYS_RESP_IDS = {'A': 0x7E8, 'B': 0x7E9, 'C': 0x7EA, 'D': 0x7EB}
UDS_FUNC_REQ_ID = 0x7DF

ECUS = ['A', 'B', 'C', 'D']


def reassemble_isotp(frames):
    """Reassemble a list of classic ISO-TP TX frames ({'data': bytes,...}) into the payload."""
    if not frames:
        return b''
    first = frames[0]['data']
    pci_type = (first[0] & 0xF0) >> 4
    if pci_type == 0x0:  # SingleFrame
        length = first[0] & 0x0F
        return bytes(first[1:1 + length])
    if pci_type == 0x1:  # FirstFrame + ConsecutiveFrames
        length = ((first[0] & 0x0F) << 8) | first[1]
        payload = bytearray(first[2:])
        for cf in frames[1:]:
            payload += cf['data'][1:]  # strip the CF PCI byte
        return bytes(payload[:length])
    raise AssertionError(f"Unexpected first frame PCI 0x{first[0]:02X}")


class TestFunctionalAddressing(unittest.TestCase):
    """
    Exercises the functional (0x7DF) UDS improvements:
      - Item A: CAN-FD escape SingleFrame requests (> 7 bytes) are accepted.
      - Item B: functional requests reach the full physical service dispatcher and
                can produce multi-frame responses on the ECU's physical response ID.
    """

    def test_functional_reaches_full_dispatcher_single_frame(self):
        """Item B: a service outside the old reduced functional set (0x31 RoutineControl)
        is now dispatched and returns a positive echo, not serviceNotSupported."""
        for letter in ECUS:
            bus = RAMNFirmwareBus(letter, mode='std')
            # 0x31 RoutineControl, subfunction 0x01 start, routine 0x0204 "echo 4 bytes"
            data = [0x04, 0x31, 0x01, 0x02, 0x04]
            responses = bus.process_msg(UDS_FUNC_REQ_ID, data, is_extended=False)
            self.assertEqual(len(responses), 1, f"ECU {letter} did not respond")
            self.assertEqual(responses[0]['id'], UDS_PHYS_RESP_IDS[letter])
            payload = reassemble_isotp(responses)
            self.assertEqual(payload[0], 0x71, f"ECU {letter} expected positive echo, got {payload.hex()}")
            self.assertEqual(payload, bytes([0x71, 0x01, 0x02, 0x04]))

    def test_functional_fd_escape_long_request_multiframe_response(self):
        """Item A + B: a CAN-FD escape SingleFrame carrying a > 7-byte request is accepted,
        dispatched functionally, and answered with a correct multi-frame response."""
        for letter in ECUS:
            bus = RAMNFirmwareBus(letter, mode='std')
            # RoutineControl 0x31 / start 0x01 / routine 0x0203 "echo full message" (12-byte request)
            request = [0x31, 0x01, 0x02, 0x03, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22]
            # CAN-FD escape SingleFrame: data[0]=0x00, data[1]=length, payload in data[2..]
            frame = [0x00, len(request)] + request
            responses = bus.process_msg(UDS_FUNC_REQ_ID, frame, is_extended=False, is_fd=True)
            self.assertTrue(responses, f"ECU {letter} did not accept the FD escape SingleFrame")
            self.assertEqual(responses[0]['id'], UDS_PHYS_RESP_IDS[letter])
            payload = reassemble_isotp(responses)
            self.assertGreater(len(payload), 7, "expected a multi-frame (> 7 byte) response")
            expected = bytes([0x71] + request[1:])
            self.assertEqual(payload, expected, f"ECU {letter} echo mismatch: {payload.hex()}")

    def test_functional_first_frame_ignored(self):
        """Item A negative: a functional FirstFrame (multi-frame request) must be ignored."""
        for letter in ECUS:
            bus = RAMNFirmwareBus(letter, mode='std')
            data = [0x10, 0x14, 0x31, 0x01, 0x02, 0x03, 0x00, 0x00]  # FirstFrame PCI 0x1X
            responses = bus.process_msg(UDS_FUNC_REQ_ID, data, is_extended=False)
            self.assertEqual(len(responses), 0, f"ECU {letter} must not answer a functional FirstFrame")

    def test_functional_tester_present_no_regression(self):
        """Existing behavior: functional Tester Present yields exactly one physical response."""
        for letter in ECUS:
            bus = RAMNFirmwareBus(letter, mode='std')
            data = [0x02, 0x3E, 0x00]
            responses = bus.process_msg(UDS_FUNC_REQ_ID, data, is_extended=False)
            self.assertEqual(len(responses), 1, f"ECU {letter} regression on functional 0x3E")
            self.assertEqual(responses[0]['id'], UDS_PHYS_RESP_IDS[letter])


if __name__ == '__main__':
    unittest.main()
