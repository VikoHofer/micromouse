/*************************************************************
Project     : Micromouse V2.0
File        : motion_controller.c
Description : Incremental forward / rotation controller
Date        : 18.03.2026
*************************************************************/

#include "motion_control.h"
#include "encoder.h"
#include "motor_driver.h"
#include "config.h"
#include "profile.h"
#include "timer.h"
#include "math.h"
#include "TimeOfLight.h"
#include "string.h"
#include "BLEDebugger.h"
#include <stdio.h>

static const uint8_t TIME_HEADER[5] = "TIME\n"; 
static const uint8_t VELOCITY_HEADER[9] = "VELOCITY\n"; 
#define RAD_TO_DEG 57.29578f

MotionController_t motion_ctrl;

Profile forward_profile;
Profile rotation_profile;

#define TOF_IDX_RM                  1U
#define TOF_IDX_LM                  4U

//#define WALL_STEER_ENABLE           0
static bool WALL_STEER_ENABLE = true;

#define TOF_VALID_MIN_MM            10U
#define TOF_VALID_MAX_MM            76U

#define WALL_DEADBAND_MM            5 // 10 gut
#define WALL_MAX_ADJUST_DPS         450.0f

// 4.5 bei 300 mm/s
// 3.5 bei 600 mm/s
#define WALL_KP                     8																																				
#define WALL_KD                     2

#define WALL_FILTER_ALPHA           0   /* 0..1, höher = ruhiger */

static uint16_t s_tof_rm_mm = VL6180X_INVALID_RANGE;
static uint16_t s_tof_lm_mm = VL6180X_INVALID_RANGE;

static volatile uint16_t s_debug_tof_rm_mm = VL6180X_INVALID_RANGE;
static volatile uint16_t s_debug_tof_lm_mm = VL6180X_INVALID_RANGE;
static volatile float s_debug_wall_error = 0.0f;
static volatile float s_debug_steering_dps = 0.0f;
//debug
static volatile uint32_t s_debug_rm_updates = 0;
static volatile uint32_t s_debug_lm_updates = 0;

static volatile uint32_t s_debug_rm_hz = 0;
static volatile uint32_t s_debug_lm_hz = 0;
static uint32_t s_update_freq_last_tick = 0;

static float s_wall_error_filt = 0.0f;
static float s_wall_error_prev = 0.0f;
static uint8_t s_tof_divider = 0;
static uint32_t  isr_counter = 0;
/* ----------------------------------------------------------
* TOF Sensor Section
* ---------------------------------------------------------- */

// steering adjustment
static void MotionController_ToFInit(void)
{
	if (WALL_STEER_ENABLE) {
		//    (void)VL6180X_StartContinuousAt(tof_sensor_addr[TOF_IDX_RM], 20);
	//    (void)VL6180X_StartContinuousAt(tof_sensor_addr[TOF_IDX_LM], 20);

		s_tof_rm_mm = VL6180X_INVALID_RANGE;
		s_tof_lm_mm = VL6180X_INVALID_RANGE;
		s_wall_error_filt = 0.0f;
		s_wall_error_prev = 0.0f;
		s_tof_divider = 0;

		s_debug_rm_updates = 0;
		s_debug_lm_updates = 0;
		s_debug_rm_hz = 0;
		s_debug_lm_hz = 0;
		s_update_freq_last_tick = HAL_GetTick();
	}

}



