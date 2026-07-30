#ifndef SENS_NORM_H
#define SENS_NORM_H

#include <stdint.h>
#include <stdbool.h>

#define IR_INVALID_VALUE (-1.0f)

typedef enum
{
    IR_SENSOR_RIGHT_45 = 0,
    IR_SENSOR_LEFT_45,
    IR_SENSOR_COUNT
} ir_sensor_id_t;

typedef struct
{
    float raw;
    float filtered;
    float scaled;
    float reference;
    float scale;
    float offset;
    uint8_t valid;
} IR_Channel_t;

void IR_Init(void);
void IR_Update(void);
void IR_CalibrateReference(void);

/* direct hardware read */
float IR_ReadHardware(ir_sensor_id_t id);
float IR_ReadHardwareScaled(ir_sensor_id_t id);

/* cached reads */
float IR_ReadRaw(ir_sensor_id_t id);
float IR_ReadFiltered(ir_sensor_id_t id);
float IR_ReadScaled(ir_sensor_id_t id);

const IR_Channel_t* IR_GetChannel(ir_sensor_id_t id);
float IR_EstimateDistanceMm(ir_sensor_id_t id, float k);

///* direct scaled hardware API */
//float readSensor_Left_45(void);
//float readSensor_Right_45(void);

#endif