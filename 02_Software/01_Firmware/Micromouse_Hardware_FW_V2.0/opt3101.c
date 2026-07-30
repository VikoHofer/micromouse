#include "opt3101.h"
#include "TimeOfLight.h"
#include "BLEDebugger.h"
#include <string.h>

#define OPT3101_REG_CONTROL        0x00U
#define OPT3101_FORCE_EN_SLAVE     (1UL << 22)
#define OPT3101_FORCE_EN_BYPASS    (1UL << 21)
#define OPT3101_I2C_CONT_RW      (1UL << 6)


#define OPT3101_REG_OUTPUT_BASE      0x08U
#define OPT3101_REG_MONOSHOT         0x27U
#define OPT3101_REG_FRAME_CFG        0x9FU
#define OPT3101_REG_TX_HDR_CFG       0x2AU
#define OPT3101_REG_ILLUM_DAC        0x29U
#define OPT3101_REG_LED_DRV          0x79U
#define OPT3101_REG_AMBIENT_CFG      0x72U

#define OPT3101_REG_TX_PIN_CFG 0x7AU


#define OPT3101_NUM_SUB_FRAMES       7U
#define OPT3101_NUM_AVG_SUB_FRAMES   7U

#define OPT3101_ILLUM_DAC_L          6U
#define OPT3101_ILLUM_DAC_H          10U

#define OPT3101_REG_TEMP_CONV      0x6EU
#define OPT3101_REG_TIMING_GEN     0x80U

#define OPT3101_TEMP_CONV_EN_VALUE 0x0A0000U  // reset 0x020000 + bit19
#define OPT3101_TG_EN_VALUE        0x004E1FU  // reset 0x004E1E + bit0

static const uint8_t s_addr[OPT3101_COUNT] =
{
    [OPT3101_LEFT_45]  = OPT3101_ADDR_LEFT_45,
    [OPT3101_RIGHT_45] = OPT3101_ADDR_RIGHT_45,
};

static OPT3101_Data_t s_data[OPT3101_COUNT];

static HAL_StatusTypeDef opt_write24(uint8_t addr, uint8_t reg, uint32_t value)
{
    uint8_t data[3];

    data[0] = (uint8_t)( value        & 0xFFU);
    data[1] = (uint8_t)((value >> 8)  & 0xFFU);
    data[2] = (uint8_t)((value >> 16) & 0xFFU);

    return HAL_I2C_Mem_Write(&hi2c2,
                             (uint16_t)(addr << 1),
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             data,
                             3,
                             100);
}

static HAL_StatusTypeDef opt_read_output(uint8_t addr,
                                          uint32_t *r8,
                                          uint32_t *r9,
                                          uint32_t *r10)
{
    uint8_t data[9];

    if (HAL_I2C_Mem_Read(&hi2c2,
                         (uint16_t)(addr << 1),
                         OPT3101_REG_OUTPUT_BASE,
                         I2C_MEMADD_SIZE_8BIT,
                         data,
                         sizeof(data),
                         20) != HAL_OK)
    {
        return HAL_ERROR;
    }

	*r8  = ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | data[0];
	*r9  = ((uint32_t)data[5] << 16) | ((uint32_t)data[4] << 8) | data[3];
	*r10 = ((uint32_t)data[8] << 16) | ((uint32_t)data[7] << 8) | data[6];
	
	printf("RAW R8=0x%06lX R9=0x%06lX R10=0x%06lX\r\n",
       (unsigned long)*r8,
       (unsigned long)*r9,
       (unsigned long)*r10);
	   
	  printf("DATA:");
for (int i = 0; i < 9; i++) {
    printf(" %02X", data[i]);
}
printf("\r\n");

    return HAL_OK;
}

static uint16_t opt_phase_to_mm(uint16_t phase)
{
    uint32_t um;

    /* PHASE_OUT LSB approx. 228.7 µm */
    um = (uint32_t)phase * 229U;

    return (uint16_t)(um / 1000U);
}

