#ifndef __STUSB4500_H
#define __STUSB4500_H

#include "stm32g4xx_hal.h"
#include "stdbool.h"

#define STUSB4500_ADDR         0x28

#define REG_DEVICE_ID 0x2F
#define REG_RDO_REG_STATUS         0x91
#define REG_ALERT_STATUS_1	0x0B
#define REG_ALERT_STATUS_1_MASK	0x0C
#define REG_PORT_STATUS_0		   0x0D
#define REG_PORT_STATUS_1		   0x0E
#define REG_CC_STATUS			   0x11
#define REG_TX_HEADER_LOW			0x51
#define REG_PD_COMMAND_CTRL			0x1A
#define REG_PRT_STATUS        0x16
#define REG_RX_HEADER         0x31
#define REG_RX_DATA_OBJS      0x33
#define REG_TYPEC_MONITORING_STATUS_1 0x10
#define REG_PE_FSM	0x29

#define CMD_SOFT_RESET			  0x0D
#define CMD_SEND_COMMAND		0x26

#define VAL_STUSB4500_ID 0x25
#define VAL_STUSB4500_ID_B 0x21
#define VAL_PRT_STATUS_AL          0x02
#define VAL_MSG_RECEIVED           0x04
#define VAL_SOURCE_CAPABILITIES    0x01
#define VAL_PE_SNK_READY 0x18
#define VAL_SNK_ATT 0xE0

typedef union {
  uint32_t d32;
  struct {
    uint32_t MaxCurrent                     :       10; //Bits 9..0
    uint32_t OperatingCurrent               :       10;
    uint8_t reserved_22_20                  :       3;
    uint8_t UnchunkedMess_sup               :       1;
    uint8_t UsbSuspend                      :       1;
    uint8_t UsbComCap                       :       1;
    uint8_t CapaMismatch                    :       1;
    uint8_t GiveBack                        :       1;
    uint8_t Object_Pos                      :       3; //Bits 30..28 (3-bit)
    uint8_t reserved_31		                :       1; //Bits 31
  } b;
} STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef;

#define DPM_PDO_NUMB           0x70
#define DPM_SNK_PDO1           0x85

typedef union {
  uint32_t d32;
  struct {
    uint32_t Operationnal_Current :10;
    uint32_t Voltage :10;
    uint8_t Reserved_22_20  :3;
    uint8_t Fast_Role_Req_cur : 2;  /* must be set to 0 in 2.0*/
    uint8_t Dual_Role_Data    :1;
    uint8_t USB_Communications_Capable :1;
    uint8_t Unconstrained_Power :1;
    uint8_t Higher_Capability :1;
    uint8_t Dual_Role_Power :1;
    uint8_t Fixed_Supply :2;
  } fix;

  struct {
    uint32_t Operating_Current :10;
    uint32_t Min_Voltage:10;
    uint32_t Max_Voltage:10;
    uint8_t VariableSupply:2;
  } var;

  struct {
    uint32_t Operating_Power :10;
    uint32_t Min_Voltage:10;
    uint32_t Max_Voltage:10;
    uint8_t Battery:2;
  } bat;

} USB_PD_SNK_PDO_TypeDef;

typedef union
{
  uint16_t d16;
  uint8_t bytes[2];
  struct
  {
    uint8_t messageType  : 5;  // Bits 0..3 Type of message: 1 = Source Capabilities
    uint8_t data_data_role	 : 1;
    uint8_t spec_rev     : 2;
    uint8_t port_power_role : 1;
    uint8_t messageID    : 3;
    uint8_t objects      : 3;  // Number of PD objects received
    uint8_t extended     : 1;
  };
} PDHeader_t;

typedef union
{
  uint8_t bytes[4];
  struct
  {
    uint32_t current    : 10;  // In units of 10mA
    uint32_t voltage    : 10;  // In units of 50mV
    uint32_t other      : 12;
  };
} PDO_t;

typedef struct
{
	PDO_t pdos[10];
	uint8_t numPDOs;
}PDO_container_t;


HAL_StatusTypeDef stusb_init();
HAL_StatusTypeDef stusb_soft_reset();
HAL_StatusTypeDef stusb_check_connection();
bool stusb_is_sink_ready();
bool stusb_is_sink_connected();
bool stusb_set_highest_pdo(uint8_t *maxPower, uint8_t currentPdoIndex);
bool poll_source();
HAL_StatusTypeDef stusb_read_rdo(STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef *Nego_RDO);
HAL_StatusTypeDef stusb_update_pdo(uint8_t pdo_number, uint16_t voltage_mv, uint16_t current_ma);
HAL_StatusTypeDef stusb_set_valid_pdo(uint8_t valid_count);

#endif
