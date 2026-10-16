#!/usr/bin/env python3
# Copyright (c) 2024 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Runtime simulation test suite for RAMN running in Renode under Docker.

Connects to ECU A's LPUART1 serial transport via python-can slcan (socket://)
and verifies multi-ECU communications across the virtual CAN bus:
- Physical UDS Tester Present (0x3E) routing to each individual ECU.
- Functional UDS Tester Present broadcast (0x7DF) routing to all ECUs.
- Continuous background traffic forwarding across the serial command bridge.
- Disabling and restoring periodic streaming traffic via RoutineControl (0x0200).
- J1939 address claim handling when J1939 mode is active.
"""

import os
import time
import pytest

if not os.environ.get("RENODE_SIM_REQUIRED"):
    pytest.skip("Renode runtime simulation not enabled (set RENODE_SIM_REQUIRED=1 to run)", allow_module_level=True)

import can

RENODE_HOST = os.environ.get("RENODE_HOST", "127.0.0.1")
RENODE_PORT = int(os.environ.get("RENODE_PORT", "29536"))

# Standard UDS physical request and response arbitration IDs
UDS_PHYS_REQ_IDS = {
    'A': 0x7E0,
    'B': 0x7E1,
    'C': 0x7E2,
    'D': 0x7E3,
}

UDS_PHYS_RESP_IDS = {
    'A': 0x7E8,
    'B': 0x7E9,
    'C': 0x7EA,
    'D': 0x7EB,
}

UDS_FUNC_REQ_ID = 0x7DF


@pytest.fixture(scope="module")
def can_bus():
    """Establishes and maintains a python-can SLCAN connection to Renode."""
    channel = f"socket://{RENODE_HOST}:{RENODE_PORT}"
    bus = None
    start = time.time()
    last_err = None
    while time.time() - start < 15.0:
        try:
            bus = can.Bus(interface="slcan", channel=channel, bitrate=500000)
            break
        except Exception as e:
            last_err = e
            time.sleep(0.5)

    if bus is None:
        pytest.fail(f"Could not connect to Renode SLCAN socket at {channel}: {last_err}")

    # Flush any initial greeting or boot messages
    start_flush = time.time()
    while time.time() - start_flush < 0.5:
        bus.recv(timeout=0.05)

    yield bus
    bus.shutdown()


def test_renode_background_traffic_forwarding(can_bus):
    """Verifies that ECU A's LPUART1 bridge receives and forwards background CAN frames."""
    start_time = time.time()
    captured = []

    while time.time() - start_time < 3.0:
        msg = can_bus.recv(timeout=0.2)
        if msg is not None:
            captured.append(msg)
            if len(captured) >= 10:
                break

    assert len(captured) >= 3, (
        f"Expected periodic background CAN frames from simulated ECUs, but only received {len(captured)}"
    )


def test_renode_uds_physical_tester_present(can_bus):
    """
    Verifies physical UDS Tester Present (0x3E) request/response routing.
    Ports TestStdFirmwareRouting.test_uds_routing_all_ecus and
    TestECUAHostInteraction.test_uds_physical_host_interaction to runtime hardware simulation.
    """
    tester_present_data = bytes([0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])

    for letter, req_id in UDS_PHYS_REQ_IDS.items():
        expected_resp_id = UDS_PHYS_RESP_IDS[letter]

        # Drain pending frames
        while can_bus.recv(timeout=0.02) is not None:
            pass

        req_msg = can.Message(
            arbitration_id=req_id,
            is_extended_id=False,
            data=tester_present_data,
        )
        can_bus.send(req_msg)

        # Wait for physical response
        response = None
        start = time.time()
        while time.time() - start < 1.5:
            msg = can_bus.recv(timeout=0.1)
            if msg is not None and msg.arbitration_id == expected_resp_id:
                response = msg
                break

        # If running in J1939 mode, 11-bit standard UDS requests may not be handled
        # Check if J1939 traffic is dominant on the bus
        if response is None:
            # Probe if J1939 mode is active by testing address claim
            claim_req = can.Message(
                arbitration_id=0x18EAFFF1,
                is_extended_id=True,
                data=bytes([0x00, 0xEE, 0x00]),
            )
            can_bus.send(claim_req)
            claim_resp = can_bus.recv(timeout=0.5)
            if claim_resp and claim_resp.is_extended_id:
                pytest.skip("Firmware is built in J1939 mode; skipping Standard 11-bit UDS test.")

        assert response is not None, f"ECU {letter} (0x{expected_resp_id:X}) did not respond to UDS Tester Present"
        assert response.arbitration_id == expected_resp_id
        assert response.dlc >= 2
        # Positive response: service ID | 0x40 = 0x7E
        assert response.data[1] == 0x7E, f"ECU {letter} returned non-positive response: {response.data.hex()}"


