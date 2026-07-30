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

#define OPT3101_ILLUM_DAC_L          1U
#define OPT3101_ILLUM_DAC_H          1U

#define OPT3101_ILLUM_SCALE_L_TX0  3U
#define OPT3101_ILLUM_SCALE_H_TX0  3U

#define OPT3101_REG_TEMP_CONV      0x6EU
#define OPT3101_REG_TIMING_GEN     0x80U

#define OPT3101_TEMP_CONV_EN_VALUE 0x0A0000U  // reset 0x020000 + bit19
#define OPT3101_TG_EN_VALUE        0x004E1FU  // reset 0x004E1E + bit0

#define OPT3101_REG_SW_RESET  0x01U
#define OPT3101_SOFTWARE_RESET (1UL << 0)

#define OPT3101_REG_TG_OVL_WINDOW_START  0x89U
#define OPT3101_REG_CLIP_MODE            0x50U
#define OPT3101_REG_IQ_READ_CFG          0x2EU
#define OPT3101_REG_POWERUP_DELAY        0x26U
#define OPT3101_REG_POWER_CTRL           0x76U

#define OPT3101_REG_CONTROL        0x00U
#define OPT3101_REG_STATUS         0x03U
#define OPT3101_REG_POWERUP_DELAY  0x26U
#define OPT3101_REG_MONOSHOT       0x27U
#define OPT3101_REG_ILLUM_DAC      0x29U
#define OPT3101_REG_TX_HDR_CFG     0x2AU
#define OPT3101_REG_IQ_CFG         0x2EU
#define OPT3101_REG_CLIP_MODE      0x50U
#define OPT3101_REG_TEMP_CONV      0x6EU
#define OPT3101_REG_POWER_CTRL     0x76U
#define OPT3101_REG_LED_DRV        0x79U
#define OPT3101_REG_TIMING_GEN     0x80U
#define OPT3101_REG_TG_OVL_START   0x89U
#define OPT3101_REG_TG_SEQ_MASK    0x97U
#define OPT3101_REG_FRAME_CFG      0x9FU
#define OPT3101_REG_ILLUM_SCALE	   0x2BU

#define OPT3101_MONOSHOT_BIT       (1UL << 23)
#define OPT3101_INIT_LOAD_DONE     (1UL << 8)

#define OPT3101_FRAME_512          (((uint32_t)511U << 12) | 511U)


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
static int opt_write24_check(uint8_t addr, uint8_t reg, uint32_t value)
{
    uint32_t rb;

    if (opt_write24(addr, reg, value) != HAL_OK) {
        printf("WRITE FAIL reg 0x%02X\r\n", reg);
        return -1;
    }

    HAL_Delay(2);

    if (opt_read24(addr, reg, &rb) != HAL_OK) {
        printf("READBACK FAIL reg 0x%02X\r\n", reg);
        return -2;
    }

    printf("WRITE reg 0x%02X = 0x%06lX, readback = 0x%06lX\r\n",
           reg,
           (unsigned long)(value & 0xFFFFFFU),
           (unsigned long)rb);

    return 0;
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
	
//	printf("RAW R8=0x%06lX R9=0x%06lX R10=0x%06lX\r\n",
//       (unsigned long)*r8,
//       (unsigned long)*r9,
//       (unsigned long)*r10);
//	   
//	  printf("DATA:");
//for (int i = 0; i < 9; i++) {
//    printf(" %02X", data[i]);
//}
//printf("\r\n");

    return HAL_OK;
}

static uint16_t opt_phase_to_mm(uint16_t phase)
{
    return (uint16_t)(((uint32_t)phase * 14990U) >> 16);
}

static HAL_StatusTypeDef opt_software_reset(uint8_t addr)
{
    uint8_t data = 0x01U;

    return HAL_I2C_Mem_Write(&hi2c2,
                             (uint16_t)(addr << 1),
                             OPT3101_REG_CONTROL,
                             I2C_MEMADD_SIZE_8BIT,
                             &data,
                             1,
                             100);
}

