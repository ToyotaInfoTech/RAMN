"""
Test that non-J1939 Command_Lights encoding places the lighting switch
value in byte 0 of the CAN payload, matching the DBC definition:

    BO_ 336 LightSwitch: 8 Vector__XXX
     SG_ Value : 2|3@0+ (1,0) [0|7] "" Vector__XXX

The DBC specifies the Value signal starts at bit 2 with length 3 in byte 0.
At minimum, the payload value MUST appear in byte 0 (the first byte), not
byte 1 (the second byte).

This test builds the non-J1939 shared library with USE_BIG_ENDIAN_CAN
defined (matching the firmware configuration in ramn_config.h) to reproduce
the on-target byte layout.
"""

import ctypes
import os
import subprocess
import sys
import pytest

REPO_ROOT = os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
)
LIB_PATH = os.path.join(os.path.dirname(__file__), "librbd_can_db_bigendian.so")


@pytest.fixture(scope="module", autouse=True)
def build_bigendian_lib():
    """Build the non-J1939 shared library with USE_BIG_ENDIAN_CAN defined.

    This matches the firmware configuration (ramn_config.h defines
    USE_BIG_ENDIAN_CAN) to reproduce the on-target byte layout.
    """
    includes = "-I scripts/tests/mocks/ -I firmware/RAMNV1/Core/Inc/"
    # -D__MAIN_H blocks the real main.h (same as build_testing_libs.sh)
    # -DUSE_BIG_ENDIAN_CAN matches the firmware's ramn_config.h setting
    cflags = "-shared -fPIC -DCPYTHON_TESTING -D__MAIN_H -DUSE_BIG_ENDIAN_CAN"
    src = "firmware/RAMNV1/Core/Src/ramn_can_database.c"
    cmd = f"gcc {cflags} {includes} -o {LIB_PATH} {src}"
    result = subprocess.run(
        cmd, shell=True, cwd=REPO_ROOT, capture_output=True, text=True
    )
    if result.returncode != 0:
        pytest.fail(f"Failed to build bigendian lib:\n{result.stderr}")


@pytest.fixture(scope="module")
def lib():
    """Load the big-endian non-J1939 shared library."""
    ramn_lib = ctypes.CDLL(LIB_PATH)
    ramn_lib.RAMN_Encode_Command_Lights.argtypes = [
        ctypes.c_uint16,
        ctypes.POINTER(ctypes.c_uint8),
    ]
    ramn_lib.RAMN_Encode_Command_Lights.restype = None
    ramn_lib.RAMN_Decode_Command_Lights.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.c_uint32,
    ]
    ramn_lib.RAMN_Decode_Command_Lights.restype = ctypes.c_uint16
    return ramn_lib


# Light switch positions per ramn_sensors.h
LIGHTSWITCH_POS1 = 1
LIGHTSWITCH_POS2 = 2
LIGHTSWITCH_POS3 = 3
LIGHTSWITCH_POS4 = 4


@pytest.mark.parametrize(
    "position",
    [LIGHTSWITCH_POS1, LIGHTSWITCH_POS2, LIGHTSWITCH_POS3, LIGHTSWITCH_POS4],
    ids=["POS1_off", "POS2_park", "POS3_low", "POS4_high"],
)
def test_command_lights_value_in_first_byte(lib, position):
    """
    Verify that the lighting switch value is encoded in byte 0 (first byte)
    of the CAN payload, not byte 1 (second byte).

    Per the DBC file (misc/busmaster_ramn.dbc), the LightSwitch message
    (CAN ID 0x150) has its Value signal at bit position 2 with length 3
    in byte 0. The value must be present in byte 0.
    """
    buf = (ctypes.c_uint8 * 8)(*([0] * 8))
    lib.RAMN_Encode_Command_Lights(ctypes.c_uint16(position), buf)

    # The value MUST be in byte 0 (first byte), not byte 1
    assert buf[0] != 0, (
        f"Encoding regression: LightSwitch position {position} has byte[0]=0x00. "
        f"The value landed in byte[1]=0x{buf[1]:02X} instead of byte[0]. "
        f"DBC specifies Value signal in byte 0 of CAN ID 0x150."
    )


@pytest.mark.parametrize(
    "position",
    [LIGHTSWITCH_POS1, LIGHTSWITCH_POS2, LIGHTSWITCH_POS3, LIGHTSWITCH_POS4],
    ids=["POS1_off", "POS2_park", "POS3_low", "POS4_high"],
)
def test_command_lights_roundtrip_first_byte(lib, position):
    """
    Verify encode/decode roundtrip with the value in byte 0.
    """
    buf = (ctypes.c_uint8 * 8)(*([0] * 8))
    lib.RAMN_Encode_Command_Lights(ctypes.c_uint16(position), buf)
    decoded = lib.RAMN_Decode_Command_Lights(buf, ctypes.c_uint32(8))
    assert decoded == position, (
        f"Roundtrip failed for position {position}: got {decoded}"
    )
