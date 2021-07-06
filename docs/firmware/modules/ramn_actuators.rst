Actuator Control
================

Description
-----------

The **ramn_actuators** module is in charge of controlling actuators of ECU, either by applying control on an expansion (e.g., the LEDs on the BODY expansion) or by updating the payload of periodic messages that control external actuators.

.. code-block:: C

	//Init the Module
	void 	RAMN_ACTUATORS_Init(void);

	//Set the value of the bit specified by "mask" to the value specified by "val" (0 or 1)
	void 	RAMN_ACTUATORS_SetLampState(uint8_t mask, uint8_t val);

	//Applies requested controls to actuators, e.g. light-up LEDs on the board, or update payload of output CAN messages
	void 	RAMN_ACTUATORS_ApplyControls(uint32_t tick);