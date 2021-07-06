Memory Layout
=============

RAMN's ECUs make use of the dual bank architecture of the STM32L5. The memory layout of the FLASH memory is as below.

.. figure:: img/memory_layout.png

	Memory Layout of RAMN's ECUs
	
	
The ECU's firmware only use one of the two flash banks at a time. The bank is divided in two parts:

- The Firmware Area, which holds the code currently being executed.
- The EEPROM Emulation Area, which holds data such as VIN, DTC, and other dynamic data.

When reprogramming the ECU, for example over UDS, the ECU will actually copy the new firmware, as well as the current EEPROM, to the inactive bank. Once the copy is completed, the ECU will switch the **SWAP_BANK** Option Bytes to swap the two banks, effectively updating the firmware run by the ECU.

.. warning:: Swapping banks may confuse many external tools, such as STM32CubeIDE or STM32CubeProgrammer. Before using those tools, you should ensure the **SWAP_BANK** Option Byte is unchecked.