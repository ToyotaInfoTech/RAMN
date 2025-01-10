# Ordering from PCBWay

The design files here are complete enough that you can fabricate and/or assemble your own RAMN and pods. But there are two 're-orders' that can be made from PCBWay to get your own RAMN: 1) the mainboard and 2) the pods (excluding debugger). The mainboard is usable for many applications all by itself, but acquiring the pods is required for changing the simulated vehicle state e.g. driving the car in CARLA.

1. [![Mainboard from PCBWay Button]][Mainboard]

2. [![Pods from PCBWay Button]][PodsPanel] (you must order additional parts yourself, see below)

The bill of materials used in the above orders were tailored to match the components that are available from PCBWay, and some parts are removed (marked DNP) to reduce costs (e.g. test points).


## Aditional Parts Ordering for Pods

The RAMN mainboard assembly can be ordered completely from PCBWay. PCBWAy is not able to obtain all the parts required for RAMN pods; therefore, the following parts must be acquired from other sources (e.g. digikey, mouser) and installed (soldered) by yourself -- don't worry these are easy to solder!

| Manufacturer Name | Manufacturer Part Number | Description |
|-------------------|--------------------------|-------------|
| Adafruit Industries LLC | 4313 | 1.3" 240X240 WIDE ANGLE TFT LCD |
| NKK Switches  | NR01104ANG13-2A | Rotary Switch 4 Position SP4T 0.4VA (AC/DC) 28 VAC Through Hole 4POS 0.4VA 28V |
| NKK Switches  | SK14EG13 | SWITCH KEYLK 3POS SP3T 0.4VA 28V |


# Creating New Orders from Updated Design Files

The PCBWay re-orders above are based on the design files in this repo at commit e3e3c4afea73fa5b9c6d3bd1bc296d8b05590554 . When changes are made to the design files here and new orders need to be made, the following procedure should be followed to created updated versions of the files in `orders/`:

[Install `kikit`](https://yaqwsx.github.io/KiKit/latest/installation/intro/) and run the script `make_orders.{bat|sh}` here from within KiCad 7.0 shell or [KiCad 7.0](https://yaqwsx.github.io/KiKit/latest/installation/intro/#running-kikit-in-ci-or-isolated-environment-via-docker) and follow the manual steps required printed to the console.

# Design Files

This project contains KiCAD projects for RAMN's hardware.

'Mainboard':
* 0_ramn

and 'Pods':
* 1_screens
* 2_chassis
* 3_powertrain
* 4_body
* 5_debugger

<!---------------------------------------------------------------------------->

[Mainboard from PCBWay Button]: https://img.shields.io/badge/Mainboard_from_PCBWay-37a779?style=for-the-badge

[Mainboard]: https://www.pcbway.com/project/shareproject/https_github_com_ToyotaInfoTech_RAMN_mainboard_ONLY_0dae9b72.html

[Pods from PCBWay Button]: https://img.shields.io/badge/Pods_from_PCBWay-37a779?style=for-the-badge

[PodsPanel]: https://www.pcbway.com/project/shareproject/https_github_com_ToyotaInfoTech_RAMN_pods_ONLY_3ff2c7a7.html
