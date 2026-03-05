import ctypes
import os
import pytest

import json
import os
import atexit
import pretty_j1939.describe

TRANSMITTERS = {
    "Command_Brake": "ECUA",
    "Command_Accel": "ECUA",
    "Status_Rpm": "ECUA",
    "Command_Steering": "ECUA",
    "Command_Shift": "ECUA",
    "Control_Horn": "ECUA",
    "Command_Sidebrake": "ECUA",
    "Control_Steering": "ECUB",
    "Control_Sidebrake": "ECUB",
    "Command_Lights": "ECUB",
    "Control_Brake": "ECUC",
    "Control_Accel": "ECUC",
    "Control_Shift_Joystick": "ECUC",
    "Joystick_Buttons": "ECUC",
    "Command_Horn": "ECUC",
    "Command_Turnindicator": "ECUC",
    "Control_Enginekey": "ECUD",
    "Control_Lights": "ECUD",
}

RECEIVERS = {
    "Control_Brake": ["ECUA", "ECUD"],
    "Control_Accel": ["ECUA"],
    "Control_Steering": ["ECUA"],
    "Control_Shift_Joystick": ["ECUA"],
    "Joystick_Buttons": ["ECUA"],
    "Control_Sidebrake": ["ECUA", "ECUD"],
    "Control_Enginekey": ["ECUA", "ECUC"],
    "Control_Horn": ["ECUC"],
    "Command_Horn": ["ECUA"],
    "Command_Lights": ["ECUA", "ECUB", "ECUD"],
    "Control_Lights": ["ECUA"],
    "Status_Rpm": ["ECUB", "ECUC", "ECUD"],
    "Command_Steering": ["ECUB"],
    "Command_Sidebrake": ["ECUB", "ECUC"],
    "Command_Brake": ["ECUC"],
    "Command_Accel": ["ECUC"],
    "Command_Shift": ["ECUC"],
    "Command_Turnindicator": ["ECUD"],
}

MAPPING_REPORT = []


def save_report():
    report_path = os.path.join(os.path.dirname(__file__), "j1939_mapping_report.json")
    with open(report_path, "w") as f:
        json.dump(MAPPING_REPORT, f, indent=4)


atexit.register(save_report)


class RecordingDescriber:
    def __init__(self, signal_name, *args, **kwargs):
        self.signal_name = signal_name
        self._describer = pretty_j1939.describe.get_describer(*args, **kwargs)

    def __call__(self, data, can_id):
        desc = self._describer(data, can_id)
        if not any(r.get("RAMN_Signal") == self.signal_name for r in MAPPING_REPORT):
            import subprocess
            import sys

            hex_id = f"{can_id:08X}"
            hex_data = bytes(data).hex().upper()
            cmd = f"echo {hex_id}#{hex_data} | {sys.executable} -m pretty_j1939 --bytes --no-summary --da-json .agent/j1939-json/J1939db.json -"
            try:
                res = subprocess.check_output(cmd, shell=True, text=True)
                for line in reversed(res.strip().split("\n")):
                    if line.strip():
                        parsed = json.loads(line.strip())
                        parsed["RAMN_Signal"] = self.signal_name
                        parsed["Tx_ECU"] = TRANSMITTERS.get(self.signal_name, "Unknown")
                        parsed["Rx_ECU"] = RECEIVERS.get(self.signal_name, [])
                        MAPPING_REPORT.append(parsed)
                        break
            except Exception as e:
                MAPPING_REPORT.append(
                    {"RAMN_Signal": self.signal_name, "error": str(e)}
                )
        return desc


import subprocess
import sys

# Build the shared library automatically
build_script_path = os.path.join(os.path.dirname(__file__), "build_serdes_lib.sh")
repo_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
try:
    print("Building shared library...", file=sys.stderr)
    subprocess.run([build_script_path], check=True, cwd=repo_root, capture_output=True)
except subprocess.CalledProcessError as e:
    print(f"Failed to build shared library:\n{e.stderr.decode()}", file=sys.stderr)
    sys.exit(1)

# Load the shared libraries
lib_path = os.path.join(os.path.dirname(__file__), "librbd_can_db.so")
lib_j1939_path = os.path.join(os.path.dirname(__file__), "librbd_can_db_j1939.so")

try:
    ramn_can_db = ctypes.CDLL(lib_path)
except OSError:
    ramn_can_db = None

try:
    ramn_can_db_j1939 = ctypes.CDLL(lib_j1939_path)
except OSError:
    ramn_can_db_j1939 = None

if ramn_can_db:
    # Setup Default functions
    # ... (signatures are the same for both libraries, we can reuse setup logic)
    pass


