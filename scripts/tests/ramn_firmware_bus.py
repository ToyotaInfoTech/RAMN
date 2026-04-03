import ctypes
import os
import sys

# ---------------------------------------------------------------------------
# C Types Definitions (matching mocks/main.h)
# ---------------------------------------------------------------------------

class FDCAN_TxHeaderTypeDef(ctypes.Structure):
    _fields_ = [
        ("Identifier", ctypes.c_uint32),
        ("IdType", ctypes.c_uint32),
        ("TxFrameType", ctypes.c_uint32),
        ("DataLength", ctypes.c_uint32),
        ("ErrorStateIndicator", ctypes.c_uint32),
        ("BitRateSwitch", ctypes.c_uint32),
        ("FDFormat", ctypes.c_uint32),
        ("TxEventFifoControl", ctypes.c_uint32),
        ("MessageMarker", ctypes.c_uint32),
    ]

class FDCAN_RxHeaderTypeDef(ctypes.Structure):
    _fields_ = [
        ("Identifier", ctypes.c_uint32),
        ("IdType", ctypes.c_uint32),
        ("RxFrameType", ctypes.c_uint32),
        ("DataLength", ctypes.c_uint32),
        ("ErrorStateIndicator", ctypes.c_uint32),
        ("BitRateSwitch", ctypes.c_uint32),
        ("FDFormat", ctypes.c_uint32),
        ("IsFilterMatchingFrame", ctypes.c_uint32),
        ("FilterIndex", ctypes.c_uint32),
    ]

TX_CALLBACK_TYPE = ctypes.CFUNCTYPE(None, ctypes.POINTER(FDCAN_TxHeaderTypeDef), ctypes.POINTER(ctypes.c_uint8))

class RAMNFirmwareBus:
    def __init__(self, ecu_letter, mode='std'):
        """
        Loads the shared library for a specific ECU and mode.
        ecu_letter: 'A', 'B', 'C', or 'D'
        mode: 'std' or 'j1939'
        """
        lib_name = f"librbd_ecu{ecu_letter.upper()}_{mode.lower()}.so"
        lib_path = os.path.join(os.path.dirname(__file__), lib_name)
        
        if not os.path.exists(lib_path):
            raise FileNotFoundError(f"Firmware library not found: {lib_path}. Run build_testing_libs.sh first.")
            
        self.lib = ctypes.CDLL(lib_path)
        self.responses = []
        
        # Setup prototypes
        self.lib.set_tx_callback.argtypes = [TX_CALLBACK_TYPE]
        self.lib.set_tx_callback.restype = None
        
        self.lib.RAMN_UDS_ProcessRxCANMessage.argtypes = [
            ctypes.POINTER(FDCAN_RxHeaderTypeDef),
            ctypes.POINTER(ctypes.c_uint8),
            ctypes.c_uint32,
            ctypes.c_void_p # stream buffer handle, mocked as void*
        ]
        self.lib.RAMN_UDS_ProcessRxCANMessage.restype = ctypes.c_int # RAMN_Bool_t
        
        self.lib.RAMN_KWP_ProcessRxCANMessage.argtypes = self.lib.RAMN_UDS_ProcessRxCANMessage.argtypes
        self.lib.RAMN_KWP_ProcessRxCANMessage.restype = ctypes.c_int
        
        self.lib.RAMN_XCP_ProcessRxCANMessage.argtypes = self.lib.RAMN_UDS_ProcessRxCANMessage.argtypes
        self.lib.RAMN_XCP_ProcessRxCANMessage.restype = ctypes.c_int

        self.lib.RAMN_CUSTOM_ProcessRxCANMessage.argtypes = [
            ctypes.POINTER(FDCAN_RxHeaderTypeDef),
            ctypes.POINTER(ctypes.c_uint8),
            ctypes.c_uint32
        ]
        self.lib.RAMN_CUSTOM_ProcessRxCANMessage.restype = None

        self.lib.get_uds_handle_ptr.argtypes = []
        self.lib.get_uds_handle_ptr.restype = ctypes.c_void_p
        self.uds_handle_ptr = self.lib.get_uds_handle_ptr()

        self.lib.get_kwp_handle_ptr.argtypes = []
        self.lib.get_kwp_handle_ptr.restype = ctypes.c_void_p
        self.kwp_handle_ptr = self.lib.get_kwp_handle_ptr()

        self.lib.get_xcp_handle_ptr.argtypes = []
        self.lib.get_xcp_handle_ptr.restype = ctypes.c_void_p
        self.xcp_handle_ptr = self.lib.get_xcp_handle_ptr()

        # Init prototypes
        self.lib.RAMN_UDS_Init.argtypes = [ctypes.c_uint32]
        self.lib.RAMN_UDS_Init.restype = ctypes.c_int
        self.lib.RAMN_KWP_Init.argtypes = [ctypes.c_uint32]
        self.lib.RAMN_KWP_Init.restype = ctypes.c_int
        self.lib.RAMN_XCP_Init.argtypes = [ctypes.c_uint32]
        self.lib.RAMN_XCP_Init.restype = ctypes.c_int

        self.lib.DLCtoUINT8.argtypes = [ctypes.c_uint32]
        self.lib.DLCtoUINT8.restype = ctypes.c_uint8

        self.responses = []

        # Register callback
        self._c_callback = TX_CALLBACK_TYPE(self._tx_callback_handler)
        self.lib.set_tx_callback(self._c_callback)

        # Initialize protocols
        self.lib.RAMN_UDS_Init(0)
        self.lib.RAMN_KWP_Init(0)
        self.lib.RAMN_XCP_Init(0)

    def _tx_callback_handler(self, header_ptr, data_ptr):
        header = header_ptr.contents
        # Use DLCtoUINT8 to get actual payload size from firmware's DLC
        size = self.lib.DLCtoUINT8(header.DataLength)
        if data_ptr and size > 0:
            data = bytes([data_ptr[i] for i in range(size)])
        else:
            data = b''
        self.responses.append({
            'id': header.Identifier,
            'data': data,
            'is_extended': bool(header.IdType == 1)
        })

    def process_msg(self, can_id, data, is_extended=True, tick=0):
        """Feed a CAN message into the firmware's diagnostic stack."""
        all_responses = []
        
        header = FDCAN_RxHeaderTypeDef()
        header.Identifier = can_id
        header.IdType = 1 if is_extended else 0
        header.DataLength = len(data)
        
        c_data = (ctypes.c_uint8 * len(data))(*data)
        
        # Try all diagnostic servers
        for func, handle in [(self.lib.RAMN_UDS_ProcessRxCANMessage, self.uds_handle_ptr),
                             (self.lib.RAMN_KWP_ProcessRxCANMessage, self.kwp_handle_ptr),
                             (self.lib.RAMN_XCP_ProcessRxCANMessage, self.xcp_handle_ptr)]:
            self.responses = [] # Clear for this specific server
            func(ctypes.byref(header), c_data, tick, handle)
            all_responses.extend(self.responses)
            if self.responses:
                break # Stop at first one that actually sent a response
        
        if not all_responses:
            # Try custom firmware logic (PGN requests, etc.)
            self.responses = []
            self.lib.RAMN_CUSTOM_ProcessRxCANMessage(ctypes.byref(header), c_data, tick)
            all_responses.extend(self.responses)
        
        return all_responses