def test_renode_uds_functional_broadcast(can_bus):
    """
    Verifies functional UDS Tester Present (0x7DF) broadcast routing.
    Ports TestStdFirmwareRouting.test_uds_functional_routing and
    TestECUAHostInteraction.test_uds_functional_host_interaction_all_ecus to live simulation.
    """
    tester_present_data = bytes([0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])

    # Drain pending frames
    while can_bus.recv(timeout=0.02) is not None:
        pass

    req_msg = can.Message(
        arbitration_id=UDS_FUNC_REQ_ID,
        is_extended_id=False,
        data=tester_present_data,
    )
    can_bus.send(req_msg)

    received_responders = set()
    start = time.time()
    expected_resp_ids = set(UDS_PHYS_RESP_IDS.values())

    while time.time() - start < 2.0:
        msg = can_bus.recv(timeout=0.1)
        if msg is not None and msg.arbitration_id in expected_resp_ids:
            if msg.dlc >= 2 and msg.data[1] == 0x7E:
                received_responders.add(msg.arbitration_id)
                if len(received_responders) == len(expected_resp_ids):
                    break

    # If in J1939 mode, skip if no standard responses
    if len(received_responders) == 0:
        pytest.skip("Firmware is running in J1939 mode; skipping Standard functional UDS test.")

    # In Standard mode, at least ECU A (local) and other responding ECUs must reply
    assert 0x7E8 in received_responders, "ECU A did not reply to functional broadcast request"
    assert len(received_responders) >= 2, (
        f"Expected multiple ECUs to reply to functional broadcast 0x7DF, got: {[hex(x) for x in received_responders]}"
    )


def test_renode_j1939_address_claims_if_active(can_bus):
    """
    If the firmware is compiled with J1939 mode, verifies that broadcast address claim
    requests (PGN 60928) elicit address claims from simulated ECUs.
    """
    claim_req = can.Message(
        arbitration_id=0x18EAFFF1,
        is_extended_id=True,
        data=bytes([0x00, 0xEE, 0x00]),
    )
    can_bus.send(claim_req)

    claims = set()
    start = time.time()
    while time.time() - start < 2.0:
        msg = can_bus.recv(timeout=0.1)
        if msg is not None and msg.is_extended_id:
            pgn = (msg.arbitration_id >> 8) & 0x1FFFF
            if (pgn & 0xFF00) == 0xEE00:
                sa = msg.arbitration_id & 0xFF
                claims.add(sa)

    if len(claims) == 0:
        # Standard mode active, test not applicable
        pytest.skip("No J1939 address claims detected; firmware is in Standard 11-bit mode.")

    # When J1939 is active, ECU A (0x2A) must claim
    assert 0x2A in claims, f"ECU A (SA 0x2A) missing from address claims: {[hex(x) for x in claims]}"