def setup_lib_functions(lib):
    if not lib:
        return
    # void (uint16_t, uint8_t*)
    encode_funcs_16 = [
        "RAMN_Encode_Command_Brake",
        "RAMN_Encode_Control_Brake",
        "RAMN_Encode_Command_Accel",
        "RAMN_Encode_Control_Accel",
        "RAMN_Encode_Status_RPM",
        "RAMN_Encode_Command_Steering",
        "RAMN_Encode_Control_Steering",
        "RAMN_Encode_Command_TurnIndicator",
        "RAMN_Encode_Command_Sidebrake",
        "RAMN_Encode_Command_Lights",
    ]
    for func_name in encode_funcs_16:
        try:
            f = getattr(lib, func_name)
            f.argtypes = [ctypes.c_uint16, ctypes.POINTER(ctypes.c_uint8)]
            f.restype = None
        except AttributeError:
            pass

    # uint16_t (uint8_t*, uint32_t)
    decode_funcs_16 = [
        "RAMN_Decode_Command_Brake",
        "RAMN_Decode_Control_Brake",
        "RAMN_Decode_Command_Accel",
        "RAMN_Decode_Control_Accel",
        "RAMN_Decode_Status_RPM",
        "RAMN_Decode_Command_Steering",
        "RAMN_Decode_Control_Steering",
        "RAMN_Decode_Command_TurnIndicator",
        "RAMN_Decode_Command_Sidebrake",
        "RAMN_Decode_Command_Lights",
    ]
    for func_name in decode_funcs_16:
        try:
            f = getattr(lib, func_name)
            f.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint32]
            f.restype = ctypes.c_uint16
        except AttributeError:
            pass

    # void (uint8_t, uint8_t*)
    encode_funcs_8 = [
        "RAMN_Encode_Command_Shift",
        "RAMN_Encode_Command_Horn",
        "RAMN_Encode_Control_Horn",
        "RAMN_Encode_Control_Sidebrake",
        "RAMN_Encode_Control_EngineKey",
        "RAMN_Encode_Control_Lights",
    ]
    for func_name in encode_funcs_8:
        try:
            f = getattr(lib, func_name)
            f.argtypes = [ctypes.c_uint8, ctypes.POINTER(ctypes.c_uint8)]
            f.restype = None
        except AttributeError:
            pass

    # void (uint8_t, uint8_t, uint8_t*)
    try:
        f = getattr(lib, "RAMN_Encode_Control_Shift_Joystick")
        f.argtypes = [ctypes.c_uint8, ctypes.c_uint8, ctypes.POINTER(ctypes.c_uint8)]
        f.restype = None
    except AttributeError:
        pass

    # uint8_t (uint8_t*, uint32_t)
    decode_funcs_8 = [
        "RAMN_Decode_Command_Shift",
        "RAMN_Decode_Control_Shift",
        "RAMN_Decode_Joystick",
        "RAMN_Decode_Command_Horn",
        "RAMN_Decode_Control_Horn",
        "RAMN_Decode_Control_Sidebrake",
        "RAMN_Decode_Control_EngineKey",
        "RAMN_Decode_Control_Lights",
    ]
    for func_name in decode_funcs_8:
        try:
            f = getattr(lib, func_name)
            f.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint32]
            f.restype = ctypes.c_uint8
        except AttributeError:
            pass


setup_lib_functions(ramn_can_db)
setup_lib_functions(ramn_can_db_j1939)


def test_ctypes_loaded():
    assert ramn_can_db is not None, "Failed to load librbd_can_db.so"
    assert ramn_can_db_j1939 is not None, "Failed to load librbd_can_db_j1939.so"


@pytest.mark.parametrize(
    "signal_name, range_limit",
    [
        ("Command_Brake", 4096),
        ("Control_Brake", 4096),
        ("Command_Accel", 4096),
        ("Control_Accel", 4096),
        ("Status_RPM", 65536),
        ("Command_Steering", 65536),
        ("Control_Steering", 65536),
        ("Command_Shift", 256),
        ("Command_Horn", 256),
        ("Control_Horn", 4),
        ("Command_TurnIndicator", 65536),
        ("Command_Sidebrake", 65536),
        ("Control_Sidebrake", 256),
        ("Control_EngineKey", 256),
        ("Command_Lights", 65536),
        ("Control_Lights", 256),
    ],
)
def test_default_signal_roundtrip(signal_name, range_limit):
    encode_func = getattr(ramn_can_db, f"RAMN_Encode_{signal_name}")
    decode_func = getattr(ramn_can_db, f"RAMN_Decode_{signal_name}")
    step = 1 if range_limit <= 4096 else range_limit // 100
    for val in range(0, range_limit, step):
        payload = (ctypes.c_uint8 * 8)()
        encode_func(val, payload)
        decoded_val = decode_func(payload, 8)
        assert (
            decoded_val == val
        ), f"Roundtrip failed for {signal_name} at value {val}: got {decoded_val}"


