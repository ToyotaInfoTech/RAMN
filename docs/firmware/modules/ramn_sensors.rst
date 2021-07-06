Sensors Management
==================

Description
-----------

The **ramn_sensors** module handles reading values from sensors on RAMN's expansion boards. It mainly relies on 3 ADC, which values are automatically handled by the DMA controller.

.. code-block:: C

	//Initializes the Sensors module
	void 	RAMN_SENSORS_Init(void);

	//Update the Sensors value. Must be called periodically
	void 	RAMN_SENSORS_Update(uint32_t tick);
