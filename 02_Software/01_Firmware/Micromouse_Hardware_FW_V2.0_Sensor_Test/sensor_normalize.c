/*************************************************************
Project     : Micromouse V1.2
File        : sensor_normalize.c
Description : IR normalization for diagonal wall sensors only
Date        : 15.04.2026
*************************************************************/

#include "sensor_normalize.h"

#include "adc.h"
#include "config.h"
#include "sensor.h"
#include "Delay.h"

#include <string.h>
#include <math.h>

/* ----------------------------------------------------------
 * Fallback configuration
 * ---------------------------------------------------------- */

#ifndef IR_ALPHA_45
#define IR_ALPHA_45 0.25f
#endif


#ifndef IR_CALIB_SAMPLES
#define IR_CALIB_SAMPLES 32U
#endif

#ifndef IR_CALIB_DELAY_MS
#define IR_CALIB_DELAY_MS 5U
#endif

#ifndef IR_REF_LEFT_45
#define IR_REF_LEFT_45 2700.0f
#endif

#ifndef IR_REF_RIGHT_45
#define IR_REF_RIGHT_45 2900.0f
#endif

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

/*
 * New sensor.h mapping:
 *
 * SENSOR_ID_1 = RIGHT_45
 *   ADC    : PB0
 *   Switch : PC13
 *
 * SENSOR_ID_2 = LEFT_45
 *   ADC    : PA4
 *   Switch : PC2
 */
static const IR_ChannelConfig_t s_cfg[IR_SENSOR_COUNT] =
{
    [IR_SENSOR_RIGHT_45] = { SENSOR_ID_1, IR_ALPHA_45, 0U, 0.0f },
    [IR_SENSOR_LEFT_45]  = { SENSOR_ID_2, IR_ALPHA_45, 0U, 0.0f }
};

static IR_Channel_t s_ir[IR_SENSOR_COUNT];

/* ----------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------- */

static uint8_t IR_IsValidId(ir_sensor_id_t id)
{
    return ((uint32_t)id < (uint32_t)IR_SENSOR_COUNT);
}

float IR_ReadHardware(ir_sensor_id_t id)
{
    uint32_t sensor_value = 0U;
    float value;

    if (!IR_IsValidId(id)) {
        return IR_INVALID_VALUE;
    }

    if (Sensor_Read(s_cfg[id].hw_id, &hadc1, &sensor_value) != SENSOR_OK) {
        return IR_INVALID_VALUE;
    }

    if (sensor_value > 4095U) {
        return IR_INVALID_VALUE;
    }

    value = (float)sensor_value - s_cfg[id].offset;

    if (value < 0.0f) {
        value = 0.0f;
    }

    return value;
}

float IR_ReadHardwareScaled(ir_sensor_id_t id)
{
    float raw;
    float ref;

    if (!IR_IsValidId(id)) {
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

static float IR_ApplyFilter(IR_Channel_t *ch,
                            float raw,
                            float alpha,
                            uint8_t use_filter)
{
    if (ch == NULL) {
        return IR_INVALID_VALUE;
    }

    if (raw < 0.0f) {
        ch->valid = 0U;
        ch->filtered = IR_INVALID_VALUE;
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

    if (!IR_IsValidId(id)) {
        return 0.0f;
    }

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

    s_ir[IR_SENSOR_RIGHT_45].offset = s_cfg[IR_SENSOR_RIGHT_45].offset;
    s_ir[IR_SENSOR_LEFT_45].offset  = s_cfg[IR_SENSOR_LEFT_45].offset;

    s_ir[IR_SENSOR_RIGHT_45].reference = IR_REF_RIGHT_45;
    s_ir[IR_SENSOR_LEFT_45].reference  = IR_REF_LEFT_45;

    s_ir[IR_SENSOR_RIGHT_45].scale = 1.0f;
    s_ir[IR_SENSOR_LEFT_45].scale  = 1.0f;
}

void IR_Update(void)
{
    float raw_right;
    float raw_left;

    raw_right = IR_ReadHardware(IR_SENSOR_RIGHT_45);
	raw_left  = IR_ReadHardware(IR_SENSOR_LEFT_45);

	if (raw_right >= 0.0f) {
		s_ir[IR_SENSOR_RIGHT_45].raw = raw_right;
		(void)IR_ApplyFilter(&s_ir[IR_SENSOR_RIGHT_45],
							 raw_right,
							 s_cfg[IR_SENSOR_RIGHT_45].alpha,
							 s_cfg[IR_SENSOR_RIGHT_45].use_filter);
		IR_UpdateScaled(&s_ir[IR_SENSOR_RIGHT_45]);
	}

	if (raw_left >= 0.0f) {
		s_ir[IR_SENSOR_LEFT_45].raw = raw_left;
		(void)IR_ApplyFilter(&s_ir[IR_SENSOR_LEFT_45],
							 raw_left,
							 s_cfg[IR_SENSOR_LEFT_45].alpha,
							 s_cfg[IR_SENSOR_LEFT_45].use_filter);
		IR_UpdateScaled(&s_ir[IR_SENSOR_LEFT_45]);
	}
}

void IR_CalibrateReference(void)
{
    s_ir[IR_SENSOR_RIGHT_45].reference = IR_AverageSensor(IR_SENSOR_RIGHT_45);
    s_ir[IR_SENSOR_LEFT_45].reference  = IR_AverageSensor(IR_SENSOR_LEFT_45);

    IR_UpdateScaled(&s_ir[IR_SENSOR_RIGHT_45]);
    IR_UpdateScaled(&s_ir[IR_SENSOR_LEFT_45]);
}

float IR_ReadRaw(ir_sensor_id_t id)
{
    if (!IR_IsValidId(id)) {
        return IR_INVALID_VALUE;
    }

    return s_ir[id].raw;
}

float IR_ReadFiltered(ir_sensor_id_t id)
{
    if (!IR_IsValidId(id)) {
        return IR_INVALID_VALUE;
    }

    if (!s_ir[id].valid) {
        return IR_INVALID_VALUE;
    }

    return s_ir[id].filtered;
}

float IR_ReadScaled(ir_sensor_id_t id)
{
    if (!IR_IsValidId(id)) {
        return IR_INVALID_VALUE;
    }

    if (!s_ir[id].valid) {
        return IR_INVALID_VALUE;
    }

    return s_ir[id].scaled;
}

const IR_Channel_t* IR_GetChannel(ir_sensor_id_t id)
{
    if (!IR_IsValidId(id)) {
        return NULL;
    }

    return &s_ir[id];
}

float IR_EstimateDistanceMm(ir_sensor_id_t id, float k)
{
    float v;

    if (!IR_IsValidId(id)) {
        return IR_INVALID_VALUE;
    }

    v = IR_ReadScaled(id);

    if (v <= 1.0f) {
        return 200.0f;
    }

    return k / sqrtf(v);
}

float readSensor_Right_45(void)
{
    return IR_ReadHardwareScaled(IR_SENSOR_RIGHT_45);
}

float readSensor_Left_45(void)
{
    return IR_ReadHardwareScaled(IR_SENSOR_LEFT_45);
}