@pytest.mark.parametrize(
    "signal_name, range_limit, tolerance, used_bytes",
    [
        ("Command_Brake", 4096, 0, [0, 1]),
        ("Control_Brake", 4096, 20, [1]),
        ("Command_Accel", 4096, 0, [1, 2]),
        ("Control_Accel", 4096, 20, [1]),
        ("Status_RPM", 65536, 0, [3, 4]),
        ("Command_Steering", 65536, 0, [0, 1]),
        ("Control_Steering", 35456, 0, [0, 1]),
        ("Command_Shift", 130, 0, [2]),
        ("Command_Horn", 2, 0, [3]),
        ("Control_Horn", 256, 0, [0]),
        # Command_TurnIndicator now uses specific RAMN mappings (0x0100, 0x0001) instead of 0-15, tested separately
        ("Command_Sidebrake", 4, 0, [0]),
        ("Control_Sidebrake", 2, 0, [3]),
    ],
)
def test_j1939_signal_roundtrip(signal_name, range_limit, tolerance, used_bytes):
    encode_func = getattr(ramn_can_db_j1939, f"RAMN_Encode_{signal_name}")
    decode_func = getattr(ramn_can_db_j1939, f"RAMN_Decode_{signal_name}")
    step = 1 if range_limit <= 4096 else range_limit // 100
    for val in range(0, range_limit, step):
        # Initialize payload with zeros to check if C code memsets to 0xFF
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(val, payload)

        # Isolation: Unused bytes should be 0xFF (set by C code)
        for i in range(8):
            if i not in used_bytes:
                assert (
                    payload[i] == 0xFF
                ), f"Isolation failure for {signal_name} at byte {i}, val {val}: expected 0xFF, got 0x{payload[i]:02X}"

        decoded_val = decode_func(payload, 8)
        assert (
            abs(int(decoded_val) - val) <= tolerance
        ), f"J1939 Roundtrip failed for {signal_name} at value {val}: got {decoded_val}, expected {val} +/- {tolerance}"