static int opt_wait_init_load_done(uint8_t addr)
{
    uint32_t v;

    for (uint32_t i = 0; i < 100; i++) {
        HAL_Delay(1);

        if (opt_read24(addr, OPT3101_REG_STATUS, &v) != HAL_OK) {
            printf("INIT_LOAD_DONE read fail\r\n");
            return -1;
        }

        printf("REG 0x03 = 0x%06lX\r\n", (unsigned long)v);

        if (v != 0U) {
            return 0;
        }
    }

    return -2;
}

static int opt_calibrate_illum_xtalk(uint8_t addr)
{
    uint32_t reg2e;

    if (opt_read24(addr, 0x2E, &reg2e) != HAL_OK) return -1;

    reg2e &= ~((uint32_t)0xF << 20);
    reg2e |=  ((uint32_t)1 << 20);   // XTALK_FILT_TIME_CONST = 1

    reg2e |=  (1U << 7);             // USE_XTALK_FILT_ILLUM = 1
    reg2e &= ~(1U << 8);             // USE_XTALK_REG_ILLUM = 0
    reg2e |=  (1U << 12);            // ILLUM_XTALK_CALIB = 1

    if (opt_write24_check(addr, 0x2E, reg2e) != 0) return -2;

    for (int i = 0; i < 12; i++) {
        OPT3101_UpdateAll();
    }

    reg2e &= ~(1U << 12);            // ILLUM_XTALK_CALIB = 0

    if (opt_write24_check(addr, 0x2E, reg2e) != 0) return -3;

    return 0;
}

