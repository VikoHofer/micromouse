/*************************************************************
Project     : Micromouse V1.2
File        : sensor_normalize.c
Description : centralized IR sensor normalization and filtering
Date        : 15.04.2026
*************************************************************/

#include "sensor_normalize.h"

#include "adc.h"
#include "config.h"
#include "sensor.h"
#include "Delay.h"

#include <string.h>
#include <math.h>
#include "config.h"
/* ----------------------------------------------------------
 * Fallbacks / configuration
 * ---------------------------------------------------------- */



// middle references
#define IR_REF_FRONT_LEFT      2790.0f
#define IR_REF_FRONT_RIGHT     2410.0f
#define IR_REF_LEFT            2430.0f
#define IR_REF_LEFT_45         2700.0f
#define IR_REF_RIGHT_45        2900.0f
#define IR_REF_RIGHT           2780.0f

/* ----------------------------------------------------------
 * Internal mapping
 * ---------------------------------------------------------- */
typedef struct
{
    SensorID hw_id;
    float alpha;
    uint8_t use_filter;
    float offset;
} IR_ChannelConfig_t;

static const IR_ChannelConfig_t s_cfg[IR_SENSOR_COUNT] =
{
    [IR_SENSOR_FRONT_LEFT]  = { SENSOR_ID_1, IR_ALPHA_FRONT, 0U, 0.0f      },
    [IR_SENSOR_FRONT_RIGHT] = { SENSOR_ID_2, IR_ALPHA_FRONT, 0U, 0.0f      },
    [IR_SENSOR_LEFT]        = { SENSOR_ID_6, IR_ALPHA_SIDE,  0U, 0.0f      },
    [IR_SENSOR_LEFT_45]     = { SENSOR_ID_3, IR_ALPHA_45,    0U, 0.0f      },
    [IR_SENSOR_RIGHT_45]    = { SENSOR_ID_4, IR_ALPHA_45,    0U, 0.0f },
    [IR_SENSOR_RIGHT]       = { SENSOR_ID_5, IR_ALPHA_SIDE,  0U, 0.0f      },
};

static IR_Channel_t s_ir[IR_SENSOR_COUNT];

/* ----------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------- */
float IR_ReadHardware(ir_sensor_id_t id)
{
    uint32_t sensorValue = 0U;
    float value;

    if ((uint32_t)id >= (uint32_t)IR_SENSOR_COUNT) {
        return IR_INVALID_VALUE;
    }

    if (Sensor_Read(s_cfg[id].hw_id, &hadc1, &sensorValue) != SENSOR_OK) {
        return IR_INVALID_VALUE;
    }

    value = (float)sensorValue - s_cfg[id].offset;

    if (value < 0.0f) {
        value = 0.0f;
    }

    return value;
}

float IR_ReadHardwareScaled(ir_sensor_id_t id)
{
    float raw;
    float ref;

    if ((uint32_t)id >= (uint32_t)IR_SENSOR_COUNT) {
        return IR_INVALID_VALUE;
    }

    raw = IR_ReadHardware(id);
    if (raw < 0.0f) {
        return IR_INVALID_VALUE;
    }

    ref = s_ir[id].reference;

    if (ref > 1.0f) {
        return raw * (IR_NOMINAL_REFERENCE / ref);
    }

    return raw;
}

static float IR_ApplyFilter(IR_Channel_t *ch, float raw, float alpha, uint8_t use_filter)
{
    if (ch == NULL) {
        return IR_INVALID_VALUE;
    }

    if (!ch->valid) {
        ch->filtered = raw;
        ch->valid = 1U;
        return ch->filtered;
    }

    if (!use_filter) {
        ch->filtered = raw;
        return ch->filtered;
    }

    ch->filtered = ch->filtered + alpha * (raw - ch->filtered);
    return ch->filtered;
}

static void IR_UpdateScaled(IR_Channel_t *ch)
{
    if (ch == NULL) {
        return;
    }

    if (!ch->valid) {
        ch->scaled = IR_INVALID_VALUE;
        return;
    }

    if (ch->reference > 1.0f) {
        ch->scale  = IR_NOMINAL_REFERENCE / ch->reference;
        ch->scaled = ch->filtered * ch->scale;
    } else {
        /* before calibration: scaled = filtered */
        ch->scale  = 1.0f;
        ch->scaled = ch->filtered;
    }
}

static float IR_AverageSensor(ir_sensor_id_t id)
{
    float sum = 0.0f;
    float v;
    uint32_t count = 0U;
    uint32_t i;

    for (i = 0U; i < IR_CALIB_SAMPLES; i++)
    {
        v = IR_ReadHardware(id);
        if (v >= 0.0f) {
            sum += v;
            count++;
        }
        Delay_ms(IR_CALIB_DELAY_MS);
    }

    if (count == 0U) {
        return 0.0f;
    }

    return sum / (float)count;
}

/* ----------------------------------------------------------
 * Public API
 * ---------------------------------------------------------- */
