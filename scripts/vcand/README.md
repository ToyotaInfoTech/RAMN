
# RAMN VCAND

This folder contains experimental scripts to connect RAMN to a socketCAN virtual CAN Network, while keeping the original serial interface accessible.

This feature is only available on Linux. It is similar to [slcand](https://github.com/linux-can/can-utils/blob/master/slcand.c), but with additional features:
- Supports CAN-FD
- Can forward the original serial I/O to one or two virtual serial port(s) (e.g., */dev/pts/x*)

Refer to [RAMN's documentation](https://ramn.readthedocs.io/) for details.

# Other

*Copyright (c) 2024 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*