static void MotionController_UpdateToFCache(void)
{
	if (WALL_STEER_ENABLE){
		
	  uint16_t rm = VL6180X_INVALID_RANGE;
		uint16_t lm = VL6180X_INVALID_RANGE;
		uint32_t now;

		s_tof_divider++;
		if (s_tof_divider < 5U) {
			return;
		}
		s_tof_divider = 0U;
		
		if (VL6180X_ReadContinuousIfReadyAt(tof_sensor_addr[TOF_IDX_RM], &rm) == TOF_OK) {
			//RHA Erkennung das Ende von forward kommt - Korrektur FUNKTIONIERT NICHT!
//			float remainingDistance = Profile_GetRemainingDistance(&forward_profile);
//			if(remainingDistance < 135.0f){ //Weniger als 1 Feld zu fahren
//				if(rm > s_tof_rm_mm + 50){ //Sensor bemerkt einen Sprung (Wand fehlt, Kurve kommt)
//					
//					float currentPosition = Profile_GetPosition(&forward_profile);
//					//printf("currentPosition: %f, sensorValuesCount: %u, totalSensorValues: %u, average: %u\n",currentPosition, sensorValuesCountRm, totalSensorValuesRm, average);
//					Profile_SetPosition(&forward_profile, 0);
//					Profile_SetFinalPosition(&forward_profile,55);
//				
//				}
//			}
			s_tof_rm_mm = rm;
			s_debug_rm_updates++;
		}

		if (VL6180X_ReadContinuousIfReadyAt(tof_sensor_addr[TOF_IDX_LM], &lm) == TOF_OK) {
			//RHA Erkennung das Ende von forward kommt - Korrektur
//			float remainingDistance = Profile_GetRemainingDistance(&forward_profile);
//			if(remainingDistance < 135.0f){ //Weniger als 1 Feld zu fahren
//				if(lm > s_tof_lm_mm + 50){ //Sensor bemerkt einen Sprung (Wand fehlt, Kurve kommt)
//					float currentPosition = Profile_GetPosition(&forward_profile);
//					//printf("currentPosition: %f, sensorValuesCount: %u, totalSensorValues: %u, average: %u\n", currentPosition, sensorValuesCountLm, totalSensorValuesLm, average);
//					Profile_SetPosition(&forward_profile, 0);
//					Profile_SetFinalPosition(&forward_profile,55);
//				}
//			}
			s_tof_lm_mm = lm;
			s_debug_lm_updates++;
		}

		now = HAL_GetTick();
		if ((now - s_update_freq_last_tick) >= 1000U) {
			s_debug_rm_hz = s_debug_rm_updates;
			s_debug_lm_hz = s_debug_lm_updates;

			s_debug_rm_updates = 0;
			s_debug_lm_updates = 0;
			s_update_freq_last_tick = now;
		}
	
	}

}

static float MotionController_CalcWallSteeringDps(void)
{
	if (WALL_STEER_ENABLE) {
		float error_mm = 0.0f;
		float derror;
		float adjust_dps;
		uint8_t left_valid;
		uint8_t right_valid;

		/* Kalibrierte Mittelwerte bei mittiger Maus */
		const float left_middle  = 63.5f;
		const float right_middle = 73.1f;

		float left_error  = 0.0f;
		float right_error = 0.0f;

		left_valid  = (s_tof_lm_mm != VL6180X_INVALID_RANGE) &&
					  (s_tof_lm_mm >= TOF_VALID_MIN_MM) &&
					  (s_tof_lm_mm <= TOF_VALID_MAX_MM);

		right_valid = (s_tof_rm_mm != VL6180X_INVALID_RANGE) &&
					  (s_tof_rm_mm >= TOF_VALID_MIN_MM) &&
					  (s_tof_rm_mm <= TOF_VALID_MAX_MM);

		/* Kein gültiger Wandwert */
		if (!left_valid && !right_valid) {
			s_wall_error_prev = 0.0f;
			s_wall_error_filt = 0.0f;
			return 0.0f;
		}

		/* Einzelfehler relativ zum kalibrierten Mittelwert */
		if (left_valid) {
			left_error = (float)s_tof_lm_mm - left_middle;
		}

		if (right_valid) {
			right_error = right_middle - (float)s_tof_rm_mm;
		}

		/* Beide Wände sichtbar -> Mittelwert der beiden normierten Fehler */
		if (left_valid && right_valid) {
			error_mm = 0.5f * (left_error + right_error);
		}
		/* Nur linke Wand sichtbar */
		else if (left_valid) {
			error_mm = left_error;
		}
		/* Nur rechte Wand sichtbar */
		else {
			error_mm = right_error;
		}

		if (fabsf(error_mm) < WALL_DEADBAND_MM) {
			error_mm = 0.0f;
		}

		s_wall_error_filt = WALL_FILTER_ALPHA * s_wall_error_filt +
							(1.0f - WALL_FILTER_ALPHA) * error_mm;

		/* Falls dein Motion-Timer mit LOOP_INTERVAL läuft, hier besser LOOP_INTERVAL verwenden */
		derror = (s_wall_error_filt - s_wall_error_prev) / CONTROL_PERIOD_S;
		s_wall_error_prev = s_wall_error_filt;

		adjust_dps = WALL_KP * s_wall_error_filt + WALL_KD * derror;

		if (adjust_dps > WALL_MAX_ADJUST_DPS) {
			adjust_dps = WALL_MAX_ADJUST_DPS;
		}
		if (adjust_dps < -WALL_MAX_ADJUST_DPS) {
			adjust_dps = -WALL_MAX_ADJUST_DPS;
		}

		return adjust_dps;
	
	} else {
		return 0.0f;
	}

}

