/*************************************************************
Project		: Micromouse V1.2
File		: motion_control.c
Description	: Functionality to make controlled motion on the mouse
Date		: 23.12.2024
*************************************************************/

#include "motion_control.h"
#include <stdio.h>
#include "encoder.h"
#include "MotorDriver.h"
#include "Delay.h"
#include "Python_Debugger.h"
#include "board_button.h"
#include "adc.h"
#include "sensor.h"
#include "profile.h"
#include "sensor_normalize.h"
#include "Timer.h"
#include "config.h"
#include <math.h>
#include "BOARD_LED.h"
#include <stdlib.h>

/*******************************************************************************
* DEFINES
*******************************************************************************/

	
typedef struct {
    double KP, KD;
    double old_error;   // old_error
    double old_pwm;   // old_PWM
} PDController;

static inline void PD_Init(PDController *c, double KP, double KD){
    c->KP = KP;
    c->KD = KD;
    c->old_error = 0.0;
    c->old_pwm = 0.0;
}

static inline void PD_Reset(PDController *c){
    c->old_error = 0.0;
    c->old_pwm = 0.0;
}



/*******************************************************************************
* Profiles -> need to be updated and reseted after drive cycle
*******************************************************************************/
Profile forward_profile;
Profile rotation_profile;
static PDController ctrl_pos;
static PDController ctrl_angle;
static double curveFactorRight = 1.00f;
static double curveFactorLeft = 1.00f;


/*******************************************************************************
* callback functions for timer -> updates for controllers
*******************************************************************************/
static void cb_update_forward(int id){
	Profile_Update(&forward_profile);
	update_motor_controllers(0, Profile_GetPosition(&forward_profile), SENSOR_ENABLE);
}

static void cb_update_rotation(int id){
	Profile_Update(&rotation_profile);
	update_motor_controllers(Profile_GetPosition(&rotation_profile), 0, false);
}


static void cb_update_combined(int id){
    // Beide Profile synchron updaten
    Profile_Update(&forward_profile);
    Profile_Update(&rotation_profile);

    double setPos  = Profile_GetPosition(&forward_profile);
		static double setAng = 0; 
		//if(DRIVE_CURVE == true){
			setAng  = Profile_GetPosition(&rotation_profile);
		//}

    update_motor_controllers(setAng, setPos, SENSOR_ENABLE);
}

void SetCurveFactorLeft(double val){
	curveFactorLeft = val;
}

void SetCurveFactorRight(double val){
	curveFactorRight = val;
}


static inline float angle_deg_to_dist_mm(float angle_deg)
{
    const float WHEEL_BASE = 50;  
    float angle_rad = angle_deg * (float)M_PI / 180.0f;
    return angle_rad * (WHEEL_BASE / 2.0f);
}




// !!! Abhaengig vom Takt !!!
// set timer
void Motion_Init(){
	
	// [find LOOP_INTERVAL in Profile.h]
	uint32_t period_ms = (uint32_t)(LOOP_INTERVAL*1000);
	Timer_Initialize(TIMER_ID_TIM2, period_ms, cb_update_forward);
	Timer_Initialize(TIMER_ID_TIM9, period_ms, cb_update_rotation);
	Timer_Initialize(TIMER_ID_TIM12, period_ms, cb_update_combined);
	
	PD_Init(&ctrl_pos, KP_pos, KD_pos);
	PD_Init(&ctrl_angle, KP_ang, KD_ang);
	

	Timer_Stop(TIMER_ID_TIM2);
	Timer_Stop(TIMER_ID_TIM9);
	Timer_Stop(TIMER_ID_TIM12);

}

/*******************************************************************************
* position control
*******************************************************************************
* output -> desired PWM value
*******************************************************************************/
static double position_controller(double set_position){
	double error = 0;
	double pwm = 0;
	static double diff = 0;
	
    // Get current position from encoders
    double position_L = Get_Encoder_Position_mm_Left();
    double position_R = Get_Encoder_Position_mm_Right();
	double position = (position_L + position_R) / 2;
	
	// Filter D -> encoder is noisy
	// [0.6 -> more KD]
	// [0.9 -> less KD]
	double const alpha = 0.85;
	
	// 1) calc error
    error = set_position - position;
	
	// 2) calculate
	// Adjust the PWM values based on the position correction
	double de     = (error - ctrl_pos.old_error) / LOOP_INTERVAL;
	diff = alpha*diff + (1.0 - alpha)*de;
    pwm = KP_pos * error + KD_pos * diff;
	
//	diff = (error - ctrl_pos.old_error);
//    pwm = KP_pos * error + KD_pos * diff;
	
	// 3) update old error
	ctrl_pos.old_error = error;
	ctrl_pos.old_pwm = pwm;
	
	return pwm;
}


