import ctypes
import os
import pytest
import subprocess
import sys

# Build the shared library automatically
build_script_path = os.path.join(os.path.dirname(__file__), 'build_serdes_lib.sh')
repo_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
try:
    print("Building shared library...", file=sys.stderr)
    subprocess.run([build_script_path], check=True, cwd=repo_root, capture_output=True)
except subprocess.CalledProcessError as e:
    print(f"Failed to build shared library:\n{e.stderr.decode()}", file=sys.stderr)
    sys.exit(1)

# Load the shared library
lib_path = os.path.join(os.path.dirname(__file__), 'librbd_can_db.so')

try:
    ramn_can_db = ctypes.CDLL(lib_path)
except OSError:
    ramn_can_db = None

if ramn_can_db:
    # Helper to setup a list of functions with the same signature
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
        f = getattr(ramn_can_db, func_name)
        f.argtypes = [ctypes.c_uint16, ctypes.POINTER(ctypes.c_uint8)]
        f.restype = None

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
        f = getattr(ramn_can_db, func_name)
        f.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint32]
        f.restype = ctypes.c_uint16

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
        f = getattr(ramn_can_db, func_name)
        f.argtypes = [ctypes.c_uint8, ctypes.POINTER(ctypes.c_uint8)]
        f.restype = None

    # void (uint8_t, uint8_t, uint8_t*)
    f = getattr(ramn_can_db, "RAMN_Encode_Control_Shift_Joystick")
    f.argtypes = [ctypes.c_uint8, ctypes.c_uint8, ctypes.POINTER(ctypes.c_uint8)]
    f.restype = None

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
        f = getattr(ramn_can_db, func_name)
        f.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint32]
        f.restype = ctypes.c_uint8

def test_ctypes_loaded():
    assert ramn_can_db is not None, "Failed to load librbd_can_db.so"

@pytest.mark.parametrize("signal_name, encode_func, decode_func, range_limit", [
    ("Command_Brake", ramn_can_db.RAMN_Encode_Command_Brake, ramn_can_db.RAMN_Decode_Command_Brake, 4096),
    ("Control_Brake", ramn_can_db.RAMN_Encode_Control_Brake, ramn_can_db.RAMN_Decode_Control_Brake, 4096),
    ("Command_Accel", ramn_can_db.RAMN_Encode_Command_Accel, ramn_can_db.RAMN_Decode_Command_Accel, 4096),
    ("Control_Accel", ramn_can_db.RAMN_Encode_Control_Accel, ramn_can_db.RAMN_Decode_Control_Accel, 4096),
    ("Status_RPM", ramn_can_db.RAMN_Encode_Status_RPM, ramn_can_db.RAMN_Decode_Status_RPM, 65536),
    ("Command_Steering", ramn_can_db.RAMN_Encode_Command_Steering, ramn_can_db.RAMN_Decode_Command_Steering, 65536),
    ("Control_Steering", ramn_can_db.RAMN_Encode_Control_Steering, ramn_can_db.RAMN_Decode_Control_Steering, 65536),
    ("Command_Shift", ramn_can_db.RAMN_Encode_Command_Shift, ramn_can_db.RAMN_Decode_Command_Shift, 256),
    ("Command_Horn", ramn_can_db.RAMN_Encode_Command_Horn, ramn_can_db.RAMN_Decode_Command_Horn, 256),
    ("Control_Horn", ramn_can_db.RAMN_Encode_Control_Horn, ramn_can_db.RAMN_Decode_Control_Horn, 256),
    ("Command_TurnIndicator", ramn_can_db.RAMN_Encode_Command_TurnIndicator, ramn_can_db.RAMN_Decode_Command_TurnIndicator, 65536),
    ("Command_Sidebrake", ramn_can_db.RAMN_Encode_Command_Sidebrake, ramn_can_db.RAMN_Decode_Command_Sidebrake, 65536),
    ("Control_Sidebrake", ramn_can_db.RAMN_Encode_Control_Sidebrake, ramn_can_db.RAMN_Decode_Control_Sidebrake, 256),
    ("Control_EngineKey", ramn_can_db.RAMN_Encode_Control_EngineKey, ramn_can_db.RAMN_Decode_Control_EngineKey, 256),
    ("Command_Lights", ramn_can_db.RAMN_Encode_Command_Lights, ramn_can_db.RAMN_Decode_Command_Lights, 65536),
    ("Control_Lights", ramn_can_db.RAMN_Encode_Control_Lights, ramn_can_db.RAMN_Decode_Control_Lights, 256),
])
def test_signal_roundtrip(signal_name, encode_func, decode_func, range_limit):
    # Test a sample of values if range is large to save time, or test all if small
    step = 1 if range_limit <= 4096 else range_limit // 100
    for val in range(0, range_limit, step):
        payload = (ctypes.c_uint8 * 8)()
        encode_func(val, payload)
        decoded_val = decode_func(payload, 8)
        assert decoded_val == val, f"Roundtrip failed for {signal_name} at value {val}: got {decoded_val}"
    # Always test the max value
    val = range_limit - 1
    payload = (ctypes.c_uint8 * 8)()
    encode_func(val, payload)
    decoded_val = decode_func(payload, 8)
    assert decoded_val == val, f"Roundtrip failed for {signal_name} at max value {val}: got {decoded_val}"

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
