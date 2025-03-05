PCB Ordering
============

The version of RAMN hardware in active development (on the `main branch <https://github.com/ToyotaInfoTech/RAMN/tree/main/hardware>`_) is aimed at ease of fabrication and cost.
The original version of RAMN is archived and is available in the `misc folder <https://github.com/ToyotaInfoTech/RAMN/tree/main/misc>`_.
This archived version uses some parts which are not easy to source anymore from turnkey fabrication services and/or are not cost effective for orders by RAMN users.

- If your goal is to make a RAMN cheaply and easily, use the information on this page.
- If your goal is to obtain the exact original hardware (e.g., for academic settings), download `RAMN_V1_reference_PCB.zip <https://github.com/ToyotaInfoTech/RAMN/blob/main/misc/RAMN_V1_reference_PCB.zip>`_ and send it to a PCB assembly prototyping service.

.. warning:: 

	**This page is provided for convenience only. We cannot guarantee the quality of your order**.

.. image:: img/1000015167.jpg
   :align: center


The design files are complete enough that you can fabricate and/or assemble your own RAMN and expansions from any PCB prototyping service.

For convenience, there are two 're-orders' that can be made from PCBWay to get your own RAMN: 

1. The `mainboard <https://www.pcbway.com/project/shareproject/https_github_com_ToyotaInfoTech_RAMN_mainboard_ONLY_0dae9b72.html>`_.
2. The `expansions <https://www.pcbway.com/project/shareproject/https_github_com_ToyotaInfoTech_RAMN_pods_ONLY_3ff2c7a7.html>`_ (excluding the optional debugger expansion).

The mainboard is usable for many applications all by itself, but acquiring the expansions is required for changing the simulated vehicle state e.g. driving the car in CARLA.

On PCBWay, you have two purchase options:

- **PCB+Assembly**: you'll get PCBs with components already soldered (**pictures below**).
- **Only PCB**: you must order components in the BOM and solder them yourself (cheaper option).


.. image:: img/1000015167-mainboard.png
   :align: center


.. button-link:: https://www.pcbway.com/project/shareproject/https_github_com_ToyotaInfoTech_RAMN_mainboard_ONLY_0dae9b72.html
    :color: primary
    :expand:

    Mainboard from PCBWay

.. image:: img/1000015167-expansions.png
   :align: center


.. button-link:: https://www.pcbway.com/project/shareproject/https_github_com_ToyotaInfoTech_RAMN_pods_ONLY_3ff2c7a7.html
    :color: secondary
    :expand:

    Expansions Panel from PCBWay

The bill of materials used in the above orders were tailored to match
the components that are available from PCBWay, and some parts are
removed (marked DNP) to reduce costs (e.g. test points).

.. warning::

	The PCBWay re-order uses **STM32L562CET6** microcontrollers. These microcontrollers feature a **hardware cryptographic engine**, which means that they may be subject to import/export restrictions.
	If these restrictions apply to you, you should request to replace **STM32L562CET6** with **STM32L552CET6**, which is the same microcontroller but without the cryptographic engine (see :ref:`microcontroller_sel`).
	RAMN's firmware does not use this engine.

Parameters
----------

You should keep most of the default parameters: 

- **FR4 material**
- **2 layers**
- **1.6mm thickness**
- **1oz (35um) copper**

You should be able to select any surface finish, although we suggest that you stay away from any solution with lead.
**"HASL lead free"** is typically the cheapest solution without lead, but may tarnish over time.
We typically use **"Immersion gold (ENIG)"**, and haven't tried other surface finishes ourselves.

**"Solder Mask"** corresponds to the color of the board, and **"Silkscreen"** corresponds to the color of the text and lines.
You can select any color you like, although you should ensure enough contrast to read the text (e.g., do not select white soldermask with white silkscreen).

With PCBWay, we recommend that you select one of these colors for the soldermask: **green, red, yellow, blue, purple, or matte green**.
If you select a **black, white, or matte black** soldermask, PCBWay may warn you that the soldermask constraints of RAMN cannot be respected (the soldermask spacing between microcontroller pins and USB connector pins is 0.20mm, but PCBWay asks for at least 0.22mm).
If you want to use one of these colors anyway, it is typically OK to ask PCBWay to ignore the issue, or to remove problematic soldermask themselves. This will however slightly increase the probability of soldering issues.

Ordering Fewer Than a Quantity of Five (5)
------------------------------------------

The PCBWay ordering system might make it seem like ordering 5 or more RAMN is required; however, only (bare) PCBs must be ordered at a minimum of 5. It is possible to order as little as 1 assembled mainboard and/or expansions panel.
If you only want 1 RAMN, you can set "5" in the number of PCBs, and "1" in the number of assemblies, as shown in the screenshot below.

.. image:: img/0121A.png
   :align: center

The more boards you order, the cheaper they become.
If you order 5 assembled RAMN sets, the expected cost is approximately 210 USD per full RAMN set (assuming a green soldermask and ENIG surface finish, and without the optional debugger expansion).
If you order only one set, that cost however becomes 430 USD. If you order 100 sets, it becomes about 140 USD.

Additional Components
---------------------

The following components are not in the PCBWay order, because they are not absolutely needed.
If you want them, consider requesting them to PCBWay or ordering them separately yourself:

- 1x Micro type B USB cable: any maker, but avoid power-only cables.
- 1x Terminal block (for external CAN/CAN-FD tools): **Phoenix Contact	1770966**.
- Test probes: **Vero Technologies 20-313143** (as many as you need).
- 1x Steering wheel potentiometer knob: **Davies Molding 1231-M**.
- 4x Hex spacers: **Keystone 24313** (male) and **Keystone 24390** (female) (8 pieces total).

Only the terminal block requires soldering.
Note that the Davies Molding 1300-F knobs used in the original RAMN are **not** compatible with the current order on PCBWay.

Alternative Components
----------------------

Components availability and cost may vary.

- **RK09D1110C0R** may not be easily available.
  **RK09K1110B26** and **RK09K1110A2S** are acceptable alternative, but they may need small adjustments to prevent the knob from hitting the edge of the PCB. The knob may also come off more easily as a result.
- **SSQ-113-23-G-D** can be replaced with **SSQ-113-23-L-D**, **SSQ-113-23-F-D**, or **SSQ-113-23-T-D**.
  They can also be replaced with **4UCON TECHNOLOGY 18507**, but those have softer pins, which are more prone to bending.
- LEDs can be replaced with **SML-D12P8WT86**, **SML-D12D1WT86**, and **SML-D12V1WT86** (depending on color). If you use other LEDs, make sure they have a similar nominal current.


Production Notes
----------------

BOM component references
########################

Expansions (1_screens to 5_debugger) use non-overlapping references and can easily be panelized together.
The main board (0_ramn) has components references that overlap with the expansions, and some fabs may refuse panelization because of that reason.


Clarifications
##############

Below are clarifications based on past questions from fabs:

- Orientation of D8 on 4_body PCB is as specified by the graphics at the bottom-right of the PCB's silkscreen (same orientation for all LEDs; K up and A down). The dot near the bottom of D8 is just art, it does not indicate the cathode.
- Y1, Y2, Y3, Y4 do not have orientation (they are passive crystals, despite their unusual footprint).


Notes from Past Issues
######################

ECU A's screen (external-1 in BOM) is rather fragile and requires careful handling and packaging.

We have had past production issues with badly soldered potentiometers (RV1 and RV2).
Similarly, we have had one rare occurrence of a defective SW2 (OS102011MA1QN1), probably also due to bad soldering.
Those should require extra attention during soldering and quality check.