/*******************************************************************************
* limit_correction_ir
*******************************************************************************/
static double limit_correction_ir(double correction_ir, double min_limit, double max_limit) {
    if (correction_ir > max_limit) {
        return max_limit;
    } else if (correction_ir < min_limit) {
        return min_limit;
    }
    return correction_ir;
}


/*******************************************************************************
* IR-Sensor error
*******************************************************************************
* calculate angle error by IR-Sensors
*******************************************************************************/

double ir_angle_error(void){
	
	// normalize steering sensors
    double sensor_L = readSensor_Left_45();
    double sensor_R = readSensor_Right_45();

    double L = IR_from_ADC(sensor_L, S_ref_left_45);
    double R = IR_from_ADC(sensor_R, S_ref_right_45);

	// ln normalizes IR values to 100 then mouse is in middle position
    double leftError  = 100 - L;
    double rightError = 100 - R;

    // keep offset low
    bool leftWall  = sensor_L < (S_ref_left_45  + 25);
    bool rightWall = sensor_R < (S_ref_right_45 + 25);

    double error = 0;
	
	// both walls
    if (leftWall && rightWall) {
        error = leftError - rightError;
    }
	// only left wall
    else if (leftWall) {
        error = +2.0 * leftError;
    }
	// only right wall
    else if (rightWall) {
        error = -2.0 * rightError;
    }
    else {
        error = 0;
    }

    return error;
}

/*******************************************************************************
* Limit angle correction depending on velocity (experimental state)
*******************************************************************************/
static double ir_speed_gain(void)
{
    // get current velocity
    double v = fabs(Profile_GetSpeed(&forward_profile));

    // velocity range
	const double V_FULL = 300.0;   	// full KP_ir
    const double V_MAX  = 2000.0;  	// minimum KP_ir
    const double G_FULL = 1.0;     	// maximum gain
    const double G_MIN  = 0.3;  	// minimum gain

    if (v <= V_FULL) {
        return G_FULL; 
    }
    if (v >= V_MAX) {
        return G_MIN;  
    }

    // linear between 
    double t = (v - V_FULL) / (V_MAX - V_FULL);  // 0..1
    double gain = G_FULL - (G_FULL - G_MIN) * t;

    return gain;
}


double calculate_steering_adjustment(double error){
	double angle_adjustment = 0;
	static double old_error = 0;
	
	 double speed_gain = 1; // ir_speed_gain(); 
	
	double pTerm = (KP_ir * speed_gain) * error;
    double dTerm = (KD_ir * speed_gain) * (error - old_error);
	angle_adjustment = pTerm + dTerm;
	// limit the output!! -> maybe need a correction
	angle_adjustment = limit_correction_ir(angle_adjustment, -ADJUSTMENT_LIMIT, ADJUSTMENT_LIMIT);
	old_error = error;
	
	return angle_adjustment;
}

/*******************************************************************************
* angle control
*******************************************************************************
* output -> desired PWM value
*******************************************************************************/
static double angle_controller(double set_angle, bool sensor_on){
	double error_encoder = 0;
	static double diff = 0;
	//double diff = 0;
	double pwm = 0;
	
	
	// get angle error by IR-Sensors
	double error_ir = 0;
	// calculate angle adjustment
	double adjustment = 0;

    // Get current position from encoders
    double position_L = Get_Encoder_Position_mm_Left_reset_Turn();
    double position_R = Get_Encoder_Position_mm_Right_reset_Turn();
	double angle = (position_L - position_R);
	
	
	// Filter D -> encoder is noisy
	// [0.6 -> more KD]
	// [0.9 -> less KD]
	double const alpha = 0.7;

	// 1) calculate angle error by encoders
	error_encoder = set_angle - angle;
	
	
	// add ir-adjustment when activated
	if (sensor_on){
		error_ir = ir_angle_error();
		adjustment = calculate_steering_adjustment(error_ir);
		
		error_encoder += adjustment;
	}
	
	// 2) calculate
	// Adjust the PWM values based on the position correction
	double de     = (error_encoder - ctrl_angle.old_error) / LOOP_INTERVAL;
	diff = alpha*diff + (1.0 - alpha)*de;
	pwm   = KP_ang*error_encoder + KD_ang*diff;
	
//	diff  = (error_encoder - ctrl_angle.old_error);
//	pwm   = KP_ang*error_encoder + KD_ang*diff;


	// 3) update old values
	ctrl_angle.old_error = error_encoder;
	ctrl_angle.old_pwm = pwm;
	
	
	return pwm;
}


