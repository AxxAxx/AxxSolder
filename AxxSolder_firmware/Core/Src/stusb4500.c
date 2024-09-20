#include "stusb4500.h"
#include "debug.h"

USB_PD_SNK_PDO_TypeDef pdo_profile[3];
extern I2C_HandleTypeDef hi2c1;
PDO_container_t pdos;

static HAL_StatusTypeDef stusb_read_register(uint8_t reg, uint8_t *data, uint8_t len)
{
  return HAL_I2C_Mem_Read(&hi2c1, (STUSB4500_ADDR << 1), reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

static HAL_StatusTypeDef stusb_write_register(uint8_t reg, uint8_t *data, uint8_t len)
{
  return HAL_I2C_Mem_Write(&hi2c1, (STUSB4500_ADDR << 1), reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

static HAL_StatusTypeDef read_header(PDHeader_t* header){
	HAL_StatusTypeDef ret = HAL_ERROR;
	uint8_t buf[2];
	ret = stusb_read_register(REG_RX_HEADER, buf, 2);
	header->d16 = (uint16_t)buf[0] + ((uint16_t) buf[1] << 8);
	return ret;
}

static HAL_StatusTypeDef read_pdo_objects(PDHeader_t* header){
	HAL_StatusTypeDef ret = HAL_ERROR;
	pdos.numPDOs = header->objects;
	uint8_t buf[pdos.numPDOs*4];
	ret = stusb_read_register(REG_RX_DATA_OBJS, buf, pdos.numPDOs*4);
	uint8_t bufIdx = 0;
	for (int obj=0; obj<header->objects; obj++) {
		for (int i=0; i<4; i++) pdos.pdos[obj].bytes[i] = buf[bufIdx++];
	}
	debug_print_pdos(DEBUG_INFO, &pdos);
	return ret;
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

bool poll_source() {
  HAL_StatusTypeDef halStatus = HAL_ERROR;
  bool ret = false;
  uint8_t alertStatus = 0;
  halStatus = stusb_read_register(REG_ALERT_STATUS_1, &alertStatus, 1);
  if((alertStatus & VAL_PRT_STATUS_AL) != 0){
	  uint8_t prtStatus = 0;
	  halStatus = stusb_read_register(REG_PRT_STATUS, &prtStatus, 1);
	  if ((prtStatus & VAL_MSG_RECEIVED) != 0) {
		  //HAL_Delay(1);
		  PDHeader_t header;
		  read_header(&header);
		  halStatus = read_pdo_objects(&header);
		  //objects == 0 is control message
		  //debug_print_int_int(DEBUG_INFO,"msg,nO", header.messageType, header.objects);
		  if (header.objects > 0 && (header.messageType == VAL_SOURCE_CAPABILITIES)) {
			  ret = true;
		  }else{
			  pdos.numPDOs = 0;
		  }
	  }
  }
  return ret;
}

bool stusb_set_highest_pdo(uint8_t *maxPower, uint8_t currentPdoIndex){
	uint8_t highPowerPdoIdx = 0;
	uint8_t maxWattage = 0;
	for(uint8_t i = 0; i < pdos.numPDOs; i++){
		uint8_t w = pdos.pdos[i].current*0.05 * pdos.pdos[i].voltage*0.01;
		if(w>maxWattage){
			highPowerPdoIdx = (w>maxWattage) ? i : highPowerPdoIdx;
			maxWattage = w;
		}
	}
	*maxPower = maxWattage;
	//already higest power PDO selected ?
	if(currentPdoIndex-1 != highPowerPdoIdx){
		stusb_update_pdo(2,pdos.pdos[highPowerPdoIdx].voltage*50, pdos.pdos[highPowerPdoIdx].current*10);
		// give the STUSB some time to apply the PDOs
		HAL_Delay(500);
		stusb_soft_reset();
		debug_print_str(DEBUG_INFO,"Re-negotiating highest power PDO");
	}
	return true;
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
	stusb_write_register(REG_TX_HEADER_LOW, cmd, 2);
	cmd[0] = CMD_SEND_COMMAND;
	stusb_write_register(REG_PD_COMMAND_CTRL, cmd, 1);
}

//read USB-C connection status

