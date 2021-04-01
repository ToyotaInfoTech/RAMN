USB
==========

USB is used for two purposes:

* power supply of 5V to all ECUs and expansions.
* communication between ECU A and USB host.

Designing a device suitable for `USB-IF certification <https://www.usb.org/compliance>`_ requires to follow `many guidelines <https://www.ftdichip.com/Documents/AppNotes/AN_146_USB_Hardware_Design_Guidelines_for_FTDI_ICs.pdf>`_.
Since RAMN is not meant to be used as a "proper" USB device (such as a keyboard, mouse, etc.) we do not attempt to fulfill all USB-IF requirements. Concretely, this means there is no "VBUS filter" on the board (ferrite beads, etc.)

2-layer version
---------------

On the standard 2-layer version, the USB connection circuit simply features an RC filter between SHIELD and GROUND. A `Shunt resistor <https://en.wikipedia.org/wiki/Shunt_(electrical)#Use_in_current_measuring>`_ can be used for simple power analysis. It is also possible to replace the shunt resistor by a `polyfuse <https://en.wikipedia.org/wiki/Resettable_fuse>`_.

.. figure:: img/usb_2layer.png

   USB connection circuit (2-layer version).
   
.. warning:: the 2-layer version of RAMN assumes the USB host is protected against failures of its loads (short-circuits, etc.). There is no fuse or ESD protection.


4-layer version
---------------
 
On the 4-layer version, the USB connection features an additional ESD protection for the USB data and power lines. It also features a polyfuse to provide additional overcurrent protection. There is still however no VBUS power filter.

.. figure:: img/usb_4layer.png

   USB connection circuit (4-layer version).
   

   