/*******************************************************************************
* update motor controller
*******************************************************************************
* input: Angle & Velocity
*******************************************************************************/
void update_motor_controllers(double setAngle, double setPosition, bool sensor_on){
	double leftMotor = 0;
	double rightMotor = 0;
	
	// position control
	double forwardSpeed = position_controller(setPosition);
	
	// angle  control
	double angleCorrection = 0;
	if (STEERING_ENABLE){
		//angleCorrection = angle_controller(setAngle, sensor_on);
	}
	
	// angle control....
	leftMotor = forwardSpeed + angleCorrection;
	rightMotor = forwardSpeed - angleCorrection;
	
	//Test Faktor 2,78 für 90° Kurve
	double const straightFactor = 0.98f;
	rightMotor = (rightMotor*straightFactor)/curveFactorRight;
	leftMotor = (leftMotor)/curveFactorLeft;
	
	Motor_Left_SetPWM(leftMotor);
	Motor_Right_SetPWM(rightMotor);
}



/*******************************************************************************
* move
*******************************************************************************
* input: distance (fields)
* return: false if wall is in front
*******************************************************************************/
bool move(float n, float top_speed, float acceleration, float final_speed){

	float goalDistance = n * DISTANCE_FIELD;
	int i = 0;

	
	Encoder_Reset();
	Profile_Reset(&forward_profile);
	PD_Reset(&ctrl_pos);
	PD_Reset(&ctrl_angle);

	Profile_Init(&forward_profile);
	
	// start drive
	Profile_Start(&forward_profile, goalDistance, top_speed, final_speed, acceleration);
	Timer_Start(TIMER_ID_TIM2);
	
	while (!Profile_IsFinished(&forward_profile)) {

//		if(check_front_wall()){
//			// slow down before wall
//			Profile_SetSpeed(&forward_profile,100);
//			// stop before wall
//			if(check_front_middle()){
//				Motor_Left_SetPWM(0);
//				Motor_Right_SetPWM(0);
//				PD_Reset(&ctrl_pos);
//				PD_Reset(&ctrl_angle);
//				Timer_Stop(TIMER_ID_TIM2);
//				return false;
//			}

//		}
	}
	
	if (top_speed != final_speed){
		Motor_Left_SetPWM(0);
		Motor_Right_SetPWM(0);
	}
	
	// Stop drive
	Timer_Stop(TIMER_ID_TIM2);
	
	
	return true;
}

/*******************************************************************************
* move with dynamic curves
*******************************************************************************
* input: n * distance
* curve_at -> distance when curve starts
* curve_type -> 0..right, 1..left 
* return: false if wall is in front
*******************************************************************************/
bool move_with_curves(uint8_t n, float top_speed, float acceleration, uint8_t curve_at, uint8_t curve_type){
	
	float goalDistance = n * DISTANCE_FIELD;
	float final_speed = 0.0f;
	int i = 0;
	
	Encoder_Reset();
	Profile_Reset(&forward_profile);
	Profile_Reset(&rotation_profile);
	PD_Reset(&ctrl_pos);
	PD_Reset(&ctrl_angle);
	Profile_Init(&forward_profile);
    Profile_Init(&rotation_profile);
	
	// with sensors until curve beginns
	SENSOR_ENABLE = true;
	
	// start drive
	Profile_Start(&forward_profile, goalDistance, top_speed, final_speed, acceleration);
	Timer_Start(TIMER_ID_TIM2);
	
	while (!Profile_IsFinished(&forward_profile)) {

		if(check_front_wall()){
			Motor_Left_SetPWM(0);
			Motor_Right_SetPWM(0);
			PD_Reset(&ctrl_pos);
			PD_Reset(&ctrl_angle);
			Timer_Stop(TIMER_ID_TIM2);
			SENSOR_ENABLE = true;
			return false;
		}
	}
	
	if (top_speed != final_speed){
		Motor_Left_SetPWM(0);
		Motor_Right_SetPWM(0);
	}
	
	// Stop drive
	Timer_Stop(TIMER_ID_TIM2);
	SENSOR_ENABLE = true;
	return true;
}


