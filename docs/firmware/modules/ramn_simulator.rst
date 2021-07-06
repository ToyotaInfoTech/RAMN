ECU Simulator
=============

Description
-----------

The **ramn_simulator** module is the module implementing the control logic of the ECU.
For example, if the powertrain ECU is implementing cruise control, the cruise control algorithm should be located here.


.. code-block:: C

	//Initializes the module
	void 	RAMN_SIM_Init(void);

	//Update the simulator
	void 	RAMN_SIM_UpdatePeriodic(uint32_t tick);
