# ECUA: Selectable serial command transport (LPUART1 backend) & host interaction forwarding

## Overview

This pull request rebases and integrates the serial command transport abstraction (PR #19) cleanly on top of the latest `ecua-host-interaction` branch. It enables ECU A to use LPUART1 as an alternative command transport (e.g. for Renode simulation or hardware UART adapters) and connects it with `ENABLE_ECUA_HOST_INTERACTION` so that internal CAN transmissions are forwarded over the active serial transport.

The PR is organized into three atomic, logical commits on top of `ecua-host-interaction` latest (`f076127`):

### 1. `ECUA: Add transport selection config, serial command abstraction, and LPUART1 backend` (`a990bee`)
- Adds `RAMN_SERIAL_CMD_TRANSPORT_USB_CDC` and `RAMN_SERIAL_CMD_TRANSPORT_LPUART1` compile-time macros with mutual exclusion XOR validation in `ramn_config.h`.
- Introduces `ramn_serial_cmd.h` / `ramn_serial_cmd.c` providing a transport-agnostic serial output interface (`RAMN_Serial_*`) with runtime backend table registration.
- Refactors `ramn_cdc.c` CLI and sLCAN processors to use `RAMN_Serial_Send*` instead of `RAMN_USB_Send*`.
- Adds UART locking and formatted ASCII transmission helpers to `ramn_uart.h` / `ramn_uart.c`.
- Guards `ramn_cdc.h` inclusion, `ReplaceTxHeader` / `FloodTxHeader` handling, and DBC processing in `ramn_dbc.c` & `ramn_dbc.h` with `#if (defined(ENABLE_CDC) || defined(RAMN_SERIAL_CMD_TRANSPORT_LPUART1)) && defined(TARGET_ECUA)` (and `(ENABLE_USB || RAMN_SERIAL_CMD_TRANSPORT_LPUART1) && TARGET_ECUA` for DBC USB buffer processing) so that non-gateway ECUs and variants with generic `ENABLE_UART` (e.g. `uart` matrix variant) or without USB CDC (e.g. `ecua_lpuart1_transport`) compile cleanly without missing symbol, undefined reference, or implicit declaration errors.
- In `main.c`, registers `usbCdcBackend` and `lpuart1Backend` with the serial command subsystem and routes incoming UART command lines through CLI/sLCAN processors when `RAMN_SERIAL_CMD_TRANSPORT_LPUART1` is active.

### 2. `ECUA, host interaction: forward internal CAN transmissions over LPUART1 transport` (`ede7077`)
- Integrates the host interaction mechanism from commit `8025e00` with the serial command abstraction.
- In `RAMN_SendCANFunc` (`main.c`), when ECU A transmits a local frame (`CANTxHeader.MessageMarker != RAMN_CAN_ORIGIN_HOST`), the slcan frame echo is guarded by `#if (defined(ENABLE_CDC) || defined(RAMN_SERIAL_CMD_TRANSPORT_LPUART1)) && defined(TARGET_ECUA)` and transmitted via `RAMN_Serial_SendFromTask(...)`.
- *Rationale*: CAN transceivers and controllers do not loop back locally transmitted frames into their own receive FIFO. For host tools (e.g., `python-can`, Wireshark) communicating over LPUART1 to observe frames originated by ECU A itself (such as J1939 Address Claimed `0x18EEFF2A` or diagnostic responses), `RAMN_SendCANFunc` forwards them over the active serial backend.

### 3. `CI, docs: add ECUA LPUART1 transport build variant and customization guide` (`54ff2ea`)
- Adds the `ecua_lpuart1_transport` matrix variant to `.github/workflows/build_all.yml`.
- Documents transport selection compilation options and build examples in `docs/firmware/customizing_guide.rst`.
- Mentions selectable serial transports in `README.md`.

### 4. `CI, tests: add Renode Docker runtime testing for LPUART transport variants`
- Adds containerized Renode multi-node simulation harness in `scripts/renode/` (`Dockerfile`, `ramn_4ecu.resc`):
  - Uses upstream Renode commit `9d398ff439f43710e889c63c7b7fb3a0ed97a0c0` (which includes the STM32L5 NVIC priority fix and RAMN updates natively without patching).
- Adds automated runtime simulation test suite `scripts/tests/test_renode_runtime.py` using `python-can` SLCAN over `socket://127.0.0.1:29536`:
  - Physical UDS Tester Present (`0x3E`) routing to each individual ECU (`0x7E0`–`0x7E3` -> `0x7E8`–`0x7EB`).
  - Functional UDS Tester Present broadcast (`0x7DF`) routing to all ECUs.
  - Continuous background traffic forwarding across the LPUART1 serial bridge.
  - J1939 broadcast address claim negotiation when J1939 mode is active.
- Adds test orchestrator `scripts/tests/run_renode_test.sh` for local developer execution and automated GitHub Actions CI.
- Integrates the runtime simulation test into `.github/workflows/build_all.yml` `macro_coverage` for any variant enabling `RAMN_SERIAL_CMD_TRANSPORT_LPUART1` or `ENABLE_UART`.

---

## Testing & Verification

1. **Clean Headless Compilation Across CI Variants**:
   - Built all 5 ECUs (ECUA, ECUB, ECUB_LINEAR, ECUC, ECUD) with STM32CubeIDE headless GCC in Docker across both:
     - `variant: uart` (`--enable "ENABLE_UART" --disable "ENABLE_CDC ENABLE_USB"`)
     - `variant: ecua_lpuart1_transport` (`--enable "RAMN_SERIAL_CMD_TRANSPORT_LPUART1" --disable "RAMN_SERIAL_CMD_TRANSPORT_USB_CDC"`)
   - All 5 ECUs compiled and linked with 0 errors.

2. **Automated Renode 4-ECU Simulation Testing**:
   - Built simulator container via `scripts/renode/Dockerfile` targeting upstream commit `9d398ff439f43710e889c63c7b7fb3a0ed97a0c0`.
   - Executed `scripts/tests/run_renode_test.sh`:
     - `test_renode_background_traffic_forwarding`: PASSED
     - `test_renode_uds_physical_tester_present`: PASSED
     - `test_renode_uds_functional_broadcast`: PASSED
     - `test_renode_j1939_address_claims_if_active`: PASSED
   - All 4 tests passed in ~9.8 seconds against real emulated ARM Cortex-M33 hardware.