/* ----------------------------------------------------------
 * Callback Timer Functions
 * ---------------------------------------------------------- */

static float forward_circle = 141 * 5;
static float omega_circle = 90 * 5;
	
static void cb_update_test_sensor(int id)
{
    float steering_adjustment_dps = 0.0f;

    (void)id;

    encoder_update();

    MotionController_UpdateToFCache();
    //steering_adjustment_dps = MotionController_CalcWallSteeringDps();
	s_debug_tof_rm_mm = s_tof_rm_mm;
    s_debug_tof_lm_mm = s_tof_lm_mm;
    s_debug_wall_error = s_wall_error_filt;
    s_debug_steering_dps = steering_adjustment_dps;
	
	// for debugging
	steering_adjustment_dps = 0;

    MotionController_SetCommand(
        forward_circle,   /* mm/s */
        omega_circle                                  /* deg/s */
    );

    MotionController_Update(steering_adjustment_dps);
}


static void cb_update_forward(int id)
{
  isr_counter++;  
	float steering_adjustment_dps = 0.0f;
		
    (void)id;

    encoder_update();
    Profile_Update(&forward_profile);

    MotionController_UpdateToFCache();
    steering_adjustment_dps = MotionController_CalcWallSteeringDps();

    MotionController_SetCommand(
        Profile_GetSpeed(&forward_profile),   /* mm/s */
        0.0f                                  /* deg/s */
    );

    MotionController_Update(steering_adjustment_dps);
}

static void cb_update_rotation(int id)
{
    (void)id;
	
	encoder_update();
    Profile_Update(&rotation_profile);

    MotionController_SetCommand(
        0.0f,                                 /* mm/s */
        Profile_GetSpeed(&rotation_profile)   /* deg/s */
    );

    MotionController_Update(0.0f);
}

static void cb_update_combined(int id)
{
    float steering_adjustment_dps = 0.0f;

    (void)id;

    encoder_update();
    Profile_Update(&forward_profile);
    Profile_Update(&rotation_profile);

    MotionController_UpdateToFCache();
    steering_adjustment_dps = MotionController_CalcWallSteeringDps();

    MotionController_SetCommand(
        Profile_GetSpeed(&forward_profile),
        Profile_GetSpeed(&rotation_profile)
    );

    MotionController_Update(steering_adjustment_dps);
}

/* ----------------------------------------------------------
 * Motion Controll
 * ---------------------------------------------------------- */
void Motion_Init(void)
{
    uint32_t period_ms = (uint32_t)(LOOP_INTERVAL * 1000.0f);

    Timer_Initialize(TIMER_ID_TIM2,  period_ms, cb_update_forward);
    Timer_Initialize(TIMER_ID_TIM9,  period_ms, cb_update_rotation);
    Timer_Initialize(TIMER_ID_TIM12, period_ms, cb_update_combined);

    Timer_Stop(TIMER_ID_TIM2);
    Timer_Stop(TIMER_ID_TIM9);
    Timer_Stop(TIMER_ID_TIM12);

    MotionController_Init();
    MotionController_Reset();
	MotionController_ToFInit();
}

static void motion_stop_all(void)
{
    MotionController_SetCommand(0.0f, 0.0f);
    MotionController_Stop();
}


/* ---------------------------------------------------------- */
/* Helpers                                                    */
/* ---------------------------------------------------------- */

static float clampf(float x, float min_val, float max_val)
{
    if (x < min_val) return min_val;
    if (x > max_val) return max_val;
    return x;
}

/* ---------------------------------------------------------- */
/* Feedforwar calculation                                                    */
/* ---------------------------------------------------------- */

static float s_prev_left_speed_ref_mms = 0.0f;
static float s_prev_right_speed_ref_mms = 0.0f;