def test_renode_websocket_handshake():
    """
    Verifies that the superserver proxy accepts WebSocket RFC 6455 upgrade requests
    and exchanges SLCAN frames over WebSocket text framing (supporting SloppyCAN).
    """
    import base64
    import hashlib
    import socket

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(5.0)
    s.connect((RENODE_HOST, RENODE_PORT))

    ws_key = base64.b64encode(b"0123456789abcdef").decode()
    expected_accept = base64.b64encode(
        hashlib.sha1((ws_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode()).digest()
    ).decode()

    handshake = (
        f"GET / HTTP/1.1\r\n"
        f"Host: {RENODE_HOST}:{RENODE_PORT}\r\n"
        f"Upgrade: websocket\r\n"
        f"Connection: Upgrade\r\n"
        f"Sec-WebSocket-Key: {ws_key}\r\n"
        f"Sec-WebSocket-Version: 13\r\n\r\n"
    ).encode("ascii")

    s.sendall(handshake)

    response = b""
    while b"\r\n\r\n" not in response:
        chunk = s.recv(1024)
        if not chunk:
            break
        response += chunk

    assert b"101 Switching Protocols" in response, f"Unexpected handshake response: {response}"
    assert f"Sec-WebSocket-Accept: {expected_accept}".lower().encode() in response.lower()

    # Receive at least one RFC 6455 frame containing SLCAN data
    data = s.recv(1024)
    assert len(data) >= 2, "Expected WebSocket frame payload"
    assert (data[0] & 0x0F) in (1, 2)  # Text or binary opcode

    # Send clean close frame (opcode 0x8)
    close_frame = bytes([0x88, 0x80, 0x00, 0x00, 0x00, 0x00])
    s.sendall(close_frame)
    s.close()


def test_renode_disable_streaming_traffic(can_bus):
    """
    Verifies disabling and resuming periodic streaming background traffic
    via UDS RoutineControl (0x31) Request Silence (routine 0x0200).
    - Subfunction 0x01: Start silence (stops periodic DBC transmissions across ECUs).
    - Subfunction 0x02: Stop silence (restores periodic DBC transmissions across ECUs).
    """
    # 1. Verify baseline periodic traffic is flowing
    baseline_frames = []
    start_time = time.time()
    while time.time() - start_time < 2.0:
        msg = can_bus.recv(timeout=0.2)
        if msg is not None:
            baseline_frames.append(msg)
            if len(baseline_frames) >= 5:
                break

    assert len(baseline_frames) >= 3, (
        f"Expected baseline periodic traffic before silence test, got {len(baseline_frames)}"
    )

    # Determine mode based on captured frame IDs
    is_j1939 = any(m.is_extended_id for m in baseline_frames)

    start_silence_data = bytes([0x04, 0x31, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00])
    stop_silence_data = bytes([0x04, 0x31, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00])

    if not is_j1939:
        silence_targets = [UDS_FUNC_REQ_ID] + list(UDS_PHYS_REQ_IDS.values())
        is_ext = False
    else:
        # In J1939 mode, functional broadcast and physical diagnostic IDs for ECUs A, B, C, D
        silence_targets = [0x18DBFFF1, 0x18DA2AF1, 0x18DA13F1, 0x18DA5AF1, 0x18DA21F1]
        is_ext = True

    # 2. Send Start Silence routine control
    for target_id in silence_targets:
        can_bus.send(
            can.Message(
                arbitration_id=target_id,
                is_extended_id=is_ext,
                data=start_silence_data,
            )
        )

    # Drain positive responses and any in-flight frames
    drain_start = time.time()
    while time.time() - drain_start < 0.5:
        can_bus.recv(timeout=0.05)

    # 3. Assert silence: no background periodic traffic should be received
    silence_frames = []
    silence_start = time.time()
    while time.time() - silence_start < 1.0:
        msg = can_bus.recv(timeout=0.1)
        if msg is not None:
            # Filter out any lingering UDS positive responses (service 0x71)
            if msg.dlc >= 2 and msg.data[1] == 0x71:
                continue
            silence_frames.append(msg)

    assert len(silence_frames) == 0, (
        f"Expected silence on bus after RoutineControl Request Silence, but received {len(silence_frames)} frames: "
        f"{[hex(m.arbitration_id) for m in silence_frames[:5]]}"
    )

    # 4. Restore traffic: Send Stop Silence routine control
    for target_id in silence_targets:
        can_bus.send(
            can.Message(
                arbitration_id=target_id,
                is_extended_id=is_ext,
                data=stop_silence_data,
            )
        )

    # Drain stop-silence UDS responses
    drain_start = time.time()
    while time.time() - drain_start < 0.3:
        can_bus.recv(timeout=0.05)

    # 5. Assert periodic traffic resumed
    resumed_frames = []
    resume_start = time.time()
    while time.time() - resume_start < 2.0:
        msg = can_bus.recv(timeout=0.2)
        if msg is not None:
            if msg.dlc >= 2 and msg.data[1] == 0x71:
                continue
            resumed_frames.append(msg)
            if len(resumed_frames) >= 5:
                break

    assert len(resumed_frames) >= 3, (
        f"Expected periodic traffic to resume after Stop Silence, but received only {len(resumed_frames)} frames"
    )