void turn(float n)
{
	float top_speed = 200;//500.0f;       
    float final_speed = 0.0f;       
    float acceleration = 3500.0f;
	float goalDistance = (1 * n * DISTANCE_90DEG);

	Encoder_Reset();
	Profile_Reset(&rotation_profile);
	PD_Reset(&ctrl_pos);
	PD_Reset(&ctrl_angle);
	Profile_Init(&rotation_profile);
	
	// start rotation
	Profile_Start(&rotation_profile, goalDistance, top_speed, final_speed, acceleration);
	Timer_Start(TIMER_ID_TIM9);
	
	double position_L = Get_Encoder_Position_mm_Left();
    double position_R = Get_Encoder_Position_mm_Right();
	double angle = (position_L - position_R);
	
	while (!Profile_IsFinished(&rotation_profile)) {}
	
	// end rotation
	Timer_Stop(TIMER_ID_TIM9);
	Motor_Left_SetPWM(0);
	Motor_Right_SetPWM(0);
}

bool straight_then_smooth_turn(float straight_mm,
                               float cruise_speed,      // mm/s (final_speed der Geraden)
                               float straight_acc,      // mm/s^2
                               float turn_angle_deg,    // +90 / -90 / etc.
                               float turn_radius_mm,    // Bogenradius (Roboter-Mittelpunkt)
                               float turn_acc){         // mm/s^2 (f�r die Bogenbewegung)


  // 1) NICHT resetten, wenn du aus einem vorherigen Segment kommst!
  //    Hier: wir starten frisch -> Encoder/Controller einmalig resetten
  Encoder_Reset();
  PD_Reset(&ctrl_pos);
  PD_Reset(&ctrl_angle);
  Profile_Init(&forward_profile);
  Profile_Init(&rotation_profile);

  // 2) Vortrieb starten, final_speed = cruise_speed (damit er NICHT stehen bleibt)
  Profile_Start(&forward_profile, straight_mm, cruise_speed, cruise_speed, straight_acc);

  // 3) Timer los
  Timer_Start(TIMER_ID_TIM12);

  // 4) Warten bis kurz vor Segmentende � hier z�nden wir den Turn
  //    Du kannst exakt am Ende starten; �fr�hestens� ist nicht n�tig,
  //    Hauptsache: NICHT stoppen/resetten und Timer weiterlaufen lassen.
  while (!Profile_IsFinished(&forward_profile)) {
    // Optional: Sensor-Checks �
  }

  // 5) Turn synchron mit der aktuellen Fahrt
  const float arc_len   = fabsf(turn_angle_deg) * (float)M_PI / 180.0f * fabsf(turn_radius_mm); // mm
  const float angle_dist = angle_deg_to_dist_mm(turn_angle_deg);

  // Skaliere Turn-Geschwindigkeit zur Bogenl�nge, damit SetPos/SetAng "gleichzeitig" vorankommen:
  // k = angle_dist / arc_len  -> Rotations-"mm" pro "mm" Vortrieb
  const float k = (arc_len > 1e-3f) ? (angle_dist / arc_len) : 0.0f;

  // Sofortiger nahtloser �bergang: Rotations-Startgeschwindigkeit auf aktuellen Vortriebs-speed mappen
  // Vortriebs-Sollgeschwindigkeit ist gerade cruise_speed
  const float turn_speed = k * cruise_speed;   // "mm-diff"/s
  const float turn_accel = k * turn_acc;       // skaliert

  // Setze die aktuelle Rotg.-Startspeed, damit keine Rampe von 0 st�rt:
  Profile_SetSpeed(&rotation_profile, turn_speed);

  // Turn starten; final_speed am Ende des Bogens = 0 (oder z.B. wieder cruise_speed, wenn du in weiteren Bogen gehst)
  Profile_Start(&rotation_profile, angle_dist, fabsf(turn_speed), 0.0f, fabsf(turn_accel));

  // 6) W�hrend des Turns l�uft forward mit cruise_speed einfach weiter (kein Reset, kein Stop).
  //    F�r einen definierten �nur Bogen�-Abschnitt kannst du zus�tzlich die Vortriebs-Strecke
  //    begrenzen: setze eine kleine Nachlaufstrecke oder nutze ein eigenes Flag.
  //    Einfacher: warte, bis Rotation fertig ist und stoppe dann sanft Vortrieb.
  while (!Profile_IsFinished(&rotation_profile)) {
    // Optional: weitere Logik/Abbruch
  }

  // 7) Jetzt kannst du entscheiden:
  //    a) Nach dem Turn weiterfahren: forward bleibt mit cruise_speed aktiv
  //    b) Oder Vortrieb gezielt auf 0 abbremsen:
  Profile_Start(&forward_profile, 0.0f, cruise_speed, 0.0f, straight_acc);
  while (!Profile_IsFinished(&forward_profile)) {}

  // 8) Aufr�umen
  Timer_Stop(TIMER_ID_TIM12);
  Motor_Left_SetPWM(0);
  Motor_Right_SetPWM(0);
  return true;
}
															 