//static int opt_init_one(uint8_t addr)
//{
//    uint32_t frame_cfg;
//    uint32_t tx_hdr_cfg;
//    uint32_t illum_dac;

//    if (HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(addr << 1), 5, 100) != HAL_OK) {
//        return -1;
//    }
//	HAL_Delay(10);
//	if (opt_write24(addr, OPT3101_REG_CONTROL, OPT3101_FORCE_EN_SLAVE | OPT3101_FORCE_EN_BYPASS | OPT3101_I2C_CONT_RW | 0x01) != HAL_OK) {
//		return -10;		
//	}
//	HAL_Delay(10);
//	 /* Enable illumination driver */
//	if (opt_write24(addr, OPT3101_REG_LED_DRV, 0x000001U) != HAL_OK) {
//		return -7;
//	}
//	HAL_Delay(2);

//	/* TX0 pin function: TX0 = CLKB */
//	if (opt_write24(addr, OPT3101_REG_TX_PIN_CFG, 0x000030U) != HAL_OK) {
//		return -11;
//	}
//	
////	
////	if (opt_write24(addr, OPT3101_REG_CONTROL, OPT3101_FORCE_EN_SLAVE | OPT3101_FORCE_EN_BYPASS | OPT3101_I2C_CONT_RW) != HAL_OK) {
////		return -10;
////	}

////	HAL_Delay(2);

////    /* Temperature conversion enable: keep reserved reset value */
////    if (opt_write24(addr, OPT3101_REG_TEMP_CONV, OPT3101_TEMP_CONV_EN_VALUE) != HAL_OK) {
////        return -2;
////    }

////    /* Continuous mode */
////    if (opt_write24(addr, OPT3101_REG_MONOSHOT, 0x000000U) != HAL_OK) {
////        return -3;
////    }

////    /* 500 Hz: 8 subframes, 8 averaged subframes */
////    frame_cfg =
////        ((uint32_t)OPT3101_NUM_AVG_SUB_FRAMES << 12) |
////        ((uint32_t)OPT3101_NUM_SUB_FRAMES);

////    if (opt_write24(addr, OPT3101_REG_FRAME_CFG, frame_cfg) != HAL_OK) {
////        return -4;
////    }

////    /*
////     * Single TX0, fixed high-current path.
////     * EN_TX_SWITCH = 0
////     * SEL_TX_CH = 0
////     * EN_ADAPTIVE_HDR = 0
////     * SEL_HDR_MODE = 1
////     */
////    //tx_hdr_cfg = (1UL << 16);
////	tx_hdr_cfg = 0x000000U;   // SEL_HDR_MODE = 0 => ILLUM_DAC_L

////    if (opt_write24(addr, OPT3101_REG_TX_HDR_CFG, tx_hdr_cfg) != HAL_OK) {
////        return -5;
////    }

////    /*
////     * TX0 DAC:
////     * ILLUM_DAC_L_TX0 = bits [4:0]
////     * ILLUM_DAC_H_TX0 = bits [9:5]
////     */
////    illum_dac =
////        ((uint32_t)OPT3101_ILLUM_DAC_H << 5) |
////        ((uint32_t)OPT3101_ILLUM_DAC_L);

////    if (opt_write24(addr, OPT3101_REG_ILLUM_DAC, illum_dac) != HAL_OK) {
////        return -6;
////    }
////		/* GP1 = ILLUM_CLK */
////	opt_write24(addr, 0x78, 0x0011C0U);


////    /* Enable illumination driver */
////	if (opt_write24(addr, OPT3101_REG_LED_DRV, 0x000001U) != HAL_OK) {
////		return -7;
////	}

////	/* TX0 pin function: TX0 = CLKB */
////	if (opt_write24(addr, OPT3101_REG_TX_PIN_CFG, 0x000030U) != HAL_OK) {
////		return -11;
////	}

////	/* Ambient support */
////	if (opt_write24(addr, OPT3101_REG_AMBIENT_CFG, 0x000000U) != HAL_OK) {
////		return -8;
////	}