static float signf_nonzero(float x)
{
    if (x > 0.0f) return 1.0f;
    if (x < 0.0f) return -1.0f;
    return 0.0f;
}

static float MotionController_LeftFeedForward(float speed_ref_mms)
{
    float ff;
    float acc_ref_mms2 = 0.0f;

    ff = FF_KV * speed_ref_mms;

    if (speed_ref_mms > 0.0f) {
        ff += FF_KS;
    } else if (speed_ref_mms < 0.0f) {
        ff -= FF_KS;
    }

	
#if FF_USE_ACCELERATION
    acc_ref_mms2 = (speed_ref_mms - s_prev_left_speed_ref_mms) / CONTROL_PERIOD_S;
    ff += FF_KA * acc_ref_mms2;
#endif

    s_prev_left_speed_ref_mms = speed_ref_mms;
    return ff;
}

static float MotionController_RightFeedForward(float speed_ref_mms)
{
    float ff;
    float acc_ref_mms2 = 0.0f;

    ff = FF_KV * speed_ref_mms;

    if (speed_ref_mms > 0.0f) {
        ff += FF_KS;
    } else if (speed_ref_mms < 0.0f) {
        ff -= FF_KS;
    }

#if FF_USE_ACCELERATION
    acc_ref_mms2 = (speed_ref_mms - s_prev_right_speed_ref_mms) / CONTROL_PERIOD_S;
    ff += FF_KA * acc_ref_mms2;
#endif

    s_prev_right_speed_ref_mms = speed_ref_mms;
    return ff;
}
/* ---------------------------------------------------------- */
/* Internal controllers                                       */
/* ---------------------------------------------------------- */


/*
 * Incremental forward controller
 *
 * error[k] = error[k-1] + (v_ref * Ts - ds_measured)
 *
 * where:
 *   v_ref * Ts     = desired forward distance increment [mm]
 *   ds_measured    = measured forward distance increment [mm]
 */
static float forward_controller(void)
{
    float increment_mm;
    float diff;
    float output;

    increment_mm = motion_ctrl.velocity_ref_mms * CONTROL_PERIOD_S;

    motion_ctrl.fwd_error += increment_mm - encoder_get_forward_change_mm();

    diff = motion_ctrl.fwd_error - motion_ctrl.previous_fwd_error;
    motion_ctrl.previous_fwd_error = motion_ctrl.fwd_error;

    output = FWD_KP * motion_ctrl.fwd_error + FWD_KD * diff;
    return output;
}

/*
 * Incremental rotation controller in DEGREE
 *
 * error[k] = error[k-1] + (omega_ref * Ts - dphi_measured)
 *
 * where:
 *   omega_ref * Ts = desired angular increment [deg]
 *   dphi_measured  = measured angular increment [deg]
 *
 * steering_adjustment_dps is treated as an additional yaw rate command [deg/s].
 */
static float rotation_controller(float steering_adjustment_dps)
{
    float increment_deg;
    float diff;
    float output;

    increment_deg = motion_ctrl.omega_ref_dps * CONTROL_PERIOD_S;

    motion_ctrl.rot_error += increment_deg - encoder_get_rotation_change_deg();
    motion_ctrl.rot_error += steering_adjustment_dps * CONTROL_PERIOD_S;

    diff = motion_ctrl.rot_error - motion_ctrl.previous_rot_error;
    motion_ctrl.previous_rot_error = motion_ctrl.rot_error;

    output = ROT_KP * motion_ctrl.rot_error + ROT_KD * diff;
    return output;
}


/* ---------------------------------------------------------- */
/* Public API                                                 */
/* ---------------------------------------------------------- */


void MotionController_Init(void)
{
    motion_ctrl.controller_output_enabled = true;

    motion_ctrl.previous_fwd_error = 0.0f;
    motion_ctrl.previous_rot_error = 0.0f;

    motion_ctrl.fwd_error = 0.0f;
    motion_ctrl.rot_error = 0.0f;

    motion_ctrl.velocity_ref_mms = 0.0f;
    motion_ctrl.omega_ref_dps = 0.0f;

    motion_ctrl.left_output_pwm = 0.0f;
    motion_ctrl.right_output_pwm = 0.0f;
}

