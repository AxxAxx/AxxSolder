/*
 * stusb4500.c - ST STUSB4500 autonomous USB-PD sink driver.
 * Primitives only; the negotiation STRATEGY lives in power_source.c.
 * See stusb4500.h for the chip model and the hardware-proven constraints
 * (no source-caps capture, Soft_Reset-only message injection).
 */
#include "stusb4500.h"
#include <string.h>

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
	uint8_t chipID = 0;
	HAL_StatusTypeDef ret = stusb_read_register(REG_DEVICE_ID, &chipID, 1);
	if (ret != HAL_OK){ return ret; }
	return ((chipID == VAL_STUSB4500_ID) || (chipID == VAL_STUSB4500_ID_B)) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef stusb_init(){
	/* clear status registers by reading, then unmask the PRT status alert */
	uint8_t dummy[10];
	HAL_StatusTypeDef ret = stusb_read_register(REG_PORT_STATUS_0, dummy, 10);
	uint8_t writeVal = (uint8_t)~VAL_PRT_STATUS_AL;
	HAL_StatusTypeDef ret2 = stusb_write_register(REG_ALERT_STATUS_1_MASK, &writeVal, 1);
	return (ret != HAL_OK) ? ret : ret2;
}

int8_t stusb_sink_attached() {
  uint8_t data = 0;
  if (stusb_read_register(REG_PORT_STATUS_1, &data, 1) != HAL_OK){ return -1; }
  return ((data & VAL_SNK_ATT) != 0) ? 1 : 0;
}

bool stusb_is_sink_ready() {
  uint8_t data = 0;
  if (stusb_read_register(REG_PE_FSM, &data, 1) != HAL_OK){ return false; }
  return data == VAL_PE_SNK_READY;   /* PE_FSM is a state CODE, not a bitmask */
}

HAL_StatusTypeDef stusb_read_rdo(STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef *Nego_RDO) {
  memset(Nego_RDO, 0, sizeof(*Nego_RDO));   /* zeroed = "no contract" on failure */
  return stusb_read_register(REG_RDO_REG_STATUS, (uint8_t *) Nego_RDO, 4);
}

HAL_StatusTypeDef stusb_set_sink_pdos(const uint16_t mv[3], const uint16_t ma[3]) {
  /* Read the current 3 x 4-byte PDO block once, patch all three slots, write
   * the block back in one transfer, then set the valid count. */
  uint8_t data[12];
  HAL_StatusTypeDef ret = stusb_read_register(DPM_SNK_PDO1, data, 12);
  if (ret != HAL_OK){ return ret; }

  USB_PD_SNK_PDO_TypeDef pdo;
  for (uint8_t i = 0; i < 3; i++) {
    pdo.d32 =  (uint32_t) data[4*i]
            + ((uint32_t) data[4*i + 1] << 8)
            + ((uint32_t) data[4*i + 2] << 16)
            + ((uint32_t) data[4*i + 3] << 24);

    pdo.fix.Operationnal_Current = ma[i] / 10;
    if (i == 0) {
      /* PDO1 voltage is forced to 5 V by the USB-PD spec */
      pdo.fix.Voltage = 100;                    /* 5000/50 */
      pdo.fix.USB_Communications_Capable = 1;
    } else {
      pdo.fix.Voltage = mv[i] / 50;
    }

    data[4*i]     = (uint8_t)(pdo.d32);
    data[4*i + 1] = (uint8_t)(pdo.d32 >> 8);
    data[4*i + 2] = (uint8_t)(pdo.d32 >> 16);
    data[4*i + 3] = (uint8_t)(pdo.d32 >> 24);
  }

  ret = stusb_write_register(DPM_SNK_PDO1, data, 12);
  if (ret != HAL_OK){ return ret; }

  uint8_t count = 3;
  return stusb_write_register(DPM_PDO_NUMB, &count, 1);
}

HAL_StatusTypeDef stusb_update_pdo(uint8_t pdo_number, uint16_t voltage_mv, uint16_t current_ma) {
  if ((pdo_number < 1) || (pdo_number > 3)){ return HAL_ERROR; }
  uint8_t data[12];
  HAL_StatusTypeDef ret = stusb_read_register(DPM_SNK_PDO1, data, 12);
  if (ret != HAL_OK){ return ret; }

  uint8_t *p = &data[4 * (pdo_number - 1)];
  USB_PD_SNK_PDO_TypeDef pdo;
  pdo.d32 =  (uint32_t) p[0]
          + ((uint32_t) p[1] << 8)
          + ((uint32_t) p[2] << 16)
          + ((uint32_t) p[3] << 24);
  pdo.fix.Operationnal_Current = current_ma / 10;
  if (pdo_number == 1) {
    /* PDO1 voltage is forced to 5 V by the USB-PD spec */
    pdo.fix.Voltage = 100;                      /* 5000/50 */
    pdo.fix.USB_Communications_Capable = 1;
  } else {
    pdo.fix.Voltage = voltage_mv / 50;
  }
  p[0] = (uint8_t)(pdo.d32);
  p[1] = (uint8_t)(pdo.d32 >> 8);
  p[2] = (uint8_t)(pdo.d32 >> 16);
  p[3] = (uint8_t)(pdo.d32 >> 24);
  return stusb_write_register(DPM_SNK_PDO1 + (4 * (pdo_number - 1)), p, 4);
}

