/*
 * stusb4500.h - Driver for the ST STUSB4500 standalone USB-PD sink
 * controller (original AxxSolder hardware).
 *
 * The STUSB4500 is an AUTONOMOUS sink: its internal policy engine runs the
 * whole PD negotiation (Source_Capabilities -> Request -> Accept -> PS_RDY)
 * with no MCU involvement. Firmware can only:
 *   - program up to 3 sink PDOs in RAM (DPM_SNK_PDO1..3, priority
 *     PDO3 > PDO2 > PDO1, PDO1 voltage fixed at 5 V by the PD spec) and
 *     trigger a renegotiation with a PD Soft_Reset message,
 *   - read back the negotiated contract (RDO_REG_STATUS) and chip status,
 *   - read/write the NVM configuration over I2C (FTP interface).
 *
 * PROVEN ON HARDWARE (do not re-attempt): the source's Source_Capabilities
 * message CANNOT be captured from the RX buffer - the policy engine consumes
 * it and issues its own Request before firmware can poll it out, at 100 kHz
 * and at 400 kHz I2C alike. Only the trailing Accept is ever caught. The
 * former capture path (poll_source/stusb_capture_source_pdos) was dead code
 * and has been removed. Injecting Get_Source_Cap via the TX header is also
 * known to hard-reset sources (ST community) - only Soft_Reset is safe.
 *
 * Max-power strategy instead relies on the NVM REQ_SRC_CURRENT bit: once set
 * (one-time provisioning, see stusb_nvm_provision), the chip requests the
 * SOURCE PDO's full advertised current at the matched voltage, so the
 * sink-PDO currents become minimum-match floors and the granted current is
 * read back from the RDO. This is ST's recommended architecture for "book the
 * maximum the charger offers" (UM2650 / ST community). The chip loads its NVM
 * only at power-up, so a freshly written bit is live from the next power
 * cycle on.
 */

#ifndef __STUSB4500_H
#define __STUSB4500_H

#include "stm32g4xx_hal.h"
#include "stdbool.h"

#define STUSB4500_ADDR         0x28

/* --- identity --- */
#define REG_DEVICE_ID          0x2F
#define VAL_STUSB4500_ID       0x25   /* STUSB4500 */
#define VAL_STUSB4500_ID_B     0x21   /* STUSB4500B variant */

/* --- status / control --- */
#define REG_ALERT_STATUS_1      0x0B
#define REG_ALERT_STATUS_1_MASK 0x0C
#define REG_PORT_STATUS_0       0x0D
#define REG_PORT_STATUS_1       0x0E
#define REG_PE_FSM              0x29   /* policy-engine state */
#define REG_TX_HEADER_LOW       0x51
#define REG_PD_COMMAND_CTRL     0x1A
#define REG_RDO_REG_STATUS      0x91   /* negotiated contract (4 bytes) */

#define CMD_SOFT_RESET          0x0D   /* PD Soft_Reset message (renegotiate, no VBUS drop) */
#define CMD_SEND_COMMAND        0x26

#define VAL_PRT_STATUS_AL       0x02
#define VAL_PE_SNK_READY        0x18   /* PE_FSM state code, compare with == */
#define VAL_SNK_ATT             0xE0

/* --- volatile (RAM) sink-PDO registers ---
 * Loaded from NVM at boot, freely rewritable at runtime, applied by a PD
 * Soft_Reset. */
#define DPM_PDO_NUMB           0x70
#define DPM_SNK_PDO1           0x85

/* --- NVM (FTP) interface ---
 * 5 sectors x 8 bytes. Register sequence per the ST reference code
 * (USBPD_CUST_NVM_API.c) / SparkFun library - field-proven at scale. */
#define FTP_CUST_PASSWORD_REG  0x95
#define FTP_CUST_PASSWORD      0x47
#define FTP_CTRL_0             0x96
#define FTP_CTRL_1             0x97
#define FTP_RW_BUFFER          0x53
/* FTP_CTRL_0 bits */
#define FTP_CUST_PWR           0x80
#define FTP_CUST_RST_N         0x40
#define FTP_CUST_REQ           0x10
#define FTP_CUST_SECT_MASK     0x07
/* FTP_CTRL_1 opcodes (low 3 bits); high 5 bits = sector-erase mask (SER) */
#define FTP_OPCODE_READ        0x00
#define FTP_OPCODE_WRITE_PL    0x01
#define FTP_OPCODE_WRITE_SER   0x02
#define FTP_OPCODE_ERASE       0x05
#define FTP_OPCODE_PROG_SECTOR 0x06
#define FTP_OPCODE_SOFT_PROG   0x07

#define STUSB_NVM_SECTORS      5
#define STUSB_NVM_SECTOR_SIZE  8
/* REQ_SRC_CURRENT lives in NVM sector 4, byte 6, bit 4 (DS12499 / SparkFun) */
#define STUSB_NVM_REQ_SRC_CURRENT_SECTOR 4
#define STUSB_NVM_REQ_SRC_CURRENT_BYTE   6
#define STUSB_NVM_REQ_SRC_CURRENT_BIT    0x10
/* Write+verify attempts per boot before giving up (NVM endurance is
 * "a few thousand" cycles; a healthy chip is written once per lifetime). */
#define STUSB_NVM_WRITE_ATTEMPTS         3

/* Set to 0 to never write the NVM. A chip that is not already provisioned
 * then always negotiates with the fallback sink-PDO sets. */
