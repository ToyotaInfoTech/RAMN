*** Settings ***
Documentation   Full test suite using Renode for RAMN firmwares.
Resource        RAMN.resource
Test Setup      RAMN Test Setup
Suite Setup     RAMN Suite Setup


*** Variables ***
# Cortex-M33 thumb shellcode: write 0xDEADBEEF to 0x20000000 and return.
# This is plain machine code, reproduce it from source with:
#
#   cat > sc.s <<'EOF'
#   .syntax unified
#   .thumb
#   .thumb_func
#   _start:
#       movw r0, #0xBEEF
#       movt r0, #0xDEAD        @ r0 = 0xDEADBEEF
#       movw r1, #0x0000
#       movt r1, #0x2000        @ r1 = 0x20000000
#       str  r0, [r1]           @ *0x20000000 = 0xDEADBEEF
#       bx   lr                 @ return to the UDS handler
#   EOF
#   arm-none-eabi-as -mcpu=cortex-m33 -mthumb sc.s -o sc.o
#   arm-none-eabi-objcopy -O binary sc.o sc.bin
#   xxd -p sc.bin
${UDS_SHELLCODE}                        4BF6EF60CDF6AD6040F20001C2F2000108607047


*** Test Cases ***
Should Provide Started Platform
    [Documentation]     Boot the 4-ECU platforms and run the emulation to skip startup sequence.

    Execute Command     emulation RunFor "5s"
    Provides            post-startup

ECUA Should Display Actuators Changes
    [Documentation]         Test that ECU{C,B,D} communicate to ECUA over CAN and that new actuator values are
    ...                     reflected on screen.

    Prepare RAMN With Frame Buffer Tester   fb_tester

    # Arbitrary sequence of commands executed before saving the reference frame
    Execute Command         adc1.engineKey CurrentState "middle"        machine=ECUD
    Execute Command         adc1.parkingBrake CurrentState "on"         machine=ECUB
    Execute Command         adc1.wheel Percentage 50                    machine=ECUB
    Execute Command         adc1.lightingSwitch CurrentState "pos4"     machine=ECUB
    Execute Command         adc1.brake Percentage 33                    machine=ECUC
    Execute Command         adc1.accel Percentage 100                   machine=ECUC
    Execute Command         adc1.joystick CurrentState "left"           machine=ECUC

    Execute Command         fb_tester WaitForFrame @${CURDIR}/ramn_screen_uds.png

ECUs Should Respond To Tester Present UDS Command
    # 0x3E Tester Present: keep-alive used to stop a diagnostic session from timing out
    Restore Started Platform
    FOR  ${ECU}  IN  ECUA  ECUB  ECUC  ECUD
        # Sub-function 0x00 requests a positive response (0x80 would suppress it)
        ${response}=  Send UDS Command To ECU  ${ECU}  0x3E  00
        Should Be Equal  ${response}  7E00
    END

ECUs Should Respond To Diagnostic Session Control UDS Command
    # 0x10 Diagnostic Session Control: switch the ECU into a diagnostic session that unlocks further services
    # Session type 0x04 (safety system) is skipped - it requires additional setup
    FOR  ${ECU}  IN  ECUA  ECUB  ECUC  ECUD
        # Switching the session is a state change, so restore a clean platform per ECU
        Restore Started Platform
        # 0x01 default, 0x02 programming, 0x03 extended; the response echoes the session type
        FOR  ${session}  IN  01  02  03
            ${response}=  Send UDS Command To ECU  ${ECU}  0x10  ${session}
            Should Start With  ${response}  50${session}
        END
    END

ECUs Should Respond To ECU Reset UDS Command
    # 0x11 ECU Reset
    FOR  ${ECU}  IN  ECUA  ECUB  ECUC  ECUD
        # Reset mutates ECU state, so restore test to a clean state per ECU under test
        Restore Started Platform
        # Sub-function 0x01 = hard reset
        ${response}=  Send UDS Command To ECU  ${ECU}  0x11  01  session=02
        Should Start With  ${response}  5101
    END

ECUs Should Respond To Read Data By Identifier UDS Command
    # 0x22 Read Data By Identifier: read one 16-bit DID per request
    Restore Started Platform
    FOR  ${ECU}  IN  ECUA  ECUB  ECUC  ECUD
        # DID 0xF184 = firmware compile-time string; response echoes 62 + the DID + the data
        ${response}=  Send UDS Command To ECU  ${ECU}  0x22  F184
        Should Start With  ${response}  62F184
    END

ECUs Should Respond To Read Memory By Address UDS Command
    # 0x23 Read Memory By Address: read arbitrary ECU memory
    Restore Started Platform
    FOR  ${ECU}  IN  ECUA  ECUB  ECUC  ECUD
        # data[1]=0x14: high nibble 1 = 1-byte length field, low nibble 4 = 32-bit address;
        # read 4 bytes from flash base 0x08000000
        ${response}=  Send UDS Command To ECU  ${ECU}  0x23  140800000004  session=02

        # Verify the read-out bytes match what Renode reads straight off that ECU's bus. Both
        # Read Memory By Address and ReadBytes return bytes in ascending-address order, so the
        # UDS payload (response without its 0x63 service byte) must equal the bus bytes
        Execute Command  mach set "${ECU}"
        ${bus_bytes}=  Execute Command  sysbus ReadBytes 0x08000000 4
        ${expected}=  Evaluate  "63" + "".join("%02X" % b for b in ${bus_bytes})
        Should Be Equal  ${response}  ${expected}
    END