bool drive_half_circle(float cruise_speed, float acc)
{
		
		float driveBeforeCurve = 120.0f;
		
    // 1) Reset und Initialisierung
    Encoder_Reset();
    PD_Reset(&ctrl_pos);
    PD_Reset(&ctrl_angle);
    Profile_Init(&forward_profile);
    Profile_Init(&rotation_profile);

    Timer_Start(TIMER_ID_TIM12);

    // 2) Parameter
    const float radius_mm   = 70.0f;
    const float nominal_angle = 110.0f;  // Zielwinkel
    const float correction_factor = 180.0f / 220.0f;  // empirische Korrektur gegen �berdrehung
    const float turn_angle  = nominal_angle * correction_factor;  // korrigiert

    const float arc_len     = (float)M_PI * radius_mm; // theoretische Bogenl�nge = p * r
    const float angle_dist  = angle_deg_to_dist_mm(turn_angle);

    const float k           = angle_dist / arc_len;
    const float turn_speed  = k * cruise_speed;
    const float turn_accel  = k * acc;

    // 3) Sanftes Anfahren: 50 mm Rampe, um gleichm��ige Geschwindigkeit aufzubauen
    const float pre_straight = driveBeforeCurve;  // Anlaufstrecke
    Profile_Start(&forward_profile, pre_straight+arc_len, cruise_speed, cruise_speed, acc);
		while (Profile_GetPosition(&forward_profile) < 100.0f) {
        // warten bis konstante Geschwindigkeit erreicht
    }
		
//    while (!Profile_IsFinished(&forward_profile)) {
//        // warten bis konstante Geschwindigkeit erreicht
//    }

    // 4) Jetzt gleichzeitigen Halbkreis starten (bei stabiler Vorw�rtsfahrt)
    //Profile_Start(&forward_profile, arc_len, cruise_speed, cruise_speed, acc);
    Profile_SetSpeed(&rotation_profile, turn_speed);
    Profile_Start(&rotation_profile, angle_dist, fabsf(turn_speed), fabsf(turn_speed), fabsf(turn_accel));

    while (!Profile_IsFinished(&forward_profile) || !Profile_IsFinished(&rotation_profile)) {
        // hier kann z. B. Motorupdate oder Telemetrie laufen
    }

    // 5) Sanft abbremsen
    Profile_Start(&forward_profile, 0.0f, cruise_speed, 0.0f, acc);
    while (!Profile_IsFinished(&forward_profile)) {}

    Timer_Stop(TIMER_ID_TIM12);
    Motor_Left_SetPWM(0);
    Motor_Right_SetPWM(0);

    return true;
}

