#ifndef OPT3101_H
#define OPT3101_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define OPT3101_ADDR_LEFT_45      0x58
#define OPT3101_ADDR_RIGHT_45     0x59//0x59

#define OPT3101_INVALID_MM        0xFFFFU

typedef enum
{
    OPT3101_LEFT_45 = 0,
    OPT3101_RIGHT_45,
    OPT3101_COUNT
} opt3101_id_t;

typedef struct
{
    uint16_t distance_mm;
    uint16_t phase;
    uint16_t amplitude;
    uint16_t ambient;
    uint16_t temperature_raw;
    uint8_t valid;
    uint8_t saturated;
    uint8_t ambient_saturated;
    uint8_t new_data;
} OPT3101_Data_t;

int OPT3101_InitAll(void);
void OPT3101_UpdateAll(void);

uint16_t OPT3101_ReadDistanceMm(opt3101_id_t id);
const OPT3101_Data_t* OPT3101_GetData(opt3101_id_t id);

HAL_StatusTypeDef opt_read24(uint8_t addr, uint8_t reg, uint32_t *value);


#endif