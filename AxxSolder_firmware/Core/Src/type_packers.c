/*
 * type_packers.c
 *
 *  Created on: Nov 5, 2024
 *      Author: Axel Johansson
 */

/* Includes ------------------------------------------------------------------*/
#include "type_packers.h"

/* Pack frame start for sending over UART */
void pack_frame_start( uint8_t *buffer, uint8_t *packet_count, uint8_t UART_packet_length)
{
    buffer[0] = ((uint8_t)0xAA);
    buffer[1] = ((uint8_t)UART_packet_length);

    *packet_count = 2;
}

void pack_u16(uint8_t *buffer, uint8_t *packet_count, const uint16_t data_u16)
{
    buffer[(*packet_count) + 0] = ((uint8_t*)&data_u16)[0];
    buffer[(*packet_count) + 1] = ((uint8_t*)&data_u16)[1];

    *packet_count+= sizeof(data_u16);
}

void unpack_u16( const uint8_t *buffer, uint8_t *packet_count,  uint16_t *data_u16)
{
    ((uint8_t*)data_u16)[0] = buffer[(*packet_count) + 0];
    ((uint8_t*)data_u16)[1] = buffer[(*packet_count) + 1];

    *packet_count+= sizeof(data_u16);
}

void pack_u32(uint8_t *buffer, uint8_t *packet_count, const uint32_t data_u32)
{
    buffer[(*packet_count) + 0] = ((uint8_t*)&data_u32)[0];
    buffer[(*packet_count) + 1] = ((uint8_t*)&data_u32)[1];
    buffer[(*packet_count) + 2] = ((uint8_t*)&data_u32)[2];
    buffer[(*packet_count) + 3] = ((uint8_t*)&data_u32)[3];

    *packet_count+= sizeof(data_u32);
}

void unpack_u32(const uint8_t *buffer, uint8_t *packet_count, uint32_t *data_u32)
{
    ((uint8_t*)data_u32)[0] = buffer[(*packet_count) + 0];
    ((uint8_t*)data_u32)[1] = buffer[(*packet_count) + 1];
    ((uint8_t*)data_u32)[2] = buffer[(*packet_count) + 2];
    ((uint8_t*)data_u32)[3] = buffer[(*packet_count) + 3];

    *packet_count += sizeof(data_u32);
}

void pack_float( uint8_t *buffer, uint8_t *packet_count, const float data_float)
{
    buffer[(*packet_count) + 0] = ((uint8_t*)&data_float)[0];
    buffer[(*packet_count) + 1] = ((uint8_t*)&data_float)[1];
    buffer[(*packet_count) + 2] = ((uint8_t*)&data_float)[2];
    buffer[(*packet_count) + 3] = ((uint8_t*)&data_float)[3];

    *packet_count+= sizeof(data_float);
}

void unpack_float( const uint8_t *buffer, uint8_t *packet_count, float *data_float)
{
    ((uint8_t*)data_float)[0] = buffer[(*packet_count) + 0];
    ((uint8_t*)data_float)[1] = buffer[(*packet_count) + 1];
    ((uint8_t*)data_float)[2] = buffer[(*packet_count) + 2];
    ((uint8_t*)data_float)[3] = buffer[(*packet_count) + 3];

    *packet_count+= sizeof(data_float);
}

/*
Using example:

for sending;

#define MAX_BUFFER_LEN 255
uint8_t transmit_buffer[MAX_BUFFER_LEN];

uint8_t packet_index = 0;
uint16_t sensor_u16 = 12345;
uint32_t sensor_u32 = 123456789;
float sensor_f32 = 15.56;

pack_u16(transmit_buffer, &packet_index, sensor_u16);
pack_u32(transmit_buffer, &packet_index, sensor_u32);
pack_float(transmit_buffer, &packet_index, sensor_f32);
for receiving;

#define MAX_BUFFER_LEN 255
uint8_t receiving_buffer[MAX_BUFFER_LEN];

uint8_t packet_index = 0;
uint16_t sensor_u16;
uint32_t sensor_u32;
float sensor_f32;

read_UART_buffer(receiving_buffer); // pseudo function for read UART buffer.

unpack_u16(receiving_buffer, &packet_index, sensor_u16);
unpack_u32(receiving_buffer, &packet_index, sensor_u32);
unpack_float(receiving_buffer, &packet_index, sensor_f32);
*/