bool drive_curve(float cruise_speed, float acc, float target_angle_deg, float drive_before_curve)
{
    // --- Initialisierung ---
    Encoder_Reset();
    PD_Reset(&ctrl_pos);
    PD_Reset(&ctrl_angle);
    Profile_Init(&forward_profile);
    Profile_Init(&rotation_profile);
    Timer_Start(TIMER_ID_TIM12);
	
		//Corrections
		target_angle_deg = target_angle_deg  *0.90f;
		float const turnEarlier = 20.0f;

			// --- Parameter ---
		const float radius_mm   = 90.0f;
		float arc_len    = (float)M_PI * radius_mm * (target_angle_deg / 180.0f); // Bogenl�nge
		float angle_dist = angle_deg_to_dist_mm(target_angle_deg);               // Rotation in "Radweg"
		float k          = angle_dist / arc_len;                                 // Verh�ltnis Rotation / Vortrieb
	
		SENSOR_ENABLE = true;

    //const float turn_speed  = k * cruise_speed;
    //const float turn_accel  = k * acc;
	
    // 3) Sanftes Anfahren: 50 mm Rampe, um gleichm��ige Geschwindigkeit aufzubauen
    const float pre_straight = drive_before_curve;  // Anlaufstrecke
		const float after_straight = 180.0f;
    Profile_Start(&forward_profile, pre_straight+arc_len+after_straight, cruise_speed, cruise_speed, acc);
		while (Profile_GetPosition(&forward_profile) < (pre_straight-turnEarlier)) {
        // warten bis Kurve gefahren werden darf
    }
		
//    while (!Profile_IsFinished(&forward_profile)) {
//        // warten bis konstante Geschwindigkeit erreicht
//    }

    // 4) Jetzt gleichzeitigen Halbkreis starten (bei stabiler Vorw�rtsfahrt)
    //Profile_Start(&forward_profile, arc_len, cruise_speed, cruise_speed, acc); //Verursacht stocken
    Profile_SetSpeed(&rotation_profile, cruise_speed);
    Profile_Start(&rotation_profile, angle_dist, fabsf(cruise_speed), fabsf(cruise_speed), fabsf(acc));
		
		while (!Profile_IsFinished(&rotation_profile)) {
        // hier kann z. B. Motorupdate oder Telemetrie laufen
    }
		
		Profile_Stop(&rotation_profile);

    while (!Profile_IsFinished(&forward_profile) || !Profile_IsFinished(&rotation_profile)) {
        // hier kann z. B. Motorupdate oder Telemetrie laufen
    }

    // 5) Sanft abbremsen
    Profile_Start(&forward_profile, 0.0f, cruise_speed, 0.0f, acc);
    while (!Profile_IsFinished(&forward_profile)) {}

    Timer_Stop(TIMER_ID_TIM12);
    Motor_Left_SetPWM(0);
    Motor_Right_SetPWM(0);

    return true;
}



bool chain_profiles(float cruise_speed, float acc, angleInput* angleInputArray, uint8_t n_AngleInputArray, float drive_total)
{
	    // --- Initialisierung ---
    Encoder_Reset();
    PD_Reset(&ctrl_pos);
    PD_Reset(&ctrl_angle);
    Profile_Init(&forward_profile);
    Profile_Init(&rotation_profile);
    Timer_Start(TIMER_ID_TIM12);
	
		//Corrections
		volatile float const angleCorrection = 1.00f;
		float const turnEarlier = 0.0f;
		float getPosition = 0.0f;

    Profile_Start(&forward_profile, drive_total, cruise_speed, 0, acc);
	
		for(uint8_t i = 0; i < n_AngleInputArray;i++){
			Board_LED_On(1);
			getPosition =Profile_GetPosition(&forward_profile);
			while (getPosition > (angleInputArray[i].startPosition)) {
					// warten bis Kurve gefahren werden darf
					getPosition =Profile_GetPosition(&forward_profile);
			}
			printf("getPosition: %f\n",getPosition);
			printf("angleInputArray[i].startPosition: %f\n",angleInputArray[i].startPosition);
			
			if(angleInputArray[i].turns == 1){
				SetCurveFactorLeft(1.0f);
				SetCurveFactorRight(1.80f);
			}
			else{
				SetCurveFactorLeft(1.8f);
				SetCurveFactorRight(1.0f);
			}
			
			getPosition =Profile_GetPosition(&forward_profile);
			while (getPosition > (angleInputArray[i].endPosition)) {
					// warten bis Kurve fertig ist
				getPosition =Profile_GetPosition(&forward_profile);
			}
			printf("getPosition: %f\n",getPosition);
			printf("angleInputArray[i].endPosition: %f\n",angleInputArray[i].endPosition);
			SetCurveFactorLeft(1.0f);
			SetCurveFactorRight(1.0f);
		}
	
//		volatile bool IsNegative = false;
//	
//		for(uint8_t i = 0; i < n_AngleInputArray;i++){
//			Board_LED_On(1);
//			while (Profile_GetPosition(&forward_profile) < (angleInputArray[i].startPosition-(turnEarlier * (1+i)))) {
//					// warten bis Kurve gefahren werden darf
//			}
//			
//			Board_LED_Off(1);
//			
//			volatile float angle_dist = 0;
//			
//			angle_dist = 1 * angleInputArray[i].turns * DISTANCE_90DEG * angleInputArray[i].correctionValue;
//			if(angle_dist < 0 && IsNegative == false){
//				IsNegative = true;
//				acc = -acc;
//			}
//			else if(angle_dist > 0 && IsNegative == true){
//				IsNegative = false;
//				acc = -acc;
//			}
//			SENSOR_ENABLE = false;
//			Profile_Start_manual(&rotation_profile, angle_dist, fabsf(cruise_speed), 0, acc, 0);
//			
//			while (!Profile_IsFinished(&rotation_profile)) {
//					// hier kann z. B. Motorupdate oder Telemetrie laufen
//			}
//			Profile_Stop(&rotation_profile);
//			SENSOR_ENABLE = true;
//			Encoder_Reset_reset_Turn();
//			Profile_Reset(&rotation_profile);
//		}
		
		//Gerade fertig fahren & STOPPEN
		while (!Profile_IsFinished(&forward_profile) || !Profile_IsFinished(&rotation_profile)) {
        // hier kann z. B. Motorupdate oder Telemetrie laufen
    }

    // 5) Sanft abbremsen
    Profile_Start(&forward_profile, 0.0f, cruise_speed, 0.0f, acc);
    while (!Profile_IsFinished(&forward_profile)) {}

    Timer_Stop(TIMER_ID_TIM12);
    Motor_Left_SetPWM(0);
    Motor_Right_SetPWM(0);
	
	
		return true;
}





