/*
 * type_packers.h
 *
 *  Created on: Nov 5, 2024
 *      Author: Axel Johansson
 */

#ifndef INC_TYPE_PACKERS_H_
#define INC_TYPE_PACKERS_H_
/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/* TypeDefs ------------------------------------------------------------------*/
void pack_frame_start( uint8_t *buffer, uint8_t *packet_count, uint8_t UART_packet_length);
void pack_u16(uint8_t *buffer, uint8_t *packet_count, const uint16_t data_u16);
void unpack_u16( const uint8_t *buffer, uint8_t *packet_count,  uint16_t *data_u16);
void pack_u32(uint8_t *buffer, uint8_t *packet_count, const uint32_t data_u32);
void unpack_u32(const uint8_t *buffer, uint8_t *packet_count, uint32_t *data_u32);
void pack_float( uint8_t *buffer, uint8_t *packet_count, const float data_float);
void unpack_float( const uint8_t *buffer, uint8_t *packet_count, float *data_float);


#endif /* INC_TYPE_PACKERS_H_ */