#ifndef STUSB_NVM_PROVISION
#define STUSB_NVM_PROVISION              1
#endif

/* Negotiated contract (RDO_REG_STATUS 0x91). NOTE: Object_Pos indexes the
 * SOURCE's advertised PDO list, which this chip never exposes - it cannot be
 * mapped to a voltage. Branch on measured VBUS instead. OperatingCurrent /
 * MaxCurrent are in 10 mA units; with NVM REQ_SRC_CURRENT set the granted
 * OperatingCurrent equals the source PDO's full advertised current.
 * (All fields are uint32_t so GCC packs them into one 32-bit word; the
 * struct is filled by a raw 4-byte little-endian read.) Object_Pos == 1 is
 * always the source's 5 V PDO. */
typedef union {
  uint32_t d32;
  struct {
    uint32_t MaxCurrent                     :       10; //Bits 9..0
    uint32_t OperatingCurrent               :       10; //Bits 19..10
    uint32_t reserved_22_20                 :       3;
    uint32_t UnchunkedMess_sup              :       1;
    uint32_t UsbSuspend                     :       1;
    uint32_t UsbComCap                      :       1;
    uint32_t CapaMismatch                   :       1;
    uint32_t GiveBack                       :       1;
    uint32_t Object_Pos                     :       3; //Bits 30..28 (3-bit)
    uint32_t reserved_31                    :       1; //Bits 31
  } b;
} STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef;
_Static_assert(sizeof(STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef) == 4, "RDO must be one 32-bit word");

/* Sink-PDO register layout (DPM_SNK_PDOx, fixed-supply form). uint32_t
 * throughout: a uint8_t field may not straddle a byte, which used to push
 * USB_Communications_Capable from bit 26 to 27 and the union to 8 bytes. */
typedef union {
  uint32_t d32;
  struct {
    uint32_t Operationnal_Current :10;  //Bits 9..0
    uint32_t Voltage :10;               //Bits 19..10
    uint32_t Reserved_22_20  :3;
    uint32_t Fast_Role_Req_cur : 2;     /* must be set to 0 in 2.0*/
    uint32_t Dual_Role_Data    :1;
    uint32_t USB_Communications_Capable :1;  //Bit 26
    uint32_t Unconstrained_Power :1;
    uint32_t Higher_Capability :1;
    uint32_t Dual_Role_Power :1;
    uint32_t Fixed_Supply :2;           //Bits 31..30
  } fix;
} USB_PD_SNK_PDO_TypeDef;
_Static_assert(sizeof(USB_PD_SNK_PDO_TypeDef) == 4, "sink PDO must be one 32-bit word");

/* --- chip primitives (all return HAL status; nothing fails silently) --- */

/* Device-ID probe; accepts the STUSB4500 and STUSB4500B variants. */
HAL_StatusTypeDef stusb_check_connection(void);
HAL_StatusTypeDef stusb_init(void);
/* USB-C source attached: 1 = yes, 0 = no, -1 = I2C read failed. */
int8_t stusb_sink_attached(void);
/* Policy engine reports an established explicit contract (PE_SNK_READY). */
bool stusb_is_sink_ready(void);
/* Read the negotiated RDO. *rdo is zeroed first; on I2C failure it stays
 * zeroed (Object_Pos == 0 == "no contract") and != HAL_OK is returned. */
HAL_StatusTypeDef stusb_read_rdo(STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef *rdo);
/* Program all three volatile sink PDOs + the valid count in one call.
 * Index 0 = PDO1 (voltage forced to 5 V), 1 = PDO2, 2 = PDO3 (highest
 * priority). Takes effect at the next soft reset. */
HAL_StatusTypeDef stusb_set_sink_pdos(const uint16_t mv[3], const uint16_t ma[3]);
/* Patch a single sink PDO (1..3; PDO1 voltage forced to 5 V) and set the
 * number of valid PDOs (1..3); used by the probe ladder. Take effect at the
 * next soft reset. */
HAL_StatusTypeDef stusb_update_pdo(uint8_t pdo_number, uint16_t voltage_mv, uint16_t current_ma);
HAL_StatusTypeDef stusb_set_valid_pdo(uint8_t valid_count);
/* Send a PD Soft_Reset message: renegotiates with the current sink PDOs.
 * VBUS dips during renegotiation but is not dropped. */
HAL_StatusTypeDef stusb_soft_reset(void);

/* --- NVM provisioning (one-time, idempotent) --- */

typedef enum {
	STUSB_NVM_ERROR = 0,   /* read unstable/failed, unknown layout or write failed */
	STUSB_NVM_CLEAR,       /* REQ_SRC_CURRENT clear, writing disabled (STUSB_NVM_PROVISION 0) */
	STUSB_NVM_SET,         /* already set at boot */
	STUSB_NVM_WRITTEN      /* written and verified now; live from the next power-up */
} stusb_nvm_state_t;

/* Check the NVM REQ_SRC_CURRENT bit and set it if clear. The NVM is read
 * twice and must match; a non-blank image is only modified if sector 0 holds
 * the known ST constant. A blank or partially erased image (interrupted
 * earlier write) is rebuilt from ST factory defaults. Writes go read-modify-
 * write from the RAM image, verified by read-back, up to
 * STUSB_NVM_WRITE_ATTEMPTS times. Never resets the chip. Works without a
 * USB-C source attached. */
stusb_nvm_state_t stusb_nvm_provision(void);

#endif