HAL_StatusTypeDef stusb_set_valid_pdo(uint8_t valid_count) {
  if ((valid_count < 1) || (valid_count > 3)){ return HAL_ERROR; }
  return stusb_write_register(DPM_PDO_NUMB, &valid_count, 1);
}

/* Renegotiate with the source via a PD Soft_Reset message (VBUS held). */
HAL_StatusTypeDef stusb_soft_reset(){
	uint8_t cmd[]={CMD_SOFT_RESET,0x00};
	HAL_StatusTypeDef ret = stusb_write_register(REG_TX_HEADER_LOW, cmd, 2);
	if(ret != HAL_OK){ return ret; }
	cmd[0] = CMD_SEND_COMMAND;
	return stusb_write_register(REG_PD_COMMAND_CTRL, cmd, 1);
}

/* ------------------------------------------------------------------ */
/* NVM (FTP) access - sequence per ST USBPD_CUST_NVM_API / SparkFun    */
/* ------------------------------------------------------------------ */

/* ST factory-default NVM content (SparkFun library). Used as a recovery
 * baseline ONLY if the on-chip NVM reads back blank/corrupt (e.g. power was
 * lost mid-write on a previous provisioning attempt). Sector 0 is the same on
 * every image ST tools generate, so it doubles as a layout sanity check. */
static const uint8_t nvm_st_defaults[STUSB_NVM_SECTORS][STUSB_NVM_SECTOR_SIZE] = {
	{0x00,0x00,0xB0,0xAA,0x00,0x45,0x00,0x00},
	{0x10,0x40,0x9C,0x1C,0xFF,0x01,0x3C,0xDF},
	{0x02,0x40,0x0F,0x00,0x32,0x00,0xFC,0xF1},
	{0x00,0x19,0x56,0xAF,0xF5,0x35,0x5F,0x00},
	{0x00,0x4B,0x90,0x21,0x43,0x00,0x40,0xFB}
};

static bool ftp_write1(uint8_t reg, uint8_t val){
	return stusb_write_register(reg, &val, 1) == HAL_OK;
}

static bool ftp_wait_req_clear(void){
	uint32_t start = HAL_GetTick();
	do {
		uint8_t ctrl = 0;
		if (stusb_read_register(FTP_CTRL_0, &ctrl, 1) != HAL_OK){ return false; }
		if (!(ctrl & FTP_CUST_REQ)){ return true; }
	} while ((HAL_GetTick() - start) < 100);
	return false;
}

static void ftp_exit(void){
	(void)ftp_write1(FTP_CTRL_0, FTP_CUST_RST_N);
	(void)ftp_write1(FTP_CTRL_1, 0x00);
	(void)ftp_write1(FTP_CUST_PASSWORD_REG, 0x00);
}

static bool nvm_read_sectors(uint8_t out[STUSB_NVM_SECTORS][STUSB_NVM_SECTOR_SIZE]){
	bool ok = ftp_write1(FTP_CUST_PASSWORD_REG, FTP_CUST_PASSWORD)
	       && ftp_write1(FTP_CTRL_0, 0x00)                          /* controller reset */
	       && ftp_write1(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N);
	for (uint8_t s = 0; ok && s < STUSB_NVM_SECTORS; s++){
		ok = ftp_write1(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N)
		  && ftp_write1(FTP_CTRL_1, FTP_OPCODE_READ)
		  && ftp_write1(FTP_CTRL_0, (uint8_t)((s & FTP_CUST_SECT_MASK) | FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ))
		  && ftp_wait_req_clear()
		  && (stusb_read_register(FTP_RW_BUFFER, out[s], STUSB_NVM_SECTOR_SIZE) == HAL_OK)
		  && ftp_write1(FTP_CTRL_0, 0x00);                         /* controller reset (ST CUST_ReadSector) */
	}
	ftp_exit();
	return ok;
}