////	/* Enable timing generator last */
////	if (opt_write24(addr, OPT3101_REG_TIMING_GEN, OPT3101_TG_EN_VALUE) != HAL_OK) {
////		return -9;
////	}
////	

//    HAL_Delay(5);

//    return 0;
//}
static int opt_init_one(uint8_t addr)
{
    uint32_t v;

    if (HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(addr << 1), 5, 100) != HAL_OK) {
        return -1;
    }

    opt_write24(addr, 0x00, 0x600040U);
    HAL_Delay(2);

    opt_write24(addr, 0x79, 0x000001U);

    opt_read24(addr, 0x7A, &v);
    printf("7A before = 0x%06lX\r\n", v);

    v &= ~(3UL << 4);
    v |=  (3UL << 4);      // TX0 static 1
    opt_write24(addr, 0x7A, v);

    opt_read24(addr, 0x7A, &v);
    printf("7A after  = 0x%06lX\r\n", v);

    return 0;
}

int OPT3101_InitAll(void)
{
    memset(s_data, 0, sizeof(s_data));

//    if (opt_init_one(s_addr[OPT3101_LEFT_45]) != 0) {
//        return -1;
//    }
	printf("Init Begin\n");
    if (opt_init_one(s_addr[OPT3101_RIGHT_45]) != 0) {
        return -2;
    }
	printf("Init Done\n");

    return 0;
}

static void opt_update_one(opt3101_id_t id)
{
    uint32_t r8, r9, r10;
    OPT3101_Data_t *d;

    if ((uint32_t)id >= (uint32_t)OPT3101_COUNT) {
        return;
    }

    d = &s_data[id];

    if (opt_read_output(s_addr[id], &r8, &r9, &r10) != HAL_OK) {
        d->valid = 0U;
        d->new_data = 0U;
        d->distance_mm = OPT3101_INVALID_MM;
        return;
    }

    d->phase           = (uint16_t)(r8 & 0xFFFFU);
    d->amplitude       = (uint16_t)(r9 & 0xFFFFU);
    d->ambient         = (uint16_t)((r10 >> 2) & 0x03FFU);
    d->temperature_raw = (uint16_t)((r10 >> 12) & 0x0FFFU);

    d->valid             = (uint8_t)(((r8 >> 20) & 0x01U) != 0U);
    d->ambient_saturated = (uint8_t)(((r8 >> 22) & 0x01U) != 0U);
    d->saturated         = (uint8_t)(((r9 >> 18) & 0x01U) != 0U);

    if (d->valid && !d->saturated && !d->ambient_saturated) {
        d->distance_mm = opt_phase_to_mm(d->phase);
    } else {
        d->distance_mm = OPT3101_INVALID_MM;
    }

    d->new_data = 1U;
}

void OPT3101_UpdateAll(void)
{
    //opt_update_one(OPT3101_LEFT_45);
    opt_update_one(OPT3101_RIGHT_45);
}

uint16_t OPT3101_ReadDistanceMm(opt3101_id_t id)
{
    if ((uint32_t)id >= (uint32_t)OPT3101_COUNT) {
        return OPT3101_INVALID_MM;
    }

    if (!s_data[id].valid) {
        return OPT3101_INVALID_MM;
    }

    return s_data[id].distance_mm;
}

const OPT3101_Data_t* OPT3101_GetData(opt3101_id_t id)
{
    if ((uint32_t)id >= (uint32_t)OPT3101_COUNT) {
        return 0;
    }

    return &s_data[id];
}

HAL_StatusTypeDef opt_read24(uint8_t addr, uint8_t reg, uint32_t *value)
{
    uint8_t data[3];

    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(&hi2c2,
                                            (uint16_t)(addr << 1),
                                            reg,
                                            I2C_MEMADD_SIZE_8BIT,
                                            data,
                                            3,
                                            100);
    if (st != HAL_OK) {
        return st;
    }

    *value = ((uint32_t)data[2] << 16) |
             ((uint32_t)data[1] << 8)  |
             ((uint32_t)data[0]);

    return HAL_OK;
}