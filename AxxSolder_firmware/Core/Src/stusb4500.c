#include "stusb4500.h"

USB_PD_SNK_PDO_TypeDef pdo_profile[3];
extern I2C_HandleTypeDef hi2c1;

static HAL_StatusTypeDef stusb_read_register(uint8_t reg, uint8_t *data, uint8_t len)
{
  return HAL_I2C_Mem_Read(&hi2c1, (STUSB4500_ADDR << 1), reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

static HAL_StatusTypeDef stusb_write_register(uint8_t reg, uint8_t *data, uint8_t len)
{
  return HAL_I2C_Mem_Write(&hi2c1, (STUSB4500_ADDR << 1), reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

HAL_StatusTypeDef stusb_check_connection(){
	HAL_StatusTypeDef ret = HAL_ERROR;
	uint8_t chipID;
	ret = stusb_read_register(REG_DEVICE_ID, &chipID, 1);
	ret = (chipID == VAL_STUSB4500_ID) ? ret : HAL_ERROR;
	return ret;
}

HAL_StatusTypeDef stusb_init(){
	HAL_StatusTypeDef ret = HAL_ERROR;
	//clear status registers by reading
	uint8_t dummy[10];
	ret = stusb_read_register(REG_PORT_STATUS_0, dummy, 10);
	uint8_t writeVal = ~VAL_PRT_STATUS_AL;
	ret = stusb_write_register(REG_ALERT_STATUS_1_MASK, &writeVal, 1);
	return ret;
}

bool stusb_is_vbus_ready() {
  uint8_t data = 0;
  stusb_read_register(REG_TYPEC_MONITORING_STATUS_1, &data, 1);
  return (data&0x08);
}

bool stusb_is_sink_ready() {
  uint8_t data = 0;
  stusb_read_register(REG_PE_FSM, &data, 1);
  return (data&VAL_PE_SNK_READY);
}

bool stusb_is_sink_connected() {
  uint8_t data = 0;
  stusb_read_register(REG_PORT_STATUS_1, &data, 1);
  return (data&VAL_SNK_ATT);
}

HAL_StatusTypeDef stusb_read_rdo(STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef *Nego_RDO) {
  HAL_StatusTypeDef ret;
  ret = stusb_read_register(REG_RDO_REG_STATUS, (uint8_t *) Nego_RDO, 4);
  return ret;
}

HAL_StatusTypeDef stusb_update_pdo(uint8_t pdo_number, uint16_t voltage_mv, uint16_t current_ma) {
  HAL_StatusTypeDef ret;
  uint16_t addr;
  uint8_t data[40];
  uint8_t i, j = 0;

  // get existing
  addr = DPM_SNK_PDO1;
  ret = stusb_read_register(addr, data, 12);
  for (i = 0 ; i < 3 ; i++) { pdo_profile[i].d32 =  (uint32_t) data[j]
                                                 + ((uint32_t) data[j+1] << 8)
                                                 + ((uint32_t) data[j+2] << 16)
                                                 + ((uint32_t) data[j+3] << 24);
    j += 4;
  }

  // update
  if ((pdo_number == 1) || (pdo_number == 2) || (pdo_number == 3)) {
    pdo_profile[pdo_number - 1].fix.Operationnal_Current = current_ma / 10;
    if (pdo_number == 1) {
      //force 5V for PDO_1 to follow the USB PD spec
      pdo_profile[pdo_number - 1].fix.Voltage = 100; // 5000/50=100
      pdo_profile[pdo_number - 1].fix.USB_Communications_Capable = 1;
    } else {
      pdo_profile[pdo_number - 1].fix.Voltage = voltage_mv / 50;
    }

    addr = DPM_SNK_PDO1 + (4 * (pdo_number - 1));
    ret = stusb_write_register(addr, (uint8_t *) &pdo_profile[pdo_number - 1].d32, 4);
  }

  return ret;
}

HAL_StatusTypeDef stusb_set_valid_pdo(uint8_t valid_count) {
  HAL_StatusTypeDef ret = HAL_ERROR;
  if (valid_count <= 3) {
    ret = stusb_write_register(DPM_PDO_NUMB, &valid_count, 1);
  }
  return ret;
}



//re-negotiate with source via soft-reset
HAL_StatusTypeDef stusb_soft_reset(){
	uint8_t cmd[]={CMD_SOFT_RESET,0x00};
	HAL_StatusTypeDef ret = stusb_write_register(REG_TX_HEADER_LOW, cmd, 2);
	if(ret != HAL_OK){ return ret; }
	cmd[0] = CMD_SEND_COMMAND;
	return stusb_write_register(REG_PD_COMMAND_CTRL, cmd, 1);
}

//read USB-C connection status