/*******************************************************************************
* SYSTEM ANALYSE
*******************************************************************************
* Create step response of the mouse
* 1. Button Press -> Drive and sample data
* 2. Button Press -> Send data via PY Debugger
*******************************************************************************/
void create_step_respose(void){
	Encoder_Reset();
	
	float time_array[SAMPLE_POINTS];
	float speed_array[SAMPLE_POINTS];
	
	float forward_speed = 0;
	
	if(BUTTON1_Pressed()){
		
		Delay_ms(1000);
		
		Motor_Left_SetPWM(STEP_INPUT_PWM);
		Motor_Right_SetPWM(STEP_INPUT_PWM);
		
		// measure
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
        {
			// Get current speed from encoders
			double speed_L = Encoder_GetSpeed_Left();
			double speed_R = Encoder_GetSpeed_Right();
			forward_speed = (speed_L + speed_R)/2;
			
			time_array[i] = i * SAMPLE_TIME_MS;
			speed_array[i] = forward_speed;//apply_fir_filter_velocity(forward_speed);
			
			
			Delay_ms(SAMPLE_TIME_MS);
        }
		
		Motor_Left_SetPWM(0);
		Motor_Right_SetPWM(0);
		
		while(!BUTTON1_Pressed()){} // wait for print data
		
		PY_Printf("TIME");
		// print time
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			PY_Printf("%f", time_array[i]);
			Delay_ms(5);
		}
		PY_Printf("VELOCITY");
		// print velocity
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			// speed
			PY_Printf("%f", speed_array[i]);
			Delay_ms(5);
		}
	}

}
void create_step_angle_vel(void){
	Encoder_Reset();
	
	float time_array[SAMPLE_POINTS];
	float speed_array[SAMPLE_POINTS];
	
	float angle_velocity = 0;
	
	if(BUTTON1_Pressed()){
		
		Delay_ms(1000);
		
		Motor_Left_SetPWM(ANGLE_INPUT_PWM);
		Motor_Right_SetPWM(-ANGLE_INPUT_PWM);
		
		// measure
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
        {
			// Get current speed from encoders
			double speed_L = Encoder_GetSpeed_Left();
			double speed_R = Encoder_GetSpeed_Right();
			angle_velocity = (speed_L - speed_R);
			
			time_array[i] = i * SAMPLE_TIME_MS;
			speed_array[i] = apply_fir_filter_velocity(angle_velocity);
			
			Delay_ms(SAMPLE_TIME_MS);
        }
		
		Motor_Left_SetPWM(0);
		Motor_Right_SetPWM(0);
		
		while(!BUTTON1_Pressed()){} // wait for print data
		
		PY_Printf("TIME");
		// print time
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			PY_Printf("%f", time_array[i]);
			Delay_ms(5);
		}
		PY_Printf("ANGLE VELOCITY");
		// print velocity
		for(uint16_t i = 0; i < SAMPLE_POINTS; i++)
		{
			// speed
			PY_Printf("%f", speed_array[i]);
			Delay_ms(5);
		}
	}

}

