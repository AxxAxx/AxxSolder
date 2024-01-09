/*
 * STUSB4500 I2C Driver
 *
 * Largely based on ST firmware example: https://github.com/usb-c/STUSB4500/tree/master/Firmware/Project/Src
 * Otter-Iron support added by J.Whittington 2020
 *
 */
#include "stusb4500.h"

USB_PD_SNK_PDO_TypeDef pdo_profile[3];
extern I2C_HandleTypeDef hi2c1;

static HAL_StatusTypeDef read_register(uint8_t device, uint8_t reg, uint8_t *data, uint8_t len)
{
  return HAL_I2C_Mem_Read(&hi2c1, (device << 1), (uint16_t) reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

static HAL_StatusTypeDef write_register(uint8_t device, uint8_t reg, uint8_t *data, uint8_t len)
{
  return HAL_I2C_Mem_Write(&hi2c1, (device << 1), (uint16_t) reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

HAL_StatusTypeDef stusb_read_rdo(STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef *Nego_RDO) {
  HAL_StatusTypeDef ret;
  ret = read_register(STUSB4500_ADDR, (uint16_t) RDO_REG_STATUS, (uint8_t *) Nego_RDO, 4);

  return ret;
}

HAL_StatusTypeDef stusb_update_pdo(uint8_t pdo_number, uint16_t voltage_mv, uint16_t current_ma) {
  HAL_StatusTypeDef ret;
  uint16_t addr;
  uint8_t data[40];
  uint8_t i, j = 0;

  // get existing
  addr = DPM_SNK_PDO1;
  ret = read_register(STUSB4500_ADDR, addr, data, 12);
  for (i = 0 ; i < 3 ; i++) { pdo_profile[i].d32 = (uint32_t) (data[j] +(data[j+1]<<8)+(data[j+2]<<16)+(data[j+3]<<24));
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
    ret = write_register(STUSB4500_ADDR, addr, (uint8_t *) &pdo_profile[pdo_number - 1].d32, 4);
  }

  return ret;
}

HAL_StatusTypeDef stusb_set_valid_pdo(uint8_t valid_count) {
  HAL_StatusTypeDef ret = -1;
  if (valid_count <= 3) {
    ret = write_register(STUSB4500_ADDR, DPM_PDO_NUMB, &valid_count, 1);
  }
  return ret;
}
