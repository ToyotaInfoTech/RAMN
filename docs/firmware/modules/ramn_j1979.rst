J1979 Diagnostics
=================

Description
-----------

The **ramn_j1979** module is a dummy implementation of OBD-II's PID (SAE J1979). 


.. code-block:: C

	RAMN_Result_t RAMN_J1979_ProcessMessage(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);