void MotionController_Reset(void)
{
    motion_ctrl.previous_fwd_error = 0.0f;
    motion_ctrl.previous_rot_error = 0.0f;

    motion_ctrl.fwd_error = 0.0f;
    motion_ctrl.rot_error = 0.0f;

    motion_ctrl.velocity_ref_mms = 0.0f;
    motion_ctrl.omega_ref_dps = 0.0f;

    motion_ctrl.left_output_pwm = 0.0f;
    motion_ctrl.right_output_pwm = 0.0f;
	
	s_prev_left_speed_ref_mms = 0.0f;
    s_prev_right_speed_ref_mms = 0.0f;
}

void MotionController_Enable(void)
{
    motion_ctrl.controller_output_enabled = true;
}

void MotionController_Disable(void)
{
    motion_ctrl.controller_output_enabled = false;
}

void MotionController_Stop(void)
{
    motion_ctrl.velocity_ref_mms = 0.0f;
    motion_ctrl.omega_ref_dps = 0.0f;

    motion_ctrl.left_output_pwm = 0.0f;
    motion_ctrl.right_output_pwm = 0.0f;
	
	s_prev_left_speed_ref_mms = 0.0f;
    s_prev_right_speed_ref_mms = 0.0f;

    Motor_Left_SetPWM(0);
    Motor_Right_SetPWM(0);
}

void MotionController_SetCommand(float velocity_mms, float omega_dps)
{
    motion_ctrl.velocity_ref_mms = velocity_mms;
    motion_ctrl.omega_ref_dps = omega_dps;
}


void MotionController_Update(float steering_adjustment_dps)
{
    float pos_output;
    float rot_output;
    float left_output;
    float right_output;

    float omega_total_dps;
    float omega_total_rad_s;
    float tangent_speed_mms;
    float left_speed_ref_mms;
    float right_speed_ref_mms;
    float left_ff = 0.0f;
    float right_ff = 0.0f;

    pos_output = forward_controller();
    rot_output = rotation_controller(steering_adjustment_dps);

    /* normal feedback control */
    left_output  = pos_output + rot_output;
    right_output = pos_output - rot_output;

	/* activate feedforwar*/
#if FEEDFORWARD
	
	// -> calculate set velocity for each wheel independently
    omega_total_dps = motion_ctrl.omega_ref_dps;
    omega_total_rad_s = omega_total_dps * ((float)M_PI / 180.0f);

    tangent_speed_mms = omega_total_rad_s * (WHEEL_BASE_MM * 0.5f);

    left_speed_ref_mms  = motion_ctrl.velocity_ref_mms + tangent_speed_mms;
    right_speed_ref_mms = motion_ctrl.velocity_ref_mms - tangent_speed_mms;
	
	// -> calculate PWM (voltage) depending on set speed
    left_ff  = MotionController_LeftFeedForward(left_speed_ref_mms);
    right_ff = MotionController_RightFeedForward(right_speed_ref_mms);
	// -> adding to feedback values 
	
    left_output  += left_ff;
    right_output += right_ff;
#endif

    left_output  = clampf(left_output,  -MAX_PWM, MAX_PWM);
    right_output = clampf(right_output, -MAX_PWM, MAX_PWM);

    motion_ctrl.left_output_pwm = left_output;
    motion_ctrl.right_output_pwm = right_output;

    if (motion_ctrl.controller_output_enabled)
    {
        Motor_Left_SetPWM((int16_t)left_output);
        Motor_Right_SetPWM((int16_t)right_output);
    }
}
float MotionController_GetLeftPWM(void)
{
    return motion_ctrl.left_output_pwm;
}

float MotionController_GetRightPWM(void)
{
    return motion_ctrl.right_output_pwm;
}

float MotionController_GetForwardError(void)
{
    return motion_ctrl.fwd_error;
}

float MotionController_GetRotationError(void)
{
    return motion_ctrl.rot_error;
}

