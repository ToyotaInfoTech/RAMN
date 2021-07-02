Practical Introduction to CAN-FD
================================

.. note:: This page assumes the reader already has knowledge about CAN.

CAN-FD (CAN with Flexible Datarate) is an evolution of the "classic" CAN (Controller Area Network). Concretely, CAN-FD brings two new major features:

* Increased maximum payload size (from 8 bytes to 64 bytes). This is achieved by increasing the bitrate during the DATA transmission phase of CAN. Bitrate switching is optional.
* Indication of the "Error State" of the transmitting node. This is achieved by adding an "ESI" flag in the message header.

CAN-FD defines a new header format for CAN-FD messages. Concretely, users have to deal with two new fields:

* The Bitrate Switch (BRS) field
* The Error Status Indicator (ESI) bit

.. warning:: A CAN-FD Controller can send CAN-FD frames OR "classic" CAN frames. However, a "classic" CAN Controller can only receive "classic" CAN frames. A "classic" CAN controller will actively destroy CAN-FD frames it sees on the bus, even when bitrate switching is not in use.

The DLC (Data Length Code - payload size) field is left unchanged. with "classic" CAN, the DLC was a 4-bit field which value could be between 0x0 and 0x8. In CAN-FD, the DLC field is still 4-bit, but can now take a value between 0x0 and 0xF. The DLC no longer directly indicate the payload size, but an index in the array of possible payload sizes.

=== ============
DLC Payload Size
--- ------------
0x0      0
0x1      1
0x2      2
0x3      3
0x4      4
0x5      5
0x6      6
0x7      7
0x8      8
0x9      12
0xA      16
0xB      20
0xC      24
0xD      32
0xE      48
0xF      64
=== ============




Bitrate switching
-----------------

When the "bitrate switching" (BRS) flag of a CAN-FD frame is active, the "arbitration" phase and the "data" phase of the transmission of a CAN frame will use different bitrates.
The "arbitration" phase is also referred as the "nominal" phase.

ESI Flag
--------