#if STUSB_NVM_PROVISION
static bool nvm_write_sectors(const uint8_t data[STUSB_NVM_SECTORS][STUSB_NVM_SECTOR_SIZE]){
	/* Enter write mode and erase all 5 sectors, then program each. Aborts on
	 * the first failed step; a partial write leaves the NVM blank, which
	 * stusb_nvm_provision() retries or recovers via the ST-defaults baseline. */
	bool ok = ftp_write1(FTP_CUST_PASSWORD_REG, FTP_CUST_PASSWORD)
	       && ftp_write1(FTP_RW_BUFFER, 0x00)                       /* NULL the buffer (ST sequence) */
	       && ftp_write1(FTP_CTRL_0, 0x00)
	       && ftp_write1(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N)
	       /* load the sector-erase register (all 5 sectors) */
	       && ftp_write1(FTP_CTRL_1, (uint8_t)(((0x1Fu << 3) & 0xF8u) | FTP_OPCODE_WRITE_SER))
	       && ftp_write1(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ)
	       && ftp_wait_req_clear()
	       /* soft-program then erase */
	       && ftp_write1(FTP_CTRL_1, FTP_OPCODE_SOFT_PROG)
	       && ftp_write1(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ)
	       && ftp_wait_req_clear()
	       && ftp_write1(FTP_CTRL_1, FTP_OPCODE_ERASE)
	       && ftp_write1(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ)
	       && ftp_wait_req_clear();

	for (uint8_t s = 0; ok && s < STUSB_NVM_SECTORS; s++){
		uint8_t buf[STUSB_NVM_SECTOR_SIZE];
		memcpy(buf, data[s], STUSB_NVM_SECTOR_SIZE);
		ok = (stusb_write_register(FTP_RW_BUFFER, buf, STUSB_NVM_SECTOR_SIZE) == HAL_OK)
		  && ftp_write1(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N)
		  && ftp_write1(FTP_CTRL_1, FTP_OPCODE_WRITE_PL)
		  && ftp_write1(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ)
		  && ftp_wait_req_clear()
		  && ftp_write1(FTP_CTRL_1, FTP_OPCODE_PROG_SECTOR)
		  && ftp_write1(FTP_CTRL_0, (uint8_t)((s & FTP_CUST_SECT_MASK) | FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ))
		  && ftp_wait_req_clear();
	}
	ftp_exit();
	return ok;
}
#endif

/* Blank/corrupt NVM detection, PER SECTOR: an interrupted provisioning write
 * (erase done, only some sectors re-programmed) leaves a MIX of valid and
 * all-0xFF sectors - a whole-image check would read the erased sector 4 as
 * "REQ_SRC_CURRENT set" and never self-heal. Any single blank sector marks
 * the image corrupt so it gets rebuilt from the ST defaults. */
static bool nvm_looks_blank(const uint8_t s[STUSB_NVM_SECTORS][STUSB_NVM_SECTOR_SIZE]){
	for (uint8_t i = 0; i < STUSB_NVM_SECTORS; i++){
		uint8_t and_sec = 0xFF, or_sec = 0x00;
		for (uint8_t j = 0; j < STUSB_NVM_SECTOR_SIZE; j++){
			and_sec &= s[i][j];
			or_sec  |= s[i][j];
		}
		if ((and_sec == 0xFF) || (or_sec == 0x00)){ return true; }
	}
	return false;
}

/* Read the whole NVM twice and accept it only if both images match, so a
 * marginal I2C read can never be mistaken for a blank or clear-bit chip. */
static bool nvm_read_stable(uint8_t out[STUSB_NVM_SECTORS][STUSB_NVM_SECTOR_SIZE]){
	uint8_t check[STUSB_NVM_SECTORS][STUSB_NVM_SECTOR_SIZE];
	return nvm_read_sectors(out) && nvm_read_sectors(check)
	    && (memcmp(out, check, sizeof(check)) == 0);
}

stusb_nvm_state_t stusb_nvm_provision(void){
	uint8_t img[STUSB_NVM_SECTORS][STUSB_NVM_SECTOR_SIZE];
	if (!nvm_read_stable(img)){ return STUSB_NVM_ERROR; }

	bool blank = nvm_looks_blank((const uint8_t (*)[STUSB_NVM_SECTOR_SIZE])img);
	if (!blank){
		/* Unknown layout: never modify an image we don't understand. */
		if (memcmp(img[0], nvm_st_defaults[0], STUSB_NVM_SECTOR_SIZE) != 0){
			return STUSB_NVM_ERROR;
		}
		if (img[STUSB_NVM_REQ_SRC_CURRENT_SECTOR][STUSB_NVM_REQ_SRC_CURRENT_BYTE]
		    & STUSB_NVM_REQ_SRC_CURRENT_BIT){
			return STUSB_NVM_SET;
		}
	}

#if STUSB_NVM_PROVISION
	if (blank){
		/* Interrupted previous write: rebuild from ST factory defaults. */
		memcpy(img, nvm_st_defaults, sizeof(img));
	}
	img[STUSB_NVM_REQ_SRC_CURRENT_SECTOR][STUSB_NVM_REQ_SRC_CURRENT_BYTE]
	    |= STUSB_NVM_REQ_SRC_CURRENT_BIT;

	/* Write from the RAM image and verify; a failed attempt leaves the NVM
	 * erased or partial, so retry right away rather than leave it that way. */
	for (uint8_t attempt = 0; attempt < STUSB_NVM_WRITE_ATTEMPTS; attempt++){
		uint8_t verify[STUSB_NVM_SECTORS][STUSB_NVM_SECTOR_SIZE];
		if (nvm_write_sectors((const uint8_t (*)[STUSB_NVM_SECTOR_SIZE])img) && nvm_read_stable(verify)
		    && (memcmp(img, verify, sizeof(img)) == 0)){
			return STUSB_NVM_WRITTEN;
		}
	}
	return STUSB_NVM_ERROR;
#else
	return blank ? STUSB_NVM_ERROR : STUSB_NVM_CLEAR;
#endif
}
