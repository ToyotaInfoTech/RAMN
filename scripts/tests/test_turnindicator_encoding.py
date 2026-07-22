"""
Test that non-J1939 Command_TurnIndicator encoding matches the DBC definition:

    BO_ 423 TurnIndicators: 8 Vector__XXX
     SG_ Left : 8|1@0- (1,0) [0|1] "" Vector__XXX
     SG_ Right : 0|1@0- (1,0) [0|1] "" Vector__XXX

Per the DBC, Right lives in byte 0 and Left lives in byte 1. RAMN's internal
convention (see RAMN_Encode_Command_TurnIndicator_J1939) is that the high
byte of the 16-bit signal value means Left and the low byte means Right, so
byte 0 of the payload must carry the low byte (Right) and byte 1 must carry
the high byte (Left), with no byte-swap.

This test builds the non-J1939 shared library with USE_BIG_ENDIAN_CAN
defined (matching the firmware configuration in ramn_config.h) to reproduce
the on-target byte layout.
"""

import ctypes
import os
import subprocess
import pytest

REPO_ROOT = os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
)
LIB_PATH = os.path.join(os.path.dirname(__file__), "librbd_can_db_bigendian_turnindicator.so")


@pytest.fixture(scope="module", autouse=True)
def build_bigendian_lib():
    """Build the non-J1939 shared library with USE_BIG_ENDIAN_CAN defined.

    This matches the firmware configuration (ramn_config.h defines
    USE_BIG_ENDIAN_CAN) to reproduce the on-target byte layout.
    """
    includes = "-I scripts/tests/mocks/ -I firmware/RAMNV1/Core/Inc/"
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
    ramn_lib.RAMN_Encode_Command_TurnIndicator_Default.argtypes = [
        ctypes.c_uint16,
        ctypes.POINTER(ctypes.c_uint8),
    ]
    ramn_lib.RAMN_Encode_Command_TurnIndicator_Default.restype = None
    ramn_lib.RAMN_Decode_Command_TurnIndicator_Default.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.c_uint32,
    ]
    ramn_lib.RAMN_Decode_Command_TurnIndicator_Default.restype = ctypes.c_uint16
    return ramn_lib


@pytest.mark.parametrize(
    "value, expect_byte0_nonzero, expect_byte1_nonzero, label",
    [
        (0x00FF, True, False, "Right_only"),
        (0xFF00, False, True, "Left_only"),
        (0x0000, False, False, "none"),
    ],
)
def test_turnindicator_bytes_match_dbc_layout(
    lib, value, expect_byte0_nonzero, expect_byte1_nonzero, label
):
    """
    Per busmaster_ramn.dbc, CAN ID 0x1A7 (TurnIndicators) has Right at bit 0
    of byte 0 and Left at bit 0 of byte 1. RAMN's internal value convention
    puts Right in the low byte (0x00FF) and Left in the high byte (0xFF00),
    so encoding must place the low byte in payload[0] and the high byte in
    payload[1] -- unswapped.
    """
    buf = (ctypes.c_uint8 * 8)(*([0] * 8))
    lib.RAMN_Encode_Command_TurnIndicator_Default(ctypes.c_uint16(value), buf)

    assert (buf[0] != 0) == expect_byte0_nonzero, (
        f"[{label}] byte[0]=0x{buf[0]:02X}: Right (byte 0 per DBC) does not "
        f"match expectation for internal value 0x{value:04X}"
    )
    assert (buf[1] != 0) == expect_byte1_nonzero, (
        f"[{label}] byte[1]=0x{buf[1]:02X}: Left (byte 1 per DBC) does not "
        f"match expectation for internal value 0x{value:04X}"
    )


@pytest.mark.parametrize("value", [0x0000, 0x00FF, 0xFF00, 0x0101, 0x1234, 0xFFFF])
def test_turnindicator_roundtrip(lib, value):
    buf = (ctypes.c_uint8 * 8)(*([0] * 8))
    lib.RAMN_Encode_Command_TurnIndicator_Default(ctypes.c_uint16(value), buf)
    decoded = lib.RAMN_Decode_Command_TurnIndicator_Default(buf, ctypes.c_uint32(8))
    assert decoded == value, f"Roundtrip failed for value 0x{value:04X}: got 0x{decoded:04X}"
