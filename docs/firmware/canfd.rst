CAN-FD
======

`CAN-FD <https://en.wikipedia.org/wiki/CAN_FD>`_ is handled by the `HAL Library of the STM32L5 <https://www.st.com/resource/en/user_manual/dm00669466-description-of-stm32l5-hal-and-lowlayer-drivers-stmicroelectronics.pdf>`_.

Received messages are a combination of FDCAN_RxHeaderTypeDef struct and uint8_t array.
A received message is a struct with:


* uint32_t Identifier
* uint32_t IdType
* uint32_t RxFrameType
* uint32_t DataLength
* uint32_t ErrorStateIndicator
* uint32_t BitRateSwitch
* uint32_t FDFormat
* uint32_t RxTimestamp
* uint32_t FilterIndex
* uint32_t IsFilterMatchingFrame

A message to send is a struct with:

* uint32_t Identifier
* uint32_t IdType
* uint32_t TxFrameType
* uint32_t DataLength
* uint32_t ErrorStateIndicator
* uint32_t BitRateSwitch
* uint32_t FDFormat
* uint32_t TxEventFifoControl
* uint32_t MessageMarker

.. warning:: The DataLength field must be sent using a dedicated enumeration, *NOT* by setting the value directly. For example, the DataLength of a 1-byte message should be set not to "1" but to :code:`FDCAN_DLC_BYTES_1` (which is actually equal to 0x00010000U)

Sent messages are a combination of FDCAN_TxHeaderTypeDef struct and uint8_t array.