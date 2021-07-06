DBC Control
===========

Description
-----------

The **ramn_dbc** module is a module to easily exchange data between ECUs. Values written to the dbc handler will transparently be made available to other ECUs, which can access that value simply by reading the same field from their own dbc handler.

This module helps to make it easy to exchange data over CAN between ECUs, without directly receiving/sending CAN message.

For example, if the powertrain ECU executes

.. code-block:: C

	//Inside Powertrain ECU
	RAMN_DBC_Handle_t.control_brake = 0x7FF;

The body ECU will read 0x7FFF when accessing it's own dbc handler.

.. code-block:: C
	
	//Inside Body ECU
	if (RAMN_DBC_Handle_t.control_brake == 0x7FF){
	//This will be true


The DBC module needs to be updated every time a CAN message is received (to update the fields maintained by other ECUs), and updated periodically to send CAN messages associated to fields maintained by the ECU.


.. code-block:: C

	typedef volatile struct
	{
		volatile uint64_t control_brake; 			//CANID_CONTROL_BRAKE
		volatile uint64_t command_brake; 			//CANID_COMMAND_BRAKE
		volatile uint64_t control_accel; 			//CANID_CONTROL_ACCEL
		volatile uint64_t command_accel; 			//CANID_COMMAND_ACCEL
		volatile uint64_t control_steer; 			//CANID_CONTROL_STEERING
		volatile uint64_t command_steer; 			//CANID_COMMAND_STEERING
		volatile uint64_t control_shift;			//CANID_CONTROL_SHIFT
		volatile uint64_t command_shift;			//CANID_COMMAND_SHIFT
		volatile uint64_t control_sidebrake; 		//CANID_CONTROL_SIDEBRAKE
		volatile uint64_t command_sidebrake; 		//CANID_COMMAND_SIDEBRAKE
		volatile uint64_t status_rpm; 				//CANID_STATUS_RPM
		volatile uint64_t command_horn;				//CANID_COMMAND_HORN
		volatile uint64_t control_horn; 			//CANID_CONTROL_HORN
		volatile uint64_t command_lights; 			//CANID_COMMAND_LIGHTS
		volatile uint64_t command_turnindicator;	//CANID_COMMAND_TURNINDICATOR
		volatile uint64_t control_enginekey;		//CANID_CONTROL_ENGINEKEY
		volatile uint64_t control_lights; 			//CANID_CONTROL_LIGHTS
	} RAMN_DBC_Handle_t;


	//Function to Init the DBC handler
	void 	RAMN_DBC_Init(void);

	//Function to update the DBC when a CAN messages has been received
	void 	RAMN_DBC_ProcessCANMessage(uint32_t canid, uint32_t dlc, const RAMN_CANFrameData_t* dataframe);

	//Function to request the sending of CAN messages maintained by the DBC handler
	void 	RAMN_DBC_Send(uint32_t tick);
