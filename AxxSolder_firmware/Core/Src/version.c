#include "version.h"
#include "main.h"   /* VERSION_BIT_1/2/3_Pin, GPIOC, HAL_GPIO_ReadPin */

uint8_t fw_version_major =  3;
uint8_t fw_version_minor =  8;
uint8_t fw_version_patch =  1;

/**
 * @brief Retrieves the hardware version of the device based on the state of 3 input pins.
 * The states of the VERSION_BIT_1..3 pins form a 3-bit number:
 *     VERSION_BIT_3 - most significant bit (MSB)
 *     VERSION_BIT_2 - middle bit
 *     VERSION_BIT_1 - least significant bit (LSB)
 *
 * Version numbering starts from 3, i.e., version = 3 + [bit value]
 *
 * @return uint8_t Hardware version number (3..10)
 */
uint8_t get_hw_version(void){
    uint8_t version_bits =
        (HAL_GPIO_ReadPin(GPIOC, VERSION_BIT_3_Pin) << 2) |
        (HAL_GPIO_ReadPin(GPIOC, VERSION_BIT_2_Pin) << 1) |
        (HAL_GPIO_ReadPin(GPIOC, VERSION_BIT_1_Pin) << 0);

    return 3 + version_bits;
}
