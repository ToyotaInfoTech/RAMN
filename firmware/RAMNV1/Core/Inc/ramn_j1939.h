#ifndef INC_RAMN_J1939_H_
#define INC_RAMN_J1939_H_

/* J1939 PGN Definitions */
#define J1939_PGN_TSC1              0       /* Torque/Speed Control 1 */
#define J1939_PGN_TC1               256     /* Transmission Control 1 */
#define J1939_PGN_EBS1              512     /* Electronic Brake System 1 */
#define J1939_PGN_XBR               1024    /* External Brake Request */
#define J1939_PGN_REQUEST           59904   /* Request PGN (0xEA00) */
#define J1939_PGN_ACK               59392   /* Acknowledgment PGN (0xE800) */
#define J1939_ACK_CONTROL_ACK       0
#define J1939_ACK_CONTROL_NACK      1
#define J1939_ACK_CONTROL_DENIED    2
#define J1939_ACK_CONTROL_BUSY      3
#define J1939_PGN_TP_DT             60160   /* TP Data Transfer (0xEB00) */
#define J1939_PGN_TP_CM             60416   /* TP Connection Management (0xEC00) */
#define J1939_PGN_ADDRESS_CLAIMED   60928   /* Address Claimed (0xEE00) */
#define J1939_PGN_PROPA             61184   /* Proprietary A (PDU1) */
#define J1939_PGN_EBC1              61441   /* Electronic Brake Controller 1 */
#define J1939_PGN_EEC2              61443   /* Electronic Engine Controller 2 */
#define J1939_PGN_EEC1              61444   /* Electronic Engine Controller 1 */
#define J1939_PGN_ETC2              61445   /* Electronic Transmission Controller 2 */
#define J1939_PGN_VDC2              61449   /* Vehicle Dynamic Stability Control 2 */
#define J1939_PGN_ESC1              61451   /* Electronic Steering Control 1 */
#define J1939_PGN_ENGINE_KEY        64960   /* PGN for Engine Key / Ignition */
#define J1939_PGN_ECU_ID            64965   /* ECU Identification Information (0xFDC5) */
#define J1939_PGN_OEL               64972   /* Operators External Light Controls */
#define J1939_PGN_CM3               64980   /* Cab Message 3 */
#define J1939_PGN_LIGHTS_CMD        65089   /* Lighting Command */
#define J1939_PGN_HORN_STATUS       65098   /* Secondary Air / Horn Status */
#define J1939_PGN_DM1               65226   /* DM1 - Active Diagnostic Trouble Codes */
#define J1939_PGN_CCVS1             65265   /* Cruise Control/Vehicle Speed 1 */
#define J1939_PGN_B1                65274   /* Brakes 1 */
#define J1939_PGN_PROPB_65280       65280   /* Proprietary B 65280 (Control_Lights) */
#define J1939_PGN_PROPB_65282       65282   /* Proprietary B 65282 (Joystick_Buttons) */

/* J1939 PDU Format (PF) values */
#define J1939_PF_ACK                0xE8    /* 232 - Acknowledgment PGN */
#define J1939_PF_REQUEST            0xEA    /* 234 - Request PGN */
#define J1939_PF_TP_DT              0xEB    /* 235 - TP Data Transfer */
#define J1939_PF_TP_CM              0xEC    /* 236 - TP Connection Management */
#define J1939_PF_ADDRESS_CLAIMED    0xEE    /* 238 - Address Claimed */

/* J1939 TP.CM Control Bytes */
#define J1939_TP_CM_RTS             0x10    /* Request To Send */
#define J1939_TP_CM_CTS             0x11    /* Clear To Send */
#define J1939_TP_CM_EOM_ACK         0x13    /* End of Message ACK */
#define J1939_TP_CM_BAM             0x20    /* Broadcast Announce Message */
#define J1939_TP_CM_ABORT           0xFF    /* Connection Abort */

/* J1939 Logical Source Addresses (SA) */
#define J1939_SA_ENGINE             0
#define J1939_SA_TRANSMISSION       3
#define J1939_SA_SHIFT_CONSOLE      5
#define J1939_SA_BRAKE_SYSTEM       11
#define J1939_SA_BRAKES_DRIVE_AXLE_1 13
#define J1939_SA_STEERING_CTRL      19
#define J1939_SA_BODY_CTRL          33
#define J1939_SA_HEADWAY_CTRL       42
#define J1939_SA_CHASSIS_CTRL_1     71
#define J1939_SA_STEERING_COLUMN    77
#define J1939_SA_POWERTRAIN_CTRL    90
#define J1939_SA_AEBS               160
#define J1939_SA_BODY_CTRL_2        166

/* J1939 Destination Addresses (DA) */
#define J1939_DA_ENGINE             0
#define J1939_DA_POWERTRAIN_CTRL    0
#define J1939_DA_TRANSMISSION       3
#define J1939_DA_BRAKE_SYSTEM       11
#define J1939_DA_STEERING_CTRL      19
#define J1939_DA_BODY_CTRL          33
#define J1939_DA_BODY_CTRL_2        166
#define J1939_DA_BROADCAST          255

/* Per-ECU primary source address for J1939 Address Claim */
#if defined(TARGET_ECUA)
#define J1939_ECU_SA                J1939_SA_HEADWAY_CTRL       /* 42 */
#elif defined(TARGET_ECUB)
#define J1939_ECU_SA                J1939_SA_STEERING_CTRL      /* 19 */
#elif defined(TARGET_ECUC)
#define J1939_ECU_SA                J1939_SA_POWERTRAIN_CTRL    /* 90 */
#elif defined(TARGET_ECUD)
#define J1939_ECU_SA                J1939_SA_BODY_CTRL          /* 33 */
#endif

/* Proprietary A Payload Definitions */
/* Command_Steering (PGN 61184): SA AEBS (160), DA Steering Controller (19). Bytes 1-2. j1939_val = ramn_val */
/* Command_Horn (PGN 61184): SA Shift Console (5), DA Body Controller (33). Byte 1. j1939_val = ramn_val & 0xFF */

#endif /* INC_RAMN_J1939_H_ */
