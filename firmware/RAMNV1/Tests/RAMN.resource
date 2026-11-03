*** Settings ***
Documentation                   Keywords and variables used by RAMN robot tests.


*** Variables ***
${LED_HOLDING_TIMEOUT}          2

&{UDS_RX_ID}                    ECUA=0x7e0  ECUB=0x7e1  ECUC=0x7e2  ECUD=0x7e3
${UDS_TX_ID_OFFSET}             8
${UDS_SESSION_CONTROL}          0x10
${UDS_TESTER_PRESENT}           0x3e
${POSITIVE_RESPONSE_OFFSET}     0x40


*** Keywords ***
RAMN Suite Setup
    [Documentation]     Display warnings if default firmware are used and setup Renode.

    FOR     ${ECU}  IN  ECUA    ECUB    ECUC    ECUD
        TRY
            Log     Using ELF file ${${ECU}_ELF} for ${ECU}
        EXCEPT  Variable * not found.*      type=GLOB
            Log     Using default ${ECU} from Renode script.     level=WARN
        END
    END
    Renode Suite Setup

RAMN Test Setup
    [Documentation]     Prepare Renode with RAMN firmwares.

    Renode Test Setup

    # Using ramn.resc from Renode, that uses bin_ECU{A,B,C,D} variables to load ELF files.
    FOR     ${ECU}  IN  ECUA    ECUB    ECUC    ECUD
        TRY
            Execute Command     $global.bin_${ECU} = @${${ECU}_ELF}
        EXCEPT  Variable * not found.*      type=GLOB
            Log     Using default ${ECU} from Renode script.     level=TRACE
        END
    END
    Execute Command     include @scripts/multi-node/ramn.resc

Prepare RAMN With Frame Buffer Tester
    [Documentation]         Prepare the frame buffer tester and emulation for display tests
    [Arguments]             ${fb_tester_name}

    # As RAMN firmware is randomly choosing a theme, the seed must be set to have reproducible tests
    Execute Command         emulation SetSeed 0
    Execute Command         emulation CreateFrameBufferTester "${fb_tester_name}" 10
    Execute Command         ${fb_tester_name} AttachTo sysbus.spi2.screen   machine=ECUA
