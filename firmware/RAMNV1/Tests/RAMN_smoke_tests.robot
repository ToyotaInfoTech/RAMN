*** Settings ***
Documentation   Smoke tests using Renode for RAMN firmwares.
Resource        RAMN.resource
Test Setup      RAMN Test Setup
Suite Setup     RAMN Suite Setup


*** Test Cases ***
Should Produce CAN Traffic
    [Documentation]     Using Renode's log tester to ensure that ECUB, ECUC and ECUD are producing
    ...                 CAN messages.

    Create Log Tester   timeout=1
    Execute Command     logLevel 0 canHub

    FOR     ${ECU}  IN  ECUB    ECUC    ECUD
        Wait For Log Entry  canHub: Received from ${ECU}
    END

ECUA Should Display Boot Menu
    [Documentation]         Using Renode's frame buffer tester to ensure that boot menu is displayed

    Prepare RAMN With Frame Buffer Tester   fb_tester
    Execute Command         fb_tester WaitForFrame @${CURDIR}/ramn_boot_menu.png

Should Provide Started Platform
    [Documentation]     Boot the 4-ECU platforms and run the emulation to skip startup sequence

    Execute Command     emulation RunFor "5s"
    Provides            post-startup

Brake Should Affect Brake LED
    [Documentation]     Using Renode's LED tester to validate the data path ECUC's ADC --> ECUC's
    ...                 CAN --> ECUD's CAN --> ECUD's SPI.

    Requires            post-startup

    Create LED Tester   sysbus.spi2.ledController.parkingBrake  machine=ECUD
    Execute Command     adc1.brake SetPercentage 0              machine=ECUC
    Execute Command     adc1.parkingBrake CurrentState "off"    machine=ECUB

    Assert And Hold Led State   false   timeoutAssert=0     timeoutHold=${LED_HOLDING_TIMEOUT}
    Execute Command     adc1.brake SetPercentage 80              machine=ECUC
    Assert And Hold Led State   true   timeoutAssert=1     timeoutHold=${LED_HOLDING_TIMEOUT}

ECUs Should Respond To Tester Present UDS Command
    [Documentation]     Using Renode's CAN Tester to test UDS reception

    ${TESTER_PRESENT_DATA} =    Set Variable    00
    ${TESTER_PRESENT_RESP} =    Evaluate        ${UDS_TESTER_PRESENT} + ${POSITIVE_RESPONSE_OFFSET}
    ${EXPECTED_RESP} =          Convert To Hex  ${TESTER_PRESENT_RESP}

    Requires                    post-startup
    Create CAN Tester           canHub  1

    FOR     ${ECU}  IN  ECUA    ECUB    ECUC    ECUD
        ${send_id} =        Evaluate    ${UDS_RX_ID}[${ECU}]
        ${recv_id} =        Evaluate    ${send_id} + ${UDS_TX_ID_OFFSET}
        ${response} =       Send UDS Command And Wait For Positive Response     ${send_id}
        ...                 ${recv_id}  service=${UDS_TESTER_PRESENT}  data=${TESTER_PRESENT_DATA}

        Should Be Equal     ${response}     ${EXPECTED_RESP}${TESTER_PRESENT_DATA}
    END
