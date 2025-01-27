PCB Ordering
============

Work In Progress.


Production notes
----------------

BOM component references
########################

Expansions (1_screens to 5_debugger) use non-overlapping references and can easily be panelized together. 
The main board (0_ramn) has components references that overlap with the expansions, and some fabs may refuse panelization because of that reason.


Clarifications
##############

Below are clarifications based on past questions from fabs.

- Orientation of D8 on 4_body PCB is as specified by the graphics at the bottom-right of the PCB's silkscreen (same orientation for all LEDs; K up and A down). The dot near the bottom of D8 is just art, it does not indicate the cathode.
- Y1, Y2, Y3, Y4 do not have orientation (they are passive crystals, despite their unusual footprint).
- RK09D1110C0R may not be easily available. RK09K1110B26 and RK09K1110A2S are acceptable alternative, but they may need small adjustments to prevent the knob from hitting the edge of the PCB. The knob may also come off more easily as a result.
- SSQ-113-23-G-D can be replaced with SSQ-113-23-L-D, SSQ-113-23-F-D, or SSQ-113-23-T-D. They can also be replaced with 4UCON TECHNOLOGY 18507, but those have softer pins, which are more prone to bending.
- LEDs can be replaced with SML-D12P8WT86, SML-D12D1WT86, and SML-D12V1WT86 (depending on color).
- It is acceptable to replace most components with alternatives. STM32L562CET6 and STM32L552CET6 are the preferred choices for microcontrollers. STM32L562CET6 is the same as STM32L552CET6 but has an encryption engine (not required by RAMN, but appreciated by user when available; may be more difficult to import/export due to restrictions).  STM32L562CCT6 is compatible but does not support some minor features, such as UDS reprogramming. 


Notes from past issues
######################

ECU A's screen (external-1 in BOM) is rather fragile and requires careful handling and packaging.

We have had past production issues with badly soldered potentiometers (RV1 and RV2).
Similarly, we have had one rare occurrence of a defective SW2 (OS102011MA1QN1), probably also due to bad soldering.
Those should require extra attention during soldering and quality check.