void IR_Init(void)
{	
	Sensor_Init();
	
    memset(s_ir, 0, sizeof(s_ir));

    s_ir[IR_SENSOR_FRONT_LEFT].offset  = s_cfg[IR_SENSOR_FRONT_LEFT].offset;
    s_ir[IR_SENSOR_FRONT_RIGHT].offset = s_cfg[IR_SENSOR_FRONT_RIGHT].offset;
    s_ir[IR_SENSOR_LEFT].offset        = s_cfg[IR_SENSOR_LEFT].offset;
    s_ir[IR_SENSOR_LEFT_45].offset     = s_cfg[IR_SENSOR_LEFT_45].offset;
    s_ir[IR_SENSOR_RIGHT_45].offset    = s_cfg[IR_SENSOR_RIGHT_45].offset;
    s_ir[IR_SENSOR_RIGHT].offset       = s_cfg[IR_SENSOR_RIGHT].offset;

#ifdef IR_REF_FRONT_LEFT
    s_ir[IR_SENSOR_FRONT_LEFT].reference = IR_REF_FRONT_LEFT;
#endif
#ifdef IR_REF_FRONT_RIGHT
    s_ir[IR_SENSOR_FRONT_RIGHT].reference = IR_REF_FRONT_RIGHT;
#endif
#ifdef IR_REF_LEFT
    s_ir[IR_SENSOR_LEFT].reference = IR_REF_LEFT;
#endif
#ifdef IR_REF_LEFT_45
    s_ir[IR_SENSOR_LEFT_45].reference = IR_REF_LEFT_45;
#endif
#ifdef IR_REF_RIGHT_45
    s_ir[IR_SENSOR_RIGHT_45].reference = IR_REF_RIGHT_45;
#endif
#ifdef IR_REF_RIGHT
    s_ir[IR_SENSOR_RIGHT].reference = IR_REF_RIGHT;
#endif
}
void IR_Update(void)
{
    float raw_left, raw_right;

    raw_left  = IR_ReadHardware(IR_SENSOR_LEFT_45);
    raw_right = IR_ReadHardware(IR_SENSOR_RIGHT_45);

    s_ir[IR_SENSOR_LEFT_45].raw  = raw_left;
    s_ir[IR_SENSOR_RIGHT_45].raw = raw_right;

    /* LEFT */
    (void)IR_ApplyFilter(&s_ir[IR_SENSOR_LEFT_45],
                         raw_left,
                         s_cfg[IR_SENSOR_LEFT_45].alpha,
                         s_cfg[IR_SENSOR_LEFT_45].use_filter);

    IR_UpdateScaled(&s_ir[IR_SENSOR_LEFT_45]);

    /* RIGHT */
    (void)IR_ApplyFilter(&s_ir[IR_SENSOR_RIGHT_45],
                         raw_right,
                         s_cfg[IR_SENSOR_RIGHT_45].alpha,
                         s_cfg[IR_SENSOR_RIGHT_45].use_filter);

    IR_UpdateScaled(&s_ir[IR_SENSOR_RIGHT_45]);
}

void IR_CalibrateReference(void)
{
    s_ir[IR_SENSOR_LEFT_45].reference  = IR_AverageSensor(IR_SENSOR_LEFT_45);
    s_ir[IR_SENSOR_RIGHT_45].reference = IR_AverageSensor(IR_SENSOR_RIGHT_45);


    /* Update scale immediately with current filtered/raw state */
    IR_UpdateScaled(&s_ir[IR_SENSOR_LEFT_45]);
    IR_UpdateScaled(&s_ir[IR_SENSOR_RIGHT_45]);
    IR_UpdateScaled(&s_ir[IR_SENSOR_FRONT_LEFT]);
    IR_UpdateScaled(&s_ir[IR_SENSOR_FRONT_RIGHT]);
    IR_UpdateScaled(&s_ir[IR_SENSOR_LEFT]);
    IR_UpdateScaled(&s_ir[IR_SENSOR_RIGHT]);
}

float IR_ReadRaw(ir_sensor_id_t id)
{
    if ((uint32_t)id >= (uint32_t)IR_SENSOR_COUNT) {
        return IR_INVALID_VALUE;
    }

    return s_ir[id].raw;
}

float IR_ReadFiltered(ir_sensor_id_t id)
{
    if ((uint32_t)id >= (uint32_t)IR_SENSOR_COUNT) {
        return IR_INVALID_VALUE;
    }

    if (!s_ir[id].valid) {
        return IR_INVALID_VALUE;
    }

    return s_ir[id].filtered;
}

float IR_ReadScaled(ir_sensor_id_t id)
{
    if ((uint32_t)id >= (uint32_t)IR_SENSOR_COUNT) {
        return IR_INVALID_VALUE;
    }

    if (!s_ir[id].valid) {
        return IR_INVALID_VALUE;
    }

    return s_ir[id].scaled;
}

const IR_Channel_t* IR_GetChannel(ir_sensor_id_t id)
{
    if ((uint32_t)id >= (uint32_t)IR_SENSOR_COUNT) {
        return NULL;
    }

    return &s_ir[id];
}

float IR_EstimateDistanceMm(ir_sensor_id_t id, float k)
{
    float v = IR_ReadScaled(id);

    if (v <= 1.0f) {
        return 200.0f;
    }

    return k / sqrtf(v);
}

float readSensor_Left(void)
{
    return (float)IR_ReadHardwareScaled(IR_SENSOR_LEFT);
}

float readSensor_Left_45(void)
{
    return (float)IR_ReadHardwareScaled(IR_SENSOR_LEFT_45);
}

float readSensor_Right(void)
{
    return (float)IR_ReadHardwareScaled(IR_SENSOR_RIGHT);
}

float readSensor_Right_45(void)
{
    return (float)IR_ReadHardwareScaled(IR_SENSOR_RIGHT_45);
}

float readSensor_Front(void)
{
    return (float)IR_ReadHardwareScaled(IR_SENSOR_FRONT_LEFT);
}

float readSensor_Front2(void)
{
    return (float)IR_ReadHardwareScaled(IR_SENSOR_FRONT_RIGHT);
}

