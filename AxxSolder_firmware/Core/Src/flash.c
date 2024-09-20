#include "flash.h"
#include "main.h"


extern CRC_HandleTypeDef hcrc;
uint8_t flashWriting = 0;
unsigned char bufferFlash[sizeof(Flash_values)];
static unsigned int bufLength = sizeof(Flash_values);


uint32_t GetPage(uint32_t Addr)
{
  return (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
}

bool FlashCheckCRC()
{
	volatile uint64_t uwCRCValue = 0;
	unsigned int cRCAddress = (bufLength / 8) * 8;
	if (bufLength % 8 != 0)
	{
		cRCAddress += 8;
	}
	uwCRCValue = HAL_CRC_Calculate(&hcrc, (uint32_t*) FLASH_USER_START_ADDR, bufLength);
	if (*(volatile uint64_t*) (FLASH_USER_START_ADDR + cRCAddress) == uwCRCValue)
	{
		return true;
	}
	return false;
}


bool FlashReadToBuf()
{
	unsigned int i = 0;
	unsigned char temp[8];
	uint32_t Address = FLASH_USER_START_ADDR;

	if (FlashCheckCRC())
	{
		while (i < bufLength && Address < FLASH_USER_END_ADDR)
		{
			*(uint64_t*) temp = *(volatile uint64_t*) Address;

			for (unsigned int j = 0; j < 8; j++)
			{
				if (j + i < bufLength)
				{
					bufferFlash[i + j] = temp[j];
				}
			}
			Address = Address + 8;
			i = i + 8;
		}
		return true;
	}
	return false;
}


//__attribute__((__section__(".user_data"))) const uint8_t userConfig[64];
bool FlashRead(Flash_values *flash_values)
{
	if (FlashReadToBuf())
	{
		memcpy(flash_values, bufferFlash, sizeof(Flash_values));
		return true;
	}
	return false;
}

uint32_t Flash_Write_Data (uint32_t StartPageAddress, uint32_t *Data, uint16_t numberofwords)
{
	static FLASH_EraseInitTypeDef EraseInitStruct;

		uint32_t PageError;
	int sofar=0;

	  /* Unlock the Flash to enable the flash control register access *************/
	   HAL_FLASH_Unlock();

	   /* Erase the user Flash area*/

	   uint32_t StartPage = GetPage(StartPageAddress);
	  uint32_t EndPageAdress = StartPageAddress + numberofwords*4;
	  uint32_t EndPage = GetPage(EndPageAdress);

		 /* Clear OPTVERR bit set on virgin samples */
		  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	   /* Fill EraseInit structure*/
		EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.Page        = StartPage;
		EraseInitStruct.NbPages     = ((EndPage - StartPage)/FLASH_PAGE_SIZE) +1;

	   if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
	   {
	     /*Error occurred while page erase.*/
		  return HAL_FLASH_GetError();
	   }

	   /* Program the user Flash area word by word*/

	   while (sofar<numberofwords)
	   {
	     if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, StartPageAddress, Data[sofar]) == HAL_OK)
	     {
	    	 StartPageAddress += 8;//8  // use StartPageAddress += 2 for half word and 8 for double word
	    	 sofar++;
	     }
	     else
	     {
	       /* Error occurred while writing data in Flash memory*/
	    	 return HAL_FLASH_GetError ();
	     }
	   }

	   /* Lock the Flash to disable the flash control register access (recommended
	      to protect the FLASH memory against possible unwanted operation) *********/
	   HAL_FLASH_Lock();

	   return 0;
}



bool FlashWrite(Flash_values *flash_values){
	static FLASH_EraseInitTypeDef EraseInitStruct;
	flashWriting = 1;
	volatile uint64_t uwCRCValue = 0;
	unsigned int i = 0;
	uint32_t Address = FLASH_USER_START_ADDR;
	uint32_t PageError = 0;
	uint64_t temp = 0;
	uint32_t NbOfPages = 0;

	if (!FlashReadToBuf())	//invalid flash memory force write all
	{
		//configurationMsg->command = ConfigurationCommand_WriteMsgToFlash;
	}

	memcpy(bufferFlash, flash_values, sizeof(Flash_values));

	uwCRCValue = HAL_CRC_Calculate(&hcrc, (uint32_t*) bufferFlash, bufLength);

	HAL_FLASH_Unlock();

	/* Clear OPTVERR bit set on virgin samples */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	/* Get the number of pages to erase from 1st page */
	NbOfPages = GetPage(FLASH_USER_END_ADDR) - GetPage(FLASH_USER_START_ADDR) + 1;

	/* Fill EraseInit structure*/
	EraseInitStruct.Banks = FLASH_BANK_1;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Page = GetPage(FLASH_USER_START_ADDR);
	EraseInitStruct.NbPages = NbOfPages;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) == HAL_OK)
	{
		//FLASH_FlushCaches();

		//__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);

		while (Address < FLASH_USER_END_ADDR)
		{
			for (unsigned int j = 0; j < 8; j++)
			{
				if (j + i < bufLength)
				{
					((unsigned char*) &temp)[j] = bufferFlash[i + j];
				}
				else
				{
					((unsigned char*) &temp)[j] = 0;
				}
			}
			if (i < bufLength && HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, temp) == HAL_OK)
			{
				Address = Address + 8;
				i = i + 8;
			}
			else if (i >= bufLength)
			{
				if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, uwCRCValue) == HAL_OK)
				{
					HAL_FLASH_Lock();
					flashWriting = 0;
					return true;
				}
			}
			else
			{
				/* Error occurred while writing data in Flash memory.
				 User can add here some code to deal with this error */
				break;
			}
		}
	}
	/* Lock the Flash to disable the flash control register access (recommended
	 to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();

	flashWriting = 0;
	return false;
}