void MotionController_Test_Standing(void){
	uint32_t period_ms = (uint32_t)(LOOP_INTERVAL * 1000.0f);

    Timer_Initialize(TIMER_ID_TIM2,  period_ms, cb_update_test_sensor);
    Timer_Start(TIMER_ID_TIM2);

    MotionController_Init();
    MotionController_Reset();
	
	while(1){
//		printf("LM: %u  RM: %u  err: %.2f  steer: %.2f  LM_Hz: %lu  RM_Hz: %lu\n",
//       (unsigned)s_debug_tof_lm_mm,
//       (unsigned)s_debug_tof_rm_mm,
//       s_debug_wall_error,
//       s_debug_steering_dps,
//       (unsigned long)s_debug_lm_hz,
//       (unsigned long)s_debug_rm_hz);
//	HAL_Delay(100);
	}

}


// motion control
bool move(float n_fields, float top_speed, float acceleration, float final_speed)
{
		
    float goal_distance_mm = n_fields * DISTANCE_FIELD;
		uint32_t period_ms = (uint32_t)(LOOP_INTERVAL * 1000.0f);
		//Timer_Initialize(TIMER_ID_TIM2,  period_ms, cb_update_forward);
		printf("period_ms = %lu\n", period_ms);
    encoder_reset();
    Profile_Reset(&forward_profile);
    Profile_Reset(&rotation_profile);
    MotionController_Reset();
		
    Profile_Init(&forward_profile);

    Profile_Start(&forward_profile,
                  goal_distance_mm,
                  top_speed,
                  final_speed,
                  acceleration);
		printf("in move before while \n");
    Timer_Start(TIMER_ID_TIM2);

	float time_array[SAMPLE_POINTS];
	float speed_array[SAMPLE_POINTS];
	uint16_t i = 0;	
	float forward_speed = 0;
	
    while (!Profile_IsFinished(&forward_profile))
    {
			float right_speed = encoder_get_right_speed_mms();
			float left_speed = encoder_get_left_speed_mms();
			forward_speed = (right_speed + left_speed) / 2;

			time_array[i] = i * SAMPLE_TIME_MS;
			speed_array[i] = forward_speed;
			++i;
			 printf("ISR calls: %lu, state: %d, pos: %.2f\n",
           isr_counter,
           forward_profile.state,
           forward_profile.position);
			HAL_Delay(20);
    }

    Timer_Stop(TIMER_ID_TIM2);
		printf("in move after while \n");

    if (final_speed == 0.0f){
        motion_stop_all();
    }else{
        MotionController_SetCommand(final_speed, 0.0f);
    }
		uint8_t sample_buf[8];
		printf("in move before sending \n");
		
		// 1. Send TIME Array
    WriteProtocol(cmd_MoveEncoder, (uint8_t*)"TIME", 4);
    Delay_ms(10);
    for(uint16_t i = 0; i < SAMPLE_POINTS; i++){
        WriteProtocol(cmd_MoveEncoder, (uint8_t*)&time_array[i], 4); // Send exactly 4 bytes
        Delay_ms(5);
    }
    
    // 2. Send ACTUAL Velocity Array
    WriteProtocol(cmd_MoveEncoder, (uint8_t*)"ACTUAL", 6);
    Delay_ms(10);
    for(uint16_t i = 0; i < SAMPLE_POINTS; i++){
        WriteProtocol(cmd_MoveEncoder, (uint8_t*)&speed_array[i], 4);
        Delay_ms(5);
    }

    // 3. Send TARGET Velocity Array (Your C# app REQUIRES this to trigger the save!)
    WriteProtocol(cmd_MoveEncoder, (uint8_t*)"TARGET", 6);
    Delay_ms(10);
    for(uint16_t i = 0; i < SAMPLE_POINTS; i++){
        float target_speed = 0.0f; // Replace with actual target speed if you track it
        WriteProtocol(cmd_MoveEncoder, (uint8_t*)&target_speed, 4);
        Delay_ms(5);
    }
		
		printf("in move after sending \n");
    return true;
}

