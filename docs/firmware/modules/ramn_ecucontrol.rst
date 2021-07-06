ECU Control
===========

Description
-----------

The **ramn_ecucontrol** module is used by ECU A to control the power supply (EN pin) and Boot mode (BOOT0 pin) of ECU B, C and D.


.. code-block:: C

	//Set the state of the "ENABLE PIN" of ECU specified by ecuName ('B', 'C', or 'D')
	void 	RAMN_ECU_SetEnable(char ecuName, uint8_t state);

	//Set the state of the "BOOT0 PIN" of ECU specified by ecuName ('B', 'C', or 'D')
	void 	RAMN_ECU_SetBoot0(char ecuName, uint8_t state);

	//Set the state of the "ENABLE PIN" of ECU B C D
	void 	RAMN_ECU_SetEnableAll(uint8_t state);

	//Set the state of the "BOOT0" of ECU B C D
	void 	RAMN_ECU_SetBoot0All(uint8_t state);

	//Sets the ECU to their default state (BOOT0=0, ENABLE=1)
	void 	RAMN_ECU_SetDefaultState(void);

	//Reset an ECU's power. This function blocks for as long as it takes for the target ECU to reboot.
	void 	RAMN_ECU_ResetECU(char ecuName);