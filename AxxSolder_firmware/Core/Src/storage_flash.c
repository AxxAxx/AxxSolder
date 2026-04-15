#include "storage.h"
#include "stm32g4xx_hal.h"
#include <string.h>

extern CRC_HandleTypeDef hcrc;

/* ---- Low-level flash helpers ---- */

static uint32_t get_page(uint32_t addr)
{
	return (addr - FLASH_BASE) / FLASH_PAGE_SIZE;
}

static uint32_t flash_write_dwords(uint32_t addr, const uint32_t *data, uint16_t ndwords)
{
	FLASH_EraseInitTypeDef erase;
	uint32_t page_err;
	int sofar = 0;

	__disable_irq();
	HAL_FLASH_Unlock();

	uint32_t start_page = get_page(addr);
	uint32_t end_page   = get_page(addr + ndwords * 8 - 1);

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	erase.Banks     = FLASH_BANK_1;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	erase.Page      = start_page;
	erase.NbPages   = end_page - start_page + 1;

	if (HAL_FLASHEx_Erase(&erase, &page_err) != HAL_OK) {
		HAL_FLASH_Lock();
		__enable_irq();
		return HAL_FLASH_GetError();
	}

	while (sofar < ndwords) {
		uint64_t dword;
		memcpy(&dword, (const uint8_t *)data + sofar * 8, 8);
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, dword) == HAL_OK) {
			addr += 8;
			sofar++;
		} else {
			HAL_FLASH_Lock();
			__enable_irq();
			return HAL_FLASH_GetError();
		}
	}

	HAL_FLASH_Lock();
	__enable_irq();
	return 0;
}

/* ---- StorageDriver implementation ---- */

static StoreResult flash_read(uint32_t addr, void *buf, uint16_t size)
{
	volatile uint8_t *src = (volatile uint8_t *)addr;
	uint8_t *dst = (uint8_t *)buf;

	for (uint16_t i = 0; i < size; i++) {
		dst[i] = src[i];
	}

	/* Verify CRC stored right after data (8-byte aligned offset) */
	uint32_t crc_offset = ((uint32_t)size + 7u) & ~7u;
	uint64_t stored_crc = *(volatile uint64_t *)(addr + crc_offset);

	__disable_irq();
	uint64_t calc_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)addr, size);
	__enable_irq();

	if (stored_crc != calc_crc) {
		return STORE_ERR_CRC;
	}

	return STORE_OK;
}

static StoreResult flash_write(uint32_t addr, const void *buf, uint16_t size)
{
	/* Prepare buffer: data + CRC, 8-byte aligned */
	uint32_t crc_offset = ((uint32_t)size + 7u) & ~7u;
	uint32_t total_size = crc_offset + 8; /* data + padding + CRC */

	/* Static buffer: must fit largest payload + CRC padding.
	 * Settings ~200 B, ProfileStore 1744 B → 1744+8 = 1752 B.
	 * Static (BSS) to avoid stack pressure on Cortex-M4. */
	static uint8_t tmp[2048];
	if (total_size > sizeof(tmp)) {
		return STORE_ERR_INVALID;
	}

	memset(tmp, 0, total_size);
	memcpy(tmp, buf, size);

	/* Calculate CRC over the original data size (protect CRC peripheral from ISR) */
	__disable_irq();
	uint64_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)tmp, size);
	__enable_irq();
	memcpy(&tmp[crc_offset], &crc, 8);

	/* Write doublewords */
	uint16_t dwords = total_size / 8;
	uint32_t result = flash_write_dwords(addr, (uint32_t *)tmp, dwords);

	return (result == 0) ? STORE_OK : STORE_ERR_WRITE;
}

static bool flash_verify(uint32_t addr, uint16_t size)
{
	uint32_t crc_offset = ((uint32_t)size + 7u) & ~7u;
	uint64_t stored_crc = *(volatile uint64_t *)(addr + crc_offset);

	__disable_irq();
	uint64_t calc_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)addr, size);
	__enable_irq();

	return stored_crc == calc_crc;
}

const StorageDriver storage_flash = {
	.read   = flash_read,
	.write  = flash_write,
	.verify = flash_verify,
};