void turn(float n_quarter_turns)
{
    float top_speed_dps = 400.0f;
    float final_speed_dps = 0.0f;
    float acceleration_dps2 = 4000.0f;

    float goal_angle_deg = n_quarter_turns * ROTATION_DEG * -1;
    uint8_t settled_count = 0;

    encoder_reset();
    Profile_Reset(&rotation_profile);
    Profile_Reset(&forward_profile);
    MotionController_Reset();

    Profile_Init(&rotation_profile);

    Profile_Start(&rotation_profile,
                  goal_angle_deg,
                  fabsf(top_speed_dps),
                  final_speed_dps,
                  fabsf(acceleration_dps2));

    if (goal_angle_deg < 0.0f)
    {
        Profile_SetSpeed(&rotation_profile, -fabsf(top_speed_dps));
    }
    else
    {
        Profile_SetSpeed(&rotation_profile, +fabsf(top_speed_dps));
    }

    Timer_Start(TIMER_ID_TIM9);

    // 1. Warten bis Profil fertig
		printf("Profile_IsFinished\n");
    while (!Profile_IsFinished(&rotation_profile))
    {
    }
		printf("leave Profile_IsFinished\n");

    // 2. Danach Sollwert 0 setzen, aber Regler weiterlaufen lassen
    MotionController_SetCommand(0.0f, 0.0f);

    // 3. Warten bis die Maus wirklich steht
		printf("settled_count\n");
    while (settled_count < 50)
    {
        float dphi = fabsf(encoder_get_rotation_change_deg());

        if (dphi < 0.3f) {
            settled_count++;
        } else {
            settled_count = 0;
        }
    }
		printf("leave settled_count\n");

    Timer_Stop(TIMER_ID_TIM9);
    motion_stop_all();
}


// experimental move functions for continious driving
void Motion_Start(void)
{
    encoder_reset();
    Profile_Reset(&forward_profile);
    Profile_Reset(&rotation_profile);
    MotionController_Reset();
    Timer_Start(TIMER_ID_TIM12);
}

void Motion_Stop(void)
{
    Timer_Stop(TIMER_ID_TIM12);
    motion_stop_all();
}


void Motion_CommandForward(float distance_mm, float top_speed, float accel, float final_speed, bool sensorEnable)
{
    float current_speed = Profile_GetSpeed(&forward_profile);
	
	WALL_STEER_ENABLE = sensorEnable;
	
	/* Rotation explizit beenden */
    Profile_Reset(&rotation_profile);
    Profile_SetSpeed(&rotation_profile, 0.0f);
    Profile_SetTargetSpeed(&rotation_profile, 0.0f);

    Profile_Start(&forward_profile, distance_mm, top_speed, final_speed, accel);
    Profile_SetSpeed(&forward_profile, current_speed);

    while (!Profile_IsFinished(&forward_profile))
    {}
}


void Motion_CommandTurn(float angle_deg, float top_omega, float alpha, float final_omega)
{
	
	WALL_STEER_ENABLE = false;
	
    Profile_Start(&rotation_profile, angle_deg, fabsf(top_omega), final_omega, fabsf(alpha));

    if (angle_deg < 0.0f)
        Profile_SetSpeed(&rotation_profile, -fabsf(top_omega));
    else
        Profile_SetSpeed(&rotation_profile, +fabsf(top_omega));
	
	
	while (!Profile_IsFinished(&rotation_profile))
    {}
}

bool Motion_CommandCurve(float arc_length_mm,
                         float radius_mm,
                         float top_speed_mms,
                         float accel_mms2,
                         float final_speed_mms)
{
    float arc_angle_deg;
    float top_omega_dps;
    float final_omega_dps;
    float alpha_dps2;
    float dir_sign;
    float current_v;
    float current_omega;
	
		WALL_STEER_ENABLE = false;

    if (fabsf(radius_mm) < 1e-3f) return false;
    if (fabsf(arc_length_mm) < 1e-3f) return false;

    dir_sign = (radius_mm >= 0.0f) ? 1.0f : -1.0f;

    arc_angle_deg   = (arc_length_mm / fabsf(radius_mm)) * RAD_TO_DEG * dir_sign;
    top_omega_dps   = (top_speed_mms   / fabsf(radius_mm)) * RAD_TO_DEG * dir_sign;
    final_omega_dps = (final_speed_mms / fabsf(radius_mm)) * RAD_TO_DEG * dir_sign;
    alpha_dps2      = (accel_mms2      / fabsf(radius_mm)) * RAD_TO_DEG;

    current_v = Profile_GetSpeed(&forward_profile);
    current_omega = Profile_GetSpeed(&rotation_profile);

    Profile_Start(&forward_profile,
                  arc_length_mm,
                  top_speed_mms,
                  final_speed_mms,
                  accel_mms2);

    Profile_Start(&rotation_profile,
                  arc_angle_deg,
                  fabsf(top_omega_dps),
                  fabsf(final_omega_dps),
                  alpha_dps2);

    Profile_SetSpeed(&forward_profile, current_v);
    Profile_SetSpeed(&rotation_profile, current_omega);

    Profile_SetTargetSpeed(&forward_profile,
                           (arc_length_mm >= 0.0f) ? fabsf(top_speed_mms) : -fabsf(top_speed_mms));

    Profile_SetTargetSpeed(&rotation_profile, top_omega_dps);

    while (!Profile_IsFinished(&forward_profile) ||
           !Profile_IsFinished(&rotation_profile))
    {
    }

    return true;
}