ECUs Should Respond To Control DTC Settings UDS Command
    # 0x85 Control DTC Settings: temporarily enable/disable DTC storage
    FOR  ${ECU}  IN  ECUA  ECUB  ECUC  ECUD
        # Mutates global platform state, restore clean state per ECU
        Restore Started Platform
        # 0x01 = on (0x02 = off)
        ${response}=  Send UDS Command To ECU  ${ECU}  0x85  01
        Should Start With  ${response}  C501
    END

ECUs Should Respond To Link Control UDS Command
    # 0x87 Link Control: stage/apply a CAN baudrate change
    FOR  ${ECU}  IN  ECUA  ECUB  ECUC  ECUD
        # Mutates global platform state, restore clean state per ECU
        Restore Started Platform
        # 0x01 verifyBaudrateTransitionWithFixedBaudrate, 0x12 = 500000
        ${response}=  Send UDS Command To ECU  ${ECU}  0x87  0112
        Should Start With  ${response}  C701
    END

ECUs Should Respond To Routine Control UDS Command
    # 0x31 Routine Control: start (sub-function 0x01) a routine by 16-bit ID.
    Restore Started Platform
    FOR  ${ECU}  IN  ECUA  ECUB  ECUC  ECUD
        # Routine 0x0203 = "echo message": no security access required; no side-effects; response echoes 71 01 0203
        ${response}=  Send UDS Command To ECU  ${ECU}  0x31  010203
        Should Start With  ${response}  71010203
    END

ECUs Should Grant Security Access UDS Command
    # 0x27 Security Access: challenge-response unlock
    FOR  ${ECU}  IN  ECUA  ECUB  ECUC  ECUD
        # Unlocking changes ECU security state, so restore a clean platform per ECU
        Restore Started Platform
        Grant Security Access To ECU  ${ECU}
    END

ECUA Should Accept Display Pixels Custom UDS Command
    # Custom service 0x41: draw pixels on the ECU screen
    Restore Started Platform
    # 2x2 RGB565 rectangle at (0,0); 2*2*2 = 8 pixel bytes
    Send UDS Command To ECU  ECUA  0x41  000002020000000000000000

ECUA Should Execute Shellcode Via Routine Control UDS Command
    # 0x31 Routine Control routine 0x0209: execute caller-supplied ARM code
    Restore Started Platform
    Grant Security Access To ECU  ECUA

    ${send_id}=  Evaluate    ${UDS_RX_ID}[ECUA]
    ${recv_id}=  Evaluate    ${UDS_RX_ID}[ECUA] + ${UDS_TX_ID_OFFSET}

    # Run the shellcode (0x0209)
    Send UDS Command And Wait For Positive Response    ${send_id}  ${recv_id}  0x31  010209${UDS_SHELLCODE}  timeout=2

    # Read those 4 bytes back via Read Memory By Address; RAM is little-endian, so 0xDEADBEEF
    # reads back as EF BE AD DE -> a full response of 63 EF BE AD DE proves the code executed
    Send ISOTP Message       ${send_id}  ${recv_id}  23142000000004
    ${mem_hex}=  Wait For ISOTP Message Hex    ${send_id}  ${recv_id}  timeout=1
    Should Be Equal          ${mem_hex}  63EFBEADDE


*** Keywords ***
Restore Started Platform
    Requires                 post-startup
    Create CAN Tester        canHub  1

Send UDS Command To ECU
    [Arguments]              ${ECU}  ${service}  ${data}  ${session}=01
    ${send_id}=  Evaluate    ${UDS_RX_ID}[${ECU}]
    ${recv_id}=  Evaluate    ${send_id} + ${UDS_TX_ID_OFFSET}

    IF  $session != "01"
        # Command needs to switch session type first
        Send UDS Command And Wait For Positive Response    ${send_id}  ${recv_id}
        ...                                                ${UDS_SESSION_CONTROL}  ${session}  timeout=1
    END

    ${response}=  Send UDS Command And Wait For Positive Response    ${send_id}  ${recv_id}
    ...                                                              ${service}  ${data}  timeout=1
    RETURN  ${response}

Grant Security Access To ECU
    [Arguments]              ${ECU}
    ${send_id}=  Evaluate    ${UDS_RX_ID}[${ECU}]
    ${recv_id}=  Evaluate    ${send_id} + ${UDS_TX_ID_OFFSET}

    # Security access is only meaningful in a non-default session
    Send UDS Command And Wait For Positive Response    ${send_id}  ${recv_id}  ${UDS_SESSION_CONTROL}  02  timeout=1

    # Request the seed, ECU responds with "6701" + 4 seed bytes
    Send ISOTP Message       ${send_id}  ${recv_id}  2701
    ${seed_hex}=             Wait For ISOTP Message Hex    ${send_id}  ${recv_id}  timeout=1
    Should Start With        ${seed_hex}  6701

    # In the current RAMN ECUn binaries key = xor(seed,0x12345678) - see `RAMN_UDS_RequestSeed` and `RAMN_UDS_TryKey`
    ${key}=  Evaluate        "2702%08X" % (int($seed_hex[4:12], 16) ^ 0x12345678)
    Send ISOTP Message       ${send_id}  ${recv_id}  ${key}
    ${resp_hex}=             Wait For ISOTP Message Hex    ${send_id}  ${recv_id}  timeout=1
    Should Start With        ${resp_hex}  6702
