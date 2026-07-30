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
#include "wall_sensors_ir.h"
#include "wall_following.h"
#include "BLEDebugger.h"

#define RAD_TO_DEG 57.29578f

MotionController_t motion_ctrl;
Profile forward_profile;
Profile rotation_profile;

/* ----------------------------------------------------------
 * Wall Following
 * ---------------------------------------------------------- */

static bool WALL_STEER_ENABLE = true;

static void MotionController_SensorInit(void)
{
    WallSensors_IR_Init();
    WallFollowing_Init();
}

static void MotionController_UpdateSensors(void)
{
    WallSensors_IR_Update();
}

static float MotionController_CalcWallSteeringDps(void)
{
    if (!WALL_STEER_ENABLE) {
        return 0.0f;
    }

    return WallFollowing_GetCorrectionDps(motion_ctrl.velocity_ref_mms);
}

/* ----------------------------------------------------------
 * Callback Timer Functions
 * ---------------------------------------------------------- */

static float forward_circle = 0;//141 * 5;
static float omega_circle = 0;//90 * 5;
static float dbg_steering = 0;
	
static void cb_update_test_sensor(int id)
{
    float steering_adjustment_dps = 0.0f;

    (void)id;

    encoder_update();

    //MotionController_UpdateSensors();
	
	// for debugging
	steering_adjustment_dps = MotionController_CalcWallSteeringDps();
	dbg_steering = steering_adjustment_dps;

    MotionController_SetCommand(
        forward_circle,   /* mm/s */
        omega_circle                                  /* deg/s */
    );

    MotionController_Update(steering_adjustment_dps);
}


static void cb_update_forward(int id)
{
    float steering_adjustment_dps = 0.0f;

    (void)id;

    encoder_update();
    Profile_Update(&forward_profile);

    //MotionController_UpdateSensors();
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

    //MotionController_UpdateSensors();
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
	MotionController_SensorInit();
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
#if TARGET_HW == HW_VERSION_1
    left_output  = pos_output - rot_output;
    right_output = pos_output + rot_output;
#endif

#if TARGET_HW == HW_VERSION_2
	left_output  = pos_output + rot_output;
    right_output = pos_output - rot_output;
#endif

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
		WallSensors_IR_Update();
		//printf("steering: %f\n", dbg_steering);
	//		printf("LM: %u  RM: %u  err: %.2f  steer: %.2f  LM_Hz: %lu  RM_Hz: %lu\n",
	//       (unsigned)s_debug_tof_lm_mm,
	//       (unsigned)s_debug_tof_rm_mm,
	//       s_debug_wall_error,
	//       s_debug_steering_dps,
	//       (unsigned long)s_debug_lm_hz,
	//       (unsigned long)s_debug_rm_hz);
			//HAL_Delay(100);
	}

}

static uint32_t s_last_service45_ms = 0U;

// motion control
bool move(float n_fields, float top_speed, float acceleration, float final_speed)
{
    float goal_distance_mm = n_fields * DISTANCE_FIELD;

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

    Timer_Start(TIMER_ID_TIM2);

    while (!Profile_IsFinished(&forward_profile))
    {
//		uint32_t now = HAL_GetTick();

//		if ((now - s_last_service45_ms) < 5) {
//			continue;
//		} else {
//			WallSensors_Update();
//		}

//		s_last_service45_ms = now;
		
		WallSensors_IR_Update();
		
       
//		if (check_front_wall()) {
//			motion_stop_all();
//			Timer_Stop(TIMER_ID_TIM2);
//			return false;
//		}
//		HAL_Delay(20);
        
    }

    Timer_Stop(TIMER_ID_TIM2);

    if (final_speed == 0.0f)
    {
        motion_stop_all();
    }
    else
    {
        MotionController_SetCommand(final_speed, 0.0f);
    }

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
    while (!Profile_IsFinished(&rotation_profile))
    {
    }

    // 2. Danach Sollwert 0 setzen, aber Regler weiterlaufen lassen
    MotionController_SetCommand(0.0f, 0.0f);

    // 3. Warten bis die Maus wirklich steht
    while (settled_count < 50)
    {
        float dphi = fabsf(encoder_get_rotation_change_deg());

        if (dphi < 0.3f) {
            settled_count++;
        } else {
            settled_count = 0;
        }
    }

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


void Motion_CommandForward(float distance_mm, float top_speed, float accel, float final_speed, bool steerEnable)
{
  float current_speed = Profile_GetSpeed(&forward_profile);
	
	WALL_STEER_ENABLE = steerEnable;
	
	/* Rotation explizit beenden */
    Profile_Reset(&rotation_profile);
    Profile_SetSpeed(&rotation_profile, 0.0f);
    Profile_SetTargetSpeed(&rotation_profile, 0.0f);

    Profile_Start(&forward_profile, distance_mm, top_speed, final_speed, accel);
    Profile_SetSpeed(&forward_profile, current_speed);

    while (!Profile_IsFinished(&forward_profile))
    {WallSensors_IR_Update();}
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