bool Motion_CommandCurveAngle(float arc_angle_deg,
                              float radius_mm,
                              float top_speed_mms,
                              float accel_mms2,
                              float final_speed_mms)
{
    float arc_length_mm;

    if (fabsf(radius_mm) < 1e-3f) return false;

    arc_length_mm = fabsf(radius_mm) * fabsf(arc_angle_deg) / RAD_TO_DEG;

    if (arc_angle_deg < 0.0f)
        radius_mm = -fabsf(radius_mm);
    else
        radius_mm = +fabsf(radius_mm);

    return Motion_CommandCurve(arc_length_mm,
                               radius_mm,
                               top_speed_mms,
                               accel_mms2,
                               final_speed_mms);
}
<<<<<<< HEAD:02_Software/01_Firmware/DebuggerTesting_Micromouse_Hardware_FW_V2.0/motion_control.c
=======


//Funktioniert nicht Encoder sind zu ungenau

//static volatile float angleCorrection = 0;
////Aktueller Winkel wird berücksichtigt
//void Motion_CommandForwardAdaption(float distance_mm, float top_speed, float accel, float final_speed, bool sensorEnable, float* currentAngle)
//{
//    float current_speed = Profile_GetSpeed(&forward_profile);
//	
//	WALL_STEER_ENABLE = sensorEnable;
//	
//	/* Rotation explizit beenden */
//    Profile_Reset(&rotation_profile);
//    Profile_SetSpeed(&rotation_profile, 0.0f);
//    Profile_SetTargetSpeed(&rotation_profile, 0.0f);

//    Profile_Start(&forward_profile, distance_mm, top_speed, final_speed, accel);
//    Profile_SetSpeed(&forward_profile, current_speed);

//    while (!Profile_IsFinished(&forward_profile))
//    {}
//		
//		*currentAngle = encoder_get_angle_rad();
//		printf("Forward *currentAngle*360/(2*M_PI) : %f\n",*currentAngle*360/(2*M_PI) );
//		*currentAngle = *currentAngle*360/(2*M_PI) - angleCorrection;
//		if(*currentAngle > 5 || *currentAngle < -5){
//				*currentAngle = 0;
//		}			
//}

////Der aktuelle Winkel wird mitgetrackt
//bool Motion_CommandCurveAngleAdaption(float arc_angle_deg,
//                              float radius_mm,
//                              float top_speed_mms,
//                              float accel_mms2,
//                              float final_speed_mms,
//															float* currentAngle)
//{
//	
//		angleCorrection += arc_angle_deg;
//		arc_angle_deg = arc_angle_deg - *currentAngle;
//	
//    float arc_length_mm;

//    if (fabsf(radius_mm) < 1e-3f) return false;

//    arc_length_mm = fabsf(radius_mm) * fabsf(arc_angle_deg) / RAD_TO_DEG;

//    if (arc_angle_deg < 0.0f)
//        radius_mm = -fabsf(radius_mm);
//    else
//        radius_mm = +fabsf(radius_mm);

//    bool result = Motion_CommandCurve(arc_length_mm,
//                               radius_mm,
//                               top_speed_mms,
//                               accel_mms2,
//                               final_speed_mms);
//		
//		*currentAngle = encoder_get_angle_rad();
//		printf("*currentAngle*360/(2*M_PI) : %f\n",*currentAngle*360/(2*M_PI) );
//		printf("angleCorrection: %f\n",angleCorrection);
//		*currentAngle = *currentAngle*360/(2*M_PI) - angleCorrection;
//		
//		return result;
//		
//}
>>>>>>> origin/development:02_Software/01_Firmware/Micromouse_Hardware_FW_V2.0/motion_control.c