static int opt_init_one(uint8_t addr)
{
    uint32_t reg2e = 0;
    uint32_t illum_dac = 0;
	uint32_t reg2b = 0;

    if (HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(addr << 1), 5, 100) != HAL_OK) {
        return -1;
    }

    /* Software reset: only write first data byte */
    if (opt_software_reset(addr) != HAL_OK) {
        return -2;
    }

    if (opt_wait_init_load_done(addr) != 0) {
        return -3;
    }
	if (opt_write24(addr, OPT3101_REG_CONTROL,
                OPT3101_FORCE_EN_SLAVE | OPT3101_I2C_CONT_RW) != HAL_OK) {
    return -4;
}

    /* Basic configuration from reference implementation */
    if (opt_write24_check(addr, OPT3101_REG_TG_OVL_START, 7000U) != HAL_OK) {
        return -4;
    }

    if (opt_write24_check(addr, OPT3101_REG_TEMP_CONV, 0x0A0000U) != HAL_OK) {
        return -5;
    }

    if (opt_write24_check(addr, OPT3101_REG_CLIP_MODE, 0x200101U) != HAL_OK) {
        return -6;
    }

    /* IQ_READ_DATA_SEL = 2 */
    if (opt_read24(addr, OPT3101_REG_IQ_CFG, &reg2e) != HAL_OK) {
        return -7;
    }

    reg2e = (reg2e & ~((uint32_t)7U << 9)) | ((uint32_t)2U << 9);

    if (opt_write24_check(addr, OPT3101_REG_IQ_CFG, reg2e) != HAL_OK) {
        return -8;
    }

    /*
     * Monoshot mode:
     * MONOSHOT_FZ_CLKCNT = default
     * MONOSHOT_NUMFRAME = 1
     * MONOSHOT_MODE = 3
     */
    if (opt_write24_check(addr, OPT3101_REG_MONOSHOT, 0x26AC07U) != HAL_OK) {
        return -9;
    }

    /*
     * Keep internal oscillator / ambient DAC / reference system powered.
     */
    if (opt_write24_check(addr, OPT3101_REG_POWER_CTRL, 0x000121U) != HAL_OK) {
        return -10;
    }

    /* POWERUP_DELAY = 95 */
    if (opt_write24_check(addr, OPT3101_REG_POWERUP_DELAY,
                    ((uint32_t)95U << 10) | 0xFU) != HAL_OK) {
        return -11;
    }

    /*
     * 512 subframes:
     * (subFrameCount - 1) | ((subFrameCount - 1) << 12)
     */
    if (opt_write24_check(addr, OPT3101_REG_FRAME_CFG, OPT3101_FRAME_512) != HAL_OK) {
        return -12;
    }

    if (opt_write24_check(addr, OPT3101_REG_TG_SEQ_MASK, OPT3101_FRAME_512) != HAL_OK) {
        return -13;
    }

    /*
     * Single TX0, no TX auto switch, low HDR path.
     */
    if (opt_write24_check(addr, OPT3101_REG_TX_HDR_CFG, 0x000000U) != HAL_OK) {
        return -14;
    }

    /*
     * Start with very low current to avoid saturation.
     * L = 1, H = 1 -> 0x000021
     */
    //illum_dac = ((uint32_t)1U << 5) | 1U;
	illum_dac =
    ((uint32_t)OPT3101_ILLUM_DAC_H << 5) |
    ((uint32_t)OPT3101_ILLUM_DAC_L);

    if (opt_write24_check(addr, OPT3101_REG_ILLUM_DAC, illum_dac) != HAL_OK) {
        return -15;
    }
	
	reg2b |= ((uint32_t)OPT3101_ILLUM_SCALE_L_TX0 << 16);
	reg2b |= ((uint32_t)OPT3101_ILLUM_SCALE_H_TX0 << 19);

	opt_write24_check(addr, OPT3101_REG_ILLUM_SCALE, reg2b);

    /* Enable illumination driver */
    if (opt_write24_check(addr, OPT3101_REG_LED_DRV, 0x000001U) != HAL_OK) {
        return -16;
    }

    /* Enable timing generator */
    if (opt_write24_check(addr, OPT3101_REG_TIMING_GEN, 0x004E1FU) != HAL_OK) {
        return -17;
    }
	
    HAL_Delay(150);
	
	opt_calibrate_illum_xtalk(addr);


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
static int opt_trigger_sample(uint8_t addr)
{
    uint32_t ctrl_base = OPT3101_FORCE_EN_SLAVE | OPT3101_I2C_CONT_RW;

    if (opt_write24(addr, OPT3101_REG_CONTROL, ctrl_base) != HAL_OK) {
        return -1;
    }

    //HAL_Delay(1);

    if (opt_write24(addr, OPT3101_REG_CONTROL, ctrl_base | OPT3101_MONOSHOT_BIT) != HAL_OK) {
        return -2;
    }

    HAL_Delay(20);

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
	
	opt_trigger_sample(s_addr[id]);

    if (opt_read_output(s_addr[id], &r8, &r9, &r10) != HAL_OK) {
        d->valid = 0U;
        d->new_data = 0U;
        d->distance_mm = OPT3101_INVALID_MM;
        return;
    }

	d->phase = (uint16_t)(r8 & 0xFFFFU);
	d->amplitude = (uint16_t)(r9 & 0xFFFFU);
	d->ambient = (uint16_t)((r10 >> 2) & 0x03FFU);
	d->temperature_raw = (uint16_t)((r10 >> 12) & 0x0FFFU);

	d->valid = (uint8_t)(((r8 >> 20) & 0x01U) != 0U);
	d->ambient_saturated = (uint8_t)(((r8 >> 22) & 0x01U) != 0U);
	d->saturated = (uint8_t)(((r9 >> 18) & 0x01U) != 0U);

    if (d->valid && !d->saturated && !d->ambient_saturated) {
        d->distance_mm = opt_phase_to_mm(d->phase);
    } else {
        d->distance_mm = OPT3101_INVALID_MM;
    }

    d->new_data = 1U;
	
	uint8_t tx_channel = (r8 >> 18) & 0x03U;
	uint8_t hdr_mode   = (r8 >> 17) & 0x01U;
	uint8_t frame_stat = (r8 >> 20) & 0x01U;
	uint8_t frame_cnt0 = (r8 >> 23) & 0x01U;

//	printf("tx=%u hdr=%u frame_stat=%u fc0=%u phase12=%u amp=%u amb=%u\r\n",
//	   tx_channel,
//	   hdr_mode,
//	   frame_stat,
//	   frame_cnt0,
//	   (uint16_t)(r8 & 0x0FFFU),
//	   d->amplitude,
//	   d->ambient);
//	uint16_t amp12 = r9 & 0x0FFFU;
//	uint16_t amp14 = r9 & 0x3FFFU;
//	uint16_t amp16 = r9 & 0xFFFFU;

//	printf("amp12=%u amp14=%u amp16=%u raw9=0x%06lX\r\n",
//	   amp12, amp14, amp16, (unsigned long)r9);
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