Please [see the docs 'Hardware' for more details](https://ramn.readthedocs.io/page/hardware.html).

# Creating New Orders from Updated Design Files

The PCBWay re-orders in the docs are based on the design files in this repo at commit e3e3c4afea73fa5b9c6d3bd1bc296d8b05590554 . When changes are made to the design files here and new orders need to be made, the following procedure should be followed to created updated versions of the files in `orders/`:

[Install `kikit`](https://yaqwsx.github.io/KiKit/latest/installation/intro/) and run the script `make_orders.{bat|sh}` here from within KiCad 7.0 shell or [KiCad 7.0](https://yaqwsx.github.io/KiKit/latest/installation/intro/#running-kikit-in-ci-or-isolated-environment-via-docker) and follow the manual steps required printed to the console.

# Design Files

This project contains KiCAD projects for RAMN's hardware.

'Mainboard':
* 0_ramn

and 'Expansions':
* 1_screens
* 2_chassis
* 3_powertrain
* 4_body
* 5_debugger
