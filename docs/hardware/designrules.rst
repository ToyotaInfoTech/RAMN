.. _hwdesignrules:

Hardware Design rules
=====================

Guidelines
----------
RAMN aims to promote education and research in automotive systems. To stay close to automotive electronics, we use AEC-Qxxx grade 0 (or equivalent) components for the ECU network board. To keep the board small and affordable, many protections (e.g. ESD protection, pull-up/pull-down resistors, etc.) are omitted, and the board would not meet reliabilily levels required in safety-critical systems.

.. figure:: img/front_pcb.png

   Picture of RAMN PCB's front size, before components soldering.

.. figure:: img/back_pcb.png

   Picture of RAMN's PCB's back size, before components soldering.

All boards are designed using only two layers, with all components on the same side. 
The boards have large manufacturing tolerances (e.g. large track width and clearance), and we restrict components to those with visible external pins, unless there really is no viable alternative.
All those elements ensure that the PCBs can be manufactured and assembled at a low cost. The board is also accessible to hobbyists who would like to fabricate and solder the board themselves.

.. figure:: img/kicad_netclasses.png

   Design tolerances for RAMN's PCB.

Impedance matching
------------------

Since frequencies involved in RAMN are relatively low, proper impedance matching is not a high priority. However, good design guidelines are followed when possible.

CAN-FD bus
**********

Although the quality of the CAN-FD bus line is limited by the tight layout, the many connections, and the use of only 2 layers, we tried to keep a 120ohm differential impedance on the CAN-FD bus.
More precisely, we used a trace width of 0.45mm and a spacing of 0.2mm, which according to `EEWeb's Edge Coupled Microstrip Impedance calculation tool <http://eeweb.com/tools/edge-coupled-microstrip-impedance/>`_, should lead to a 120ohm differential impedance on a standard 35um copper 1.6mm FR-4 thick layout.


.. figure:: img/CANbus_layout.png

   Layout of the CAN-FD bus.

USB line
********

For the USB line, no impedance matching is attempted. Instead, we follow `FTDI's recommendations for USB hardware design <http://ftdichip.com/Documents/AppNotes/AN_146_USB_Hardware_Design_Guidelines_for_FTDI_ICs.pdf>`_:

* Equal length for D+ and D-
* Ground plane under the D+ and D-
* Etc.

.. figure:: img/USB_layout.png

   Layout of the USB connection.
   

Other considerations
--------------------

We tried to follow other common guidelines for PCB design:

* Redundant vias for the 5V and 3.3V line
* Ensuring an uninterrupted ground plane below crystals
* Etc.

CAD Software
------------

PCBs are designed using `KiCAD <https://kicad.org/>`_, which is an open-source tool for PCB design.
Design files can be found on the `github repository <https://github.com/ToyotaInfoTech/RAMN/tree/main/hardware/V1>`_.

.. figure:: img/kicad1.png

   PCB CAD view of RAMN in KiCAD
   
   
.. figure:: img/kicad2.png

   3D view of RAMN in KiCAD