def test_j1939_command_brake_pretty_decode():
    import pretty_j1939.describe

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Brake")
    describer = RecordingDescriber(
        "Command_Brake", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0 -> 0 m/s2
    payload = (ctypes.c_uint8 * 8)()
    encode_func(0, payload)
    desc = describer(bytes(payload), 0x04040B2A)

    # It should be exactly 0.0 or very close (1.17e-05)
    assert "1.1718750000611067e-05 [m/s2]" in str(
        desc.get("External Acceleration Demand")
    )

    # 2048 -> 1 m/s2 deceleration
    payload = (ctypes.c_uint8 * 8)()
    encode_func(2048, payload)
    desc = describer(bytes(payload), 0x04040B2A)
    assert str(desc.get("External Acceleration Demand")).startswith("-0.99998828")

    # 4095 -> approx 2 m/s2 deceleration
    payload = (ctypes.c_uint8 * 8)()
    encode_func(4095, payload)
    desc = describer(bytes(payload), 0x04040B2A)
    assert str(desc.get("External Acceleration Demand")).startswith("-1.999")


def test_j1939_control_brake_pretty_decode():
    import pretty_j1939.describe

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Brake")
    describer = RecordingDescriber(
        "Control_Brake", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0 -> 0%
    payload = (ctypes.c_uint8 * 8)()
    encode_func(0, payload)
    desc = describer(bytes(payload), 0x18F0015A)

    assert desc.get("Brake Pedal Position") == "0.0 [%]"

    # 2048 -> ~50%
    payload = (ctypes.c_uint8 * 8)()
    encode_func(2048, payload)
    desc = describer(bytes(payload), 0x18F0015A)
    assert desc.get("Brake Pedal Position") == "50.0 [%]"

    # 4095 -> 100%
    payload = (ctypes.c_uint8 * 8)()
    encode_func(4095, payload)
    desc = describer(bytes(payload), 0x18F0015A)
    assert desc.get("Brake Pedal Position") == "100.0 [%]"


def test_j1939_command_accel_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Accel")
    describer = RecordingDescriber(
        "Command_Accel", da_json=".agent/j1939-json/J1939db.json"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(0, payload)
    assert (
        describer(bytes(payload), 0x0C00002A).get("Engine Requested Speed/Speed Limit")
        == "0.0 [rpm]"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(2048, payload)
    assert (
        describer(bytes(payload), 0x0C00002A).get("Engine Requested Speed/Speed Limit")
        == "256.0 [rpm]"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(4095, payload)
    assert (
        describer(bytes(payload), 0x0C00002A).get("Engine Requested Speed/Speed Limit")
        == "511.875 [rpm]"
    )


def test_j1939_control_accel_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Accel")
    describer = RecordingDescriber(
        "Control_Accel", da_json=".agent/j1939-json/J1939db.json"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(0, payload)
    assert (
        describer(bytes(payload), 0x18F0035A).get("Accelerator Pedal Position 1")
        == "0.0 [%]"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(2048, payload)
    assert (
        describer(bytes(payload), 0x18F0035A).get("Accelerator Pedal Position 1")
        == "50.0 [%]"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(4095, payload)
    assert (
        describer(bytes(payload), 0x18F0035A).get("Accelerator Pedal Position 1")
        == "100.0 [%]"
    )


def test_j1939_status_rpm_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Status_RPM")
    describer = RecordingDescriber(
        "Status_Rpm", da_json=".agent/j1939-json/J1939db.json"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(0, payload)
    assert describer(bytes(payload), 0x0CF0042A).get("Engine Speed") == "0.0 [rpm]"

    payload = (ctypes.c_uint8 * 8)()
    encode_func(2048, payload)
    assert describer(bytes(payload), 0x0CF0042A).get("Engine Speed") == "256.0 [rpm]"

    payload = (ctypes.c_uint8 * 8)()
    encode_func(4095, payload)
    assert describer(bytes(payload), 0x0CF0042A).get("Engine Speed") == "511.875 [rpm]"


def test_j1939_command_steering_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Steering")
    describer = RecordingDescriber(
        "Command_Steering", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x08EF13A0 (Proprietary A, DA 19, SA 160)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(4095, payload)
    desc = describer(bytes(payload), 0x08EF132A)

    # Proprietary A won't have specific semantic decoding, just a byte dump
    assert "Manufacturer Specific Information (PropA_PDU1)" in str(desc)


def test_j1939_control_steering_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Steering")
    describer = RecordingDescriber(
        "Control_Steering", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x18F00913 (VDC2, SA 19)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(0, payload)
    assert str(
        describer(bytes(payload), 0x18F00913).get("Steering Wheel Angle")
    ).startswith("-1.999")

    payload = (ctypes.c_uint8 * 8)()
    encode_func(2048, payload)
    assert "2.3437" in str(
        describer(bytes(payload), 0x18F00913).get("Steering Wheel Angle")
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(4095, payload)
    assert str(
        describer(bytes(payload), 0x18F00913).get("Steering Wheel Angle")
    ).startswith("1.999")


def test_j1939_command_shift_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Shift")
    describer = RecordingDescriber(
        "Command_Shift", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x0C010005 (TC1, DA 0, SA 5)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(0, payload)
    assert (
        describer(bytes(payload), 0x0C01032A).get("Transmission Requested Gear")
        == "0 [gear value]"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(5, payload)
    assert (
        describer(bytes(payload), 0x0C01032A).get("Transmission Requested Gear")
        == "5 [gear value]"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(125, payload)
    assert (
        describer(bytes(payload), 0x0C01032A).get("Transmission Requested Gear")
        == "125 [gear value]"
    )


def test_j1939_control_shift_joystick_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Shift_Joystick")
    describer = RecordingDescriber(
        "Control_Shift_Joystick", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x18F00503 (ETC2, SA 3)
    # 0 gear (125 raw)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(0, 0, payload)
    desc = describer(bytes(payload), 0x18F00503)

    assert desc.get("Transmission Current Gear") == "0 [gear value]"

    payload = (ctypes.c_uint8 * 8)()
    encode_func(64, 64, payload)
    desc = describer(bytes(payload), 0x18F00503)
    assert desc.get("Transmission Current Gear") == "64 [gear value]"

    payload = (ctypes.c_uint8 * 8)()
    encode_func(125, 125, payload)
    desc = describer(bytes(payload), 0x18F00503)
    assert desc.get("Transmission Current Gear") == "125 [gear value]"


def test_j1939_command_horn_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Horn")
    describer = RecordingDescriber(
        "Command_Horn", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x18FDD45A (CM3, PGN 64980, SA 90)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(1, payload)
    desc = describer(bytes(payload), 0x18FDD45A)

    assert desc.get("Horn Switch") == "1 (horn switch is on)"


def test_j1939_control_horn_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Horn")
    describer = RecordingDescriber(
        "Control_Horn", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x08EF5A2A (Proprietary A) -- RAMN Uses DA=POWERTRAIN_CTRL(90), SA=HEADWAY_CTRL(42) => 0x08EF5A2A
    payload = (ctypes.c_uint8 * 8)()
    encode_func(255, payload)
    desc = describer(bytes(payload), 0x08EF5A2A)

    assert "Manufacturer Specific Information (PropA_PDU1)" in str(desc)


def test_j1939_command_turnindicator_pretty_decode():
    import pretty_j1939.describe
    import ctypes
    import os

    db_path = ".agent/j1939-json/J1939db.json"
    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_TurnIndicator")
    describer = RecordingDescriber("Command_Turnindicator", da_json=db_path)

    # 0x0CFDCC05 (OEL, PGN 64972, SA 5)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(0, payload)
    assert "no turn being signaled" in describer(bytes(payload), 0x0CFDCC05).get(
        "Turn Signal Switch"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(0x0100, payload)  # Left
    assert "left turn to be flashing" in describer(bytes(payload), 0x0CFDCC05).get(
        "Turn Signal Switch"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(0x0001, payload)  # Right
    assert "right turn to be flashing" in describer(bytes(payload), 0x0CFDCC05).get(
        "Turn Signal Switch"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(0x0101, payload)  # Hazard
    assert "reserved" in describer(bytes(payload), 0x0CFDCC05).get("Turn Signal Switch")


def test_j1939_command_sidebrake_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Sidebrake")
    describer = RecordingDescriber(
        "Command_Sidebrake", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x18FEF105 (CCVS1, PGN 65265, SA 5) -> updated to SA 42 (0x18FEF12A)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(1, payload)
    desc = describer(bytes(payload), 0x18FEF12A)

    assert desc.get("Parking Brake Switch") == "1 (parking brake set)"


def test_j1939_control_sidebrake_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Sidebrake")
    describer = RecordingDescriber(
        "Control_Sidebrake", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x18FEFA0D (Brakes 1, PGN 65274, SA 13)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(1, payload)
    desc = describer(bytes(payload), 0x18FEFA0D)

    assert desc.get("Parking Brake Actuator") == "1 (parking brake actuator active)"


def test_j1939_command_lights_roundtrip():
    # Command_Lights now has specific J1939 bit mapping (POS1-4)
    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Lights")
    decode_func = getattr(ramn_can_db_j1939, "RAMN_Decode_Command_Lights")

    # Valid POS values are 1, 2, 3, 4
    for val in [1, 2, 3, 4]:
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(val, payload)

        # Isolation: bytes other than 0 should be 0xFF
        for i in range(8):
            if i != 0:
                assert payload[i] == 0xFF

        decoded_val = decode_func(payload, 8)
        assert (
            decoded_val == val
        ), f"Command_Lights J1939 roundtrip failed for {val}, got {decoded_val}"


def test_j1939_command_lights_pretty_decode():
    import pretty_j1939.describe

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Lights")
    describer = RecordingDescriber(
        "Command_Lights", da_json=".agent/j1939-json/J1939db.json"
    )

    # POS1: Off
    payload = (ctypes.c_uint8 * 8)()
    encode_func(1, payload)
    desc = describer(bytes(payload), 0x0CFE4147)

    assert desc.get("Running Light Command") == "0 (de-activate)"
    assert desc.get("Low Beam Head Light Command") == "0 (de-activate)"
    assert desc.get("High Beam Head Light Command") == "0 (de-activate)"

    # POS2: Park
    payload = (ctypes.c_uint8 * 8)()
    encode_func(2, payload)
    desc = describer(bytes(payload), 0x0CFE4147)
    assert desc.get("Running Light Command") == "1 (activate)"
    assert desc.get("Low Beam Head Light Command") == "0 (de-activate)"
    assert desc.get("High Beam Head Light Command") == "0 (de-activate)"

    # POS3: Lowbeam
    payload = (ctypes.c_uint8 * 8)()
    encode_func(3, payload)
    desc = describer(bytes(payload), 0x0CFE4147)
    assert desc.get("Running Light Command") == "1 (activate)"
    assert desc.get("Low Beam Head Light Command") == "1 (activate)"
    assert desc.get("High Beam Head Light Command") == "0 (de-activate)"

    # POS4: Highbeam
    payload = (ctypes.c_uint8 * 8)()
    encode_func(4, payload)
    desc = describer(bytes(payload), 0x0CFE4147)
    assert desc.get("Running Light Command") == "1 (activate)"
    assert desc.get("Low Beam Head Light Command") == "1 (activate)"
    assert desc.get("High Beam Head Light Command") == "1 (activate)"


def test_j1939_control_enginekey_roundtrip():
    # RAMN values are 1 (OFF), 2 (ACC), 3 (IGN)
    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_EngineKey")
    decode_func = getattr(ramn_can_db_j1939, "RAMN_Decode_Control_EngineKey")

    for val in [1, 2, 3]:
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(val, payload)

        # Isolation: bytes other than 2 should be 0xFF
        for i in range(8):
            if i != 2:
                assert payload[i] == 0xFF

        decoded_val = decode_func(payload, 8)
        assert (
            decoded_val == val
        ), f"Control_EngineKey J1939 roundtrip failed for {val}, got {decoded_val}"


def test_j1939_control_enginekey_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_EngineKey")
    describer = RecordingDescriber(
        "Control_Enginekey", da_json=".agent/j1939-json/J1939db.json"
    )

    # MIDDLE: ACC (RAMN_ENGINEKEY_MIDDLE = 2)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(2, payload)
    desc = describer(bytes(payload), 0x18FDD421)
    # PGN 64980 SA 33
    assert (
        desc.get("Operator Key Switch Accessory Power")
        == "1 (accessory power state active)"
    )
    assert (
        desc.get("Operator Key Switch Ignition Power")
        == "0 (ignition state is not active)"
    )

    # RIGHT: IGN (RAMN_ENGINEKEY_RIGHT = 3)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(3, payload)
    desc = describer(bytes(payload), 0x18FDD421)
    assert (
        desc.get("Operator Key Switch Accessory Power")
        == "1 (accessory power state active)"
    )
    assert (
        desc.get("Operator Key Switch Ignition Power") == "1 (ignition state is active)"
    )


def test_j1939_joystick_buttons_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_JoystickButtons")
    describer = RecordingDescriber(
        "Joystick_Buttons", da_json=".agent/j1939-json/J1939db.json"
    )

    # Test UP
    payload = (ctypes.c_uint8 * 8)()
    encode_func(2, payload)  # RAMN_SHIFT_UP = 2
    desc = describer(bytes(payload), 0x18FF0205)
    # PropB, SA 5
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)

    # Test DOWN
    payload = (ctypes.c_uint8 * 8)()
    encode_func(3, payload)  # RAMN_SHIFT_DOWN = 3
    desc = describer(bytes(payload), 0x18FF0205)
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)

    # Test LEFT
    payload = (ctypes.c_uint8 * 8)()
    encode_func(5, payload)  # RAMN_SHIFT_LEFT = 5
    desc = describer(bytes(payload), 0x18FF0205)
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)

    # Test RIGHT
    payload = (ctypes.c_uint8 * 8)()
    encode_func(4, payload)  # RAMN_SHIFT_RIGHT = 4
    desc = describer(bytes(payload), 0x18FF0205)
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)

    # Test PUSH
    payload = (ctypes.c_uint8 * 8)()
    encode_func(6, payload)  # RAMN_SHIFT_PUSH = 6
    desc = describer(bytes(payload), 0x18FF0205)
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)


def test_shift_joystick_roundtrip():
    encode_func = getattr(ramn_can_db, "RAMN_Encode_Control_Shift_Joystick")
    decode_shift = getattr(ramn_can_db, "RAMN_Decode_Control_Shift")
    decode_joystick = getattr(ramn_can_db, "RAMN_Decode_Joystick")

    for shift_val in [0, 127, 255]:
        for joy_val in [0, 127, 255]:
            payload = (ctypes.c_uint8 * 8)()
            encode_func(shift_val, joy_val, payload)
            assert decode_shift(payload, 8) == shift_val
            assert decode_joystick(payload, 8) == joy_val


def test_j1939_shift_joystick_roundtrip():
    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Shift_Joystick")
    decode_shift = getattr(ramn_can_db_j1939, "RAMN_Decode_Control_Shift")

    for shift_val in [0, 64, 129]:
        # joy_val is no longer used in this message
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(shift_val, 0, payload)

        # Isolation: bytes other than 3 should be 0xFF
        for i in range(8):
            if i not in [3]:
                assert payload[i] == 0xFF

        assert decode_shift(payload, 8) == shift_val


def test_j1939_turn_indicator_roundtrip():
    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_TurnIndicator")
    decode_func = getattr(ramn_can_db_j1939, "RAMN_Decode_Command_TurnIndicator")

    # 0: None, 0x0100: Left, 0x0001: Right, 0x0101: Hazard
    for val in [0, 0x0100, 0x0001, 0x0101]:
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(val, payload)

        for i in range(8):
            if i != 1:
                assert payload[i] == 0xFF

        assert decode_func(payload, 8) == val


def test_j1939_control_lights_roundtrip():
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Lights")
    decode_func = getattr(ramn_can_db_j1939, "RAMN_Decode_Control_Lights")

    # Test all 256 values
    for val in range(256):
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(val, payload)

        # Verify isolation/encoding matches spec
        assert payload[0] == (0xFA if (val & 0x08) else 0x00)
        assert payload[1] == (0xFA if (val & 0x10) else 0x00)
        assert payload[2] == (0xFA if (val & 0x20) else 0x00)
        assert payload[3] == (0xFA if (val & 0x40) else 0x00)
        assert payload[4] == (0xFA if (val & 0x80) else 0x00)
        assert payload[5] == (0xFA if (val & 0x01) else 0x00)
        assert payload[6] == (0xFA if (val & 0x02) else 0x00)
        assert payload[7] == (0xFA if (val & 0x04) else 0x00)

        decoded_val = decode_func(payload, 8)
        assert decoded_val == val, f"Roundtrip failed for val {val}, got {decoded_val}"


def test_j1939_control_lights_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Lights")
    describer = RecordingDescriber(
        "Control_Lights", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x18FF0021 (PropB, PGN 65280, SA 33)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(0x08, payload)  # Taillamp only
    desc = describer(bytes(payload), 0x18FF0021)

    assert "Manufacturer Defined Usage (PropB_PDU2)" in str(desc) or "Bytes" in str(
        desc
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(0x0100, payload)  # Left
    assert "left turn to be flashing" in describer(bytes(payload), 0x0CFDCC05).get(
        "Turn Signal Switch"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(0x0001, payload)  # Right
    assert "right turn to be flashing" in describer(bytes(payload), 0x0CFDCC05).get(
        "Turn Signal Switch"
    )

    payload = (ctypes.c_uint8 * 8)()
    encode_func(0x0101, payload)  # Hazard
    assert "reserved" in describer(bytes(payload), 0x0CFDCC05).get("Turn Signal Switch")


def test_j1939_command_sidebrake_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Sidebrake")
    describer = RecordingDescriber(
        "Command_Sidebrake", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x18FEF105 (CCVS1, PGN 65265, SA 5)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(1, payload)
    desc = describer(bytes(payload), 0x18FEF12A)

    assert desc.get("Parking Brake Switch") == "1 (parking brake set)"


def test_j1939_control_sidebrake_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Sidebrake")
    describer = RecordingDescriber(
        "Control_Sidebrake", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x18FEFA0D (Brake System, PGN 65274, SA 13)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(1, payload)
    desc = describer(bytes(payload), 0x18FEFA0D)

    assert desc.get("Parking Brake Actuator") == "1 (parking brake actuator active)"


def test_j1939_command_lights_roundtrip():
    # Command_Lights now has specific J1939 bit mapping (POS1-4)
    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Lights")
    decode_func = getattr(ramn_can_db_j1939, "RAMN_Decode_Command_Lights")

    # Valid POS values are 1, 2, 3, 4
    for val in [1, 2, 3, 4]:
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(val, payload)

        # Isolation: bytes other than 0 should be 0xFF
        for i in range(8):
            if i != 0:
                assert payload[i] == 0xFF

        decoded_val = decode_func(payload, 8)
        assert (
            decoded_val == val
        ), f"Command_Lights J1939 roundtrip failed for {val}, got {decoded_val}"


def test_j1939_command_lights_pretty_decode():
    import pretty_j1939.describe

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_Lights")
    describer = RecordingDescriber(
        "Command_Lights", da_json=".agent/j1939-json/J1939db.json"
    )

    # POS1: Off
    payload = (ctypes.c_uint8 * 8)()
    encode_func(1, payload)
    desc = describer(bytes(payload), 0x0CFE4147)

    assert desc.get("Running Light Command") == "0 (de-activate)"
    assert desc.get("Low Beam Head Light Command") == "0 (de-activate)"
    assert desc.get("High Beam Head Light Command") == "0 (de-activate)"

    # POS2: Park
    payload = (ctypes.c_uint8 * 8)()
    encode_func(2, payload)
    desc = describer(bytes(payload), 0x0CFE4147)
    assert desc.get("Running Light Command") == "1 (activate)"
    assert desc.get("Low Beam Head Light Command") == "0 (de-activate)"
    assert desc.get("High Beam Head Light Command") == "0 (de-activate)"

    # POS3: Lowbeam
    payload = (ctypes.c_uint8 * 8)()
    encode_func(3, payload)
    desc = describer(bytes(payload), 0x0CFE4147)
    assert desc.get("Running Light Command") == "1 (activate)"
    assert desc.get("Low Beam Head Light Command") == "1 (activate)"
    assert desc.get("High Beam Head Light Command") == "0 (de-activate)"

    # POS4: Highbeam
    payload = (ctypes.c_uint8 * 8)()
    encode_func(4, payload)
    desc = describer(bytes(payload), 0x0CFE4147)
    assert desc.get("Running Light Command") == "1 (activate)"
    assert desc.get("Low Beam Head Light Command") == "1 (activate)"
    assert desc.get("High Beam Head Light Command") == "1 (activate)"


def test_j1939_control_enginekey_roundtrip():
    # RAMN values are 1 (OFF), 2 (ACC), 3 (IGN)
    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_EngineKey")
    decode_func = getattr(ramn_can_db_j1939, "RAMN_Decode_Control_EngineKey")

    for val in [1, 2, 3]:
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(val, payload)

        # Isolation: bytes other than 2 should be 0xFF
        for i in range(8):
            if i != 2:
                assert payload[i] == 0xFF

        decoded_val = decode_func(payload, 8)
        assert (
            decoded_val == val
        ), f"Control_EngineKey J1939 roundtrip failed for {val}, got {decoded_val}"


def test_j1939_control_enginekey_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_EngineKey")
    describer = RecordingDescriber(
        "Control_Enginekey", da_json=".agent/j1939-json/J1939db.json"
    )

    # MIDDLE: ACC (RAMN_ENGINEKEY_MIDDLE = 2)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(2, payload)
    desc = describer(bytes(payload), 0x18FDD421)
    # PGN 64980 SA 33
    assert (
        desc.get("Operator Key Switch Accessory Power")
        == "1 (accessory power state active)"
    )
    assert (
        desc.get("Operator Key Switch Ignition Power")
        == "0 (ignition state is not active)"
    )

    # RIGHT: IGN (RAMN_ENGINEKEY_RIGHT = 3)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(3, payload)
    desc = describer(bytes(payload), 0x18FDD421)
    assert (
        desc.get("Operator Key Switch Accessory Power")
        == "1 (accessory power state active)"
    )
    assert (
        desc.get("Operator Key Switch Ignition Power") == "1 (ignition state is active)"
    )


def test_j1939_joystick_buttons_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_JoystickButtons")
    describer = RecordingDescriber(
        "Joystick_Buttons", da_json=".agent/j1939-json/J1939db.json"
    )

    # Test UP
    payload = (ctypes.c_uint8 * 8)()
    encode_func(2, payload)  # RAMN_SHIFT_UP = 2
    desc = describer(bytes(payload), 0x18FF0205)
    # PropB, SA 5
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)

    # Test DOWN
    payload = (ctypes.c_uint8 * 8)()
    encode_func(3, payload)  # RAMN_SHIFT_DOWN = 3
    desc = describer(bytes(payload), 0x18FF0205)
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)

    # Test LEFT
    payload = (ctypes.c_uint8 * 8)()
    encode_func(5, payload)  # RAMN_SHIFT_LEFT = 5
    desc = describer(bytes(payload), 0x18FF0205)
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)

    # Test RIGHT
    payload = (ctypes.c_uint8 * 8)()
    encode_func(4, payload)  # RAMN_SHIFT_RIGHT = 4
    desc = describer(bytes(payload), 0x18FF0205)
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)

    # Test PUSH
    payload = (ctypes.c_uint8 * 8)()
    encode_func(6, payload)  # RAMN_SHIFT_PUSH = 6
    desc = describer(bytes(payload), 0x18FF0205)
    assert "Bytes" in str(desc) or "Manufacturer" in str(desc)


def test_shift_joystick_roundtrip():
    encode_func = getattr(ramn_can_db, "RAMN_Encode_Control_Shift_Joystick")
    decode_shift = getattr(ramn_can_db, "RAMN_Decode_Control_Shift")
    decode_joystick = getattr(ramn_can_db, "RAMN_Decode_Joystick")

    for shift_val in [0, 127, 255]:
        for joy_val in [0, 127, 255]:
            payload = (ctypes.c_uint8 * 8)()
            encode_func(shift_val, joy_val, payload)
            assert decode_shift(payload, 8) == shift_val
            assert decode_joystick(payload, 8) == joy_val


def test_j1939_shift_joystick_roundtrip():
    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Shift_Joystick")
    decode_shift = getattr(ramn_can_db_j1939, "RAMN_Decode_Control_Shift")

    for shift_val in [0, 64, 129]:
        # joy_val is no longer used in this message
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(shift_val, 0, payload)

        # Isolation: bytes other than 3 should be 0xFF
        for i in range(8):
            if i not in [3]:
                assert payload[i] == 0xFF

        assert decode_shift(payload, 8) == shift_val


def test_j1939_turn_indicator_roundtrip():
    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Command_TurnIndicator")
    decode_func = getattr(ramn_can_db_j1939, "RAMN_Decode_Command_TurnIndicator")

    # 0: None, 0x0100: Left, 0x0001: Right, 0x0101: Hazard
    for val in [0, 0x0100, 0x0001, 0x0101]:
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(val, payload)

        for i in range(8):
            if i != 1:
                assert payload[i] == 0xFF

        assert decode_func(payload, 8) == val


def test_j1939_control_lights_roundtrip():
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Lights")
    decode_func = getattr(ramn_can_db_j1939, "RAMN_Decode_Control_Lights")

    # Test all 256 values
    for val in range(256):
        payload = (ctypes.c_uint8 * 8)(0, 0, 0, 0, 0, 0, 0, 0)
        encode_func(val, payload)

        # Verify isolation/encoding matches spec
        assert payload[0] == (0xFA if (val & 0x08) else 0x00)
        assert payload[1] == (0xFA if (val & 0x10) else 0x00)
        assert payload[2] == (0xFA if (val & 0x20) else 0x00)
        assert payload[3] == (0xFA if (val & 0x40) else 0x00)
        assert payload[4] == (0xFA if (val & 0x80) else 0x00)
        assert payload[5] == (0xFA if (val & 0x01) else 0x00)
        assert payload[6] == (0xFA if (val & 0x02) else 0x00)
        assert payload[7] == (0xFA if (val & 0x04) else 0x00)

        decoded_val = decode_func(payload, 8)
        assert decoded_val == val, f"Roundtrip failed for val {val}, got {decoded_val}"


def test_j1939_control_lights_pretty_decode():
    import pretty_j1939.describe
    import ctypes

    encode_func = getattr(ramn_can_db_j1939, "RAMN_Encode_Control_Lights")
    describer = RecordingDescriber(
        "Control_Lights", da_json=".agent/j1939-json/J1939db.json"
    )

    # 0x18FF0021 (PropB, PGN 65280, SA 33)
    payload = (ctypes.c_uint8 * 8)()
    encode_func(0x08, payload)  # Taillamp only
    desc = describer(bytes(payload), 0x18FF0021)

    assert "Manufacturer Defined Usage (PropB_PDU2)" in str(desc) or "Bytes" in str(
        desc
    )
