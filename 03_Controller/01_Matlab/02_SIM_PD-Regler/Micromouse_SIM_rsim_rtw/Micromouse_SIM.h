/*
 * Micromouse_SIM.h
 *
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * Code generation for model "Micromouse_SIM".
 *
 * Model version              : 1.11
 * Simulink Coder version : 23.2 (R2023b) 01-Aug-2023
 * C source code generated on : Mon Oct 21 21:39:56 2024
 *
 * Target selection: rsim.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Debugging
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Micromouse_SIM_h_
#define RTW_HEADER_Micromouse_SIM_h_
#ifndef Micromouse_SIM_COMMON_INCLUDES_
#define Micromouse_SIM_COMMON_INCLUDES_
#include <stdlib.h>
#include "rtwtypes.h"
#include "simstruc.h"
#include "fixedpoint.h"
#include "rsim.h"
#include "rt_logging.h"
#include "dt_info.h"
#endif                                 /* Micromouse_SIM_COMMON_INCLUDES_ */

#include "Micromouse_SIM_types.h"
#include "rt_defines.h"
#include <stddef.h>
#include <string.h>
#include "rt_nonfinite.h"
#define MODEL_NAME                     Micromouse_SIM
#define NSAMPLE_TIMES                  (2)                       /* Number of sample times */
#define NINPUTS                        (0)                       /* Number of model inputs */
#define NOUTPUTS                       (0)                       /* Number of model outputs */
#define NBLOCKIO                       (38)                      /* Number of data output port signals */
#define NUM_ZC_EVENTS                  (0)                       /* Number of zero-crossing events */
#ifndef NCSTATES
#define NCSTATES                       (8)                       /* Number of continuous states */
#elif NCSTATES != 8
# error Invalid specification of NCSTATES defined in compiler command
#endif

#ifndef rtmGetDataMapInfo
#define rtmGetDataMapInfo(rtm)         (NULL)
#endif

#ifndef rtmSetDataMapInfo
#define rtmSetDataMapInfo(rtm, val)
#endif

/* Block signals (default storage) */
typedef struct {
  real_T Step;                         /* '<Root>/Step' */
  real_T Speedroundss;                 /* '<S1>/DC-Motor' */
  real_T Speedmms;                     /* '<S1>/Gain' */
  real_T Sum;                          /* '<Root>/Sum' */
  real_T ProportionalGain;             /* '<S44>/Proportional Gain' */
  real_T Integrator;                   /* '<S39>/Integrator' */
  real_T DerivativeGain;               /* '<S33>/Derivative Gain' */
  real_T Filter;                       /* '<S34>/Filter' */
  real_T SumD;                         /* '<S34>/SumD' */
  real_T FilterCoefficient;            /* '<S42>/Filter Coefficient' */
  real_T Sum_b;                        /* '<S48>/Sum' */
  real_T Saturation;                   /* '<S46>/Saturation' */
  real_T IntegralGain;                 /* '<S36>/Integral Gain' */
  real_T Step_g;                       /* '<S7>/Step' */
  real_T Clock;                        /* '<S7>/Clock' */
  real_T Sum_m;                        /* '<S7>/Sum' */
  real_T Product;                      /* '<S7>/Product' */
  real_T Output;                       /* '<S7>/Output' */
  real_T Saturation_d;                 /* '<Root>/Saturation' */
  real_T Integrator_f;                 /* '<Root>/Integrator' */
  real_T Sum2;                         /* '<Root>/Sum2' */
  real_T ProportionalGain_b;           /* '<S140>/Proportional Gain' */
  real_T SetSpeed;                     /* '<Root>/SetSpeed' */
  real_T Speedroundss_b;               /* '<S3>/DC-Motor' */
  real_T Speedmms_n;                   /* '<S3>/Gain' */
  real_T Sum1;                         /* '<Root>/Sum1' */
  real_T ProportionalGain_by;          /* '<S92>/Proportional Gain' */
  real_T Integrator_o;                 /* '<S87>/Integrator' */
  real_T DerivativeGain_k;             /* '<S81>/Derivative Gain' */
  real_T Filter_f;                     /* '<S82>/Filter' */
  real_T SumD_i;                       /* '<S82>/SumD' */
  real_T FilterCoefficient_g;          /* '<S90>/Filter Coefficient' */
  real_T Sum_j;                        /* '<S96>/Sum' */
  real_T Saturation_f;                 /* '<S94>/Saturation' */
  real_T IntegralGain_j;               /* '<S84>/Integral Gain' */
  real_T Speedroundsmin;               /* '<S2>/DC-Motor' */
  real_T Speedmms_h;                   /* '<S2>/Gain' */
  real_T PWM;                          /* '<Root>/PWM' */
} B;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  struct {
    void *LoggedData;
  } Ctrl_PWORK;                        /* '<Root>/Ctrl' */

  struct {
    void *LoggedData;
  } Error_PWORK;                       /* '<Root>/Error' */

  struct {
    void *LoggedData[2];
  } Scope_PWORK;                       /* '<Root>/Scope' */

  struct {
    void *LoggedData;
  } Ctrl1_PWORK;                       /* '<Root>/Ctrl1' */

  struct {
    void *LoggedData;
  } Error1_PWORK;                      /* '<Root>/Error1' */

  struct {
    void *LoggedData[2];
  } Scope1_PWORK;                      /* '<Root>/Scope1' */

  struct {
    void *LoggedData;
  } Scope2_PWORK;                      /* '<Root>/Scope2' */

  struct {
    void *LoggedData;
  } Scope3_PWORK;                      /* '<Root>/Scope3' */

  struct {
    void *LoggedData;
  } Wheel_PWORK;                       /* '<Root>/Wheel' */
} DW;

/* Continuous states (default storage) */
typedef struct {
  real_T DCMotor_CSTATE;               /* '<S1>/DC-Motor' */
  real_T Integrator_CSTATE;            /* '<S39>/Integrator' */
  real_T Filter_CSTATE;                /* '<S34>/Filter' */
  real_T Integrator_CSTATE_g;          /* '<Root>/Integrator' */
  real_T DCMotor_CSTATE_h;             /* '<S3>/DC-Motor' */
  real_T Integrator_CSTATE_l;          /* '<S87>/Integrator' */
  real_T Filter_CSTATE_h;              /* '<S82>/Filter' */
  real_T DCMotor_CSTATE_p;             /* '<S2>/DC-Motor' */
} X;

/* State derivatives (default storage) */
typedef struct {
  real_T DCMotor_CSTATE;               /* '<S1>/DC-Motor' */
  real_T Integrator_CSTATE;            /* '<S39>/Integrator' */
  real_T Filter_CSTATE;                /* '<S34>/Filter' */
  real_T Integrator_CSTATE_g;          /* '<Root>/Integrator' */
  real_T DCMotor_CSTATE_h;             /* '<S3>/DC-Motor' */
  real_T Integrator_CSTATE_l;          /* '<S87>/Integrator' */
  real_T Filter_CSTATE_h;              /* '<S82>/Filter' */
  real_T DCMotor_CSTATE_p;             /* '<S2>/DC-Motor' */
} XDot;

/* State disabled  */
typedef struct {
  boolean_T DCMotor_CSTATE;            /* '<S1>/DC-Motor' */
  boolean_T Integrator_CSTATE;         /* '<S39>/Integrator' */
  boolean_T Filter_CSTATE;             /* '<S34>/Filter' */
  boolean_T Integrator_CSTATE_g;       /* '<Root>/Integrator' */
  boolean_T DCMotor_CSTATE_h;          /* '<S3>/DC-Motor' */
  boolean_T Integrator_CSTATE_l;       /* '<S87>/Integrator' */
  boolean_T Filter_CSTATE_h;           /* '<S82>/Filter' */
  boolean_T DCMotor_CSTATE_p;          /* '<S2>/DC-Motor' */
} XDis;

/* External data declarations for dependent source files */
extern const char_T *RT_MEMORY_ALLOCATION_ERROR;
extern B rtB;                          /* block i/o */
extern X rtX;                          /* states (continuous) */
extern DW rtDW;                        /* states (dwork) */

/* Simulation Structure */
extern SimStruct *const rtS;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'Micromouse_SIM'
 * '<S1>'   : 'Micromouse_SIM/DC-Motor'
 * '<S2>'   : 'Micromouse_SIM/DC-Motor1'
 * '<S3>'   : 'Micromouse_SIM/DC-Motor2'
 * '<S4>'   : 'Micromouse_SIM/PID Controller'
 * '<S5>'   : 'Micromouse_SIM/PID Controller1'
 * '<S6>'   : 'Micromouse_SIM/PID Controller2'
 * '<S7>'   : 'Micromouse_SIM/Pos,soll'
 * '<S8>'   : 'Micromouse_SIM/PID Controller/Anti-windup'
 * '<S9>'   : 'Micromouse_SIM/PID Controller/D Gain'
 * '<S10>'  : 'Micromouse_SIM/PID Controller/Filter'
 * '<S11>'  : 'Micromouse_SIM/PID Controller/Filter ICs'
 * '<S12>'  : 'Micromouse_SIM/PID Controller/I Gain'
 * '<S13>'  : 'Micromouse_SIM/PID Controller/Ideal P Gain'
 * '<S14>'  : 'Micromouse_SIM/PID Controller/Ideal P Gain Fdbk'
 * '<S15>'  : 'Micromouse_SIM/PID Controller/Integrator'
 * '<S16>'  : 'Micromouse_SIM/PID Controller/Integrator ICs'
 * '<S17>'  : 'Micromouse_SIM/PID Controller/N Copy'
 * '<S18>'  : 'Micromouse_SIM/PID Controller/N Gain'
 * '<S19>'  : 'Micromouse_SIM/PID Controller/P Copy'
 * '<S20>'  : 'Micromouse_SIM/PID Controller/Parallel P Gain'
 * '<S21>'  : 'Micromouse_SIM/PID Controller/Reset Signal'
 * '<S22>'  : 'Micromouse_SIM/PID Controller/Saturation'
 * '<S23>'  : 'Micromouse_SIM/PID Controller/Saturation Fdbk'
 * '<S24>'  : 'Micromouse_SIM/PID Controller/Sum'
 * '<S25>'  : 'Micromouse_SIM/PID Controller/Sum Fdbk'
 * '<S26>'  : 'Micromouse_SIM/PID Controller/Tracking Mode'
 * '<S27>'  : 'Micromouse_SIM/PID Controller/Tracking Mode Sum'
 * '<S28>'  : 'Micromouse_SIM/PID Controller/Tsamp - Integral'
 * '<S29>'  : 'Micromouse_SIM/PID Controller/Tsamp - Ngain'
 * '<S30>'  : 'Micromouse_SIM/PID Controller/postSat Signal'
 * '<S31>'  : 'Micromouse_SIM/PID Controller/preSat Signal'
 * '<S32>'  : 'Micromouse_SIM/PID Controller/Anti-windup/Passthrough'
 * '<S33>'  : 'Micromouse_SIM/PID Controller/D Gain/Internal Parameters'
 * '<S34>'  : 'Micromouse_SIM/PID Controller/Filter/Cont. Filter'
 * '<S35>'  : 'Micromouse_SIM/PID Controller/Filter ICs/Internal IC - Filter'
 * '<S36>'  : 'Micromouse_SIM/PID Controller/I Gain/Internal Parameters'
 * '<S37>'  : 'Micromouse_SIM/PID Controller/Ideal P Gain/Passthrough'
 * '<S38>'  : 'Micromouse_SIM/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S39>'  : 'Micromouse_SIM/PID Controller/Integrator/Continuous'
 * '<S40>'  : 'Micromouse_SIM/PID Controller/Integrator ICs/Internal IC'
 * '<S41>'  : 'Micromouse_SIM/PID Controller/N Copy/Disabled'
 * '<S42>'  : 'Micromouse_SIM/PID Controller/N Gain/Internal Parameters'
 * '<S43>'  : 'Micromouse_SIM/PID Controller/P Copy/Disabled'
 * '<S44>'  : 'Micromouse_SIM/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S45>'  : 'Micromouse_SIM/PID Controller/Reset Signal/Disabled'
 * '<S46>'  : 'Micromouse_SIM/PID Controller/Saturation/Enabled'
 * '<S47>'  : 'Micromouse_SIM/PID Controller/Saturation Fdbk/Disabled'
 * '<S48>'  : 'Micromouse_SIM/PID Controller/Sum/Sum_PID'
 * '<S49>'  : 'Micromouse_SIM/PID Controller/Sum Fdbk/Disabled'
 * '<S50>'  : 'Micromouse_SIM/PID Controller/Tracking Mode/Disabled'
 * '<S51>'  : 'Micromouse_SIM/PID Controller/Tracking Mode Sum/Passthrough'
 * '<S52>'  : 'Micromouse_SIM/PID Controller/Tsamp - Integral/TsSignalSpecification'
 * '<S53>'  : 'Micromouse_SIM/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S54>'  : 'Micromouse_SIM/PID Controller/postSat Signal/Forward_Path'
 * '<S55>'  : 'Micromouse_SIM/PID Controller/preSat Signal/Forward_Path'
 * '<S56>'  : 'Micromouse_SIM/PID Controller1/Anti-windup'
 * '<S57>'  : 'Micromouse_SIM/PID Controller1/D Gain'
 * '<S58>'  : 'Micromouse_SIM/PID Controller1/Filter'
 * '<S59>'  : 'Micromouse_SIM/PID Controller1/Filter ICs'
 * '<S60>'  : 'Micromouse_SIM/PID Controller1/I Gain'
 * '<S61>'  : 'Micromouse_SIM/PID Controller1/Ideal P Gain'
 * '<S62>'  : 'Micromouse_SIM/PID Controller1/Ideal P Gain Fdbk'
 * '<S63>'  : 'Micromouse_SIM/PID Controller1/Integrator'
 * '<S64>'  : 'Micromouse_SIM/PID Controller1/Integrator ICs'
 * '<S65>'  : 'Micromouse_SIM/PID Controller1/N Copy'
 * '<S66>'  : 'Micromouse_SIM/PID Controller1/N Gain'
 * '<S67>'  : 'Micromouse_SIM/PID Controller1/P Copy'
 * '<S68>'  : 'Micromouse_SIM/PID Controller1/Parallel P Gain'
 * '<S69>'  : 'Micromouse_SIM/PID Controller1/Reset Signal'
 * '<S70>'  : 'Micromouse_SIM/PID Controller1/Saturation'
 * '<S71>'  : 'Micromouse_SIM/PID Controller1/Saturation Fdbk'
 * '<S72>'  : 'Micromouse_SIM/PID Controller1/Sum'
 * '<S73>'  : 'Micromouse_SIM/PID Controller1/Sum Fdbk'
 * '<S74>'  : 'Micromouse_SIM/PID Controller1/Tracking Mode'
 * '<S75>'  : 'Micromouse_SIM/PID Controller1/Tracking Mode Sum'
 * '<S76>'  : 'Micromouse_SIM/PID Controller1/Tsamp - Integral'
 * '<S77>'  : 'Micromouse_SIM/PID Controller1/Tsamp - Ngain'
 * '<S78>'  : 'Micromouse_SIM/PID Controller1/postSat Signal'
 * '<S79>'  : 'Micromouse_SIM/PID Controller1/preSat Signal'
 * '<S80>'  : 'Micromouse_SIM/PID Controller1/Anti-windup/Passthrough'
 * '<S81>'  : 'Micromouse_SIM/PID Controller1/D Gain/Internal Parameters'
 * '<S82>'  : 'Micromouse_SIM/PID Controller1/Filter/Cont. Filter'
 * '<S83>'  : 'Micromouse_SIM/PID Controller1/Filter ICs/Internal IC - Filter'
 * '<S84>'  : 'Micromouse_SIM/PID Controller1/I Gain/Internal Parameters'
 * '<S85>'  : 'Micromouse_SIM/PID Controller1/Ideal P Gain/Passthrough'
 * '<S86>'  : 'Micromouse_SIM/PID Controller1/Ideal P Gain Fdbk/Disabled'
 * '<S87>'  : 'Micromouse_SIM/PID Controller1/Integrator/Continuous'
 * '<S88>'  : 'Micromouse_SIM/PID Controller1/Integrator ICs/Internal IC'
 * '<S89>'  : 'Micromouse_SIM/PID Controller1/N Copy/Disabled'
 * '<S90>'  : 'Micromouse_SIM/PID Controller1/N Gain/Internal Parameters'
 * '<S91>'  : 'Micromouse_SIM/PID Controller1/P Copy/Disabled'
 * '<S92>'  : 'Micromouse_SIM/PID Controller1/Parallel P Gain/Internal Parameters'
 * '<S93>'  : 'Micromouse_SIM/PID Controller1/Reset Signal/Disabled'
 * '<S94>'  : 'Micromouse_SIM/PID Controller1/Saturation/Enabled'
 * '<S95>'  : 'Micromouse_SIM/PID Controller1/Saturation Fdbk/Disabled'
 * '<S96>'  : 'Micromouse_SIM/PID Controller1/Sum/Sum_PID'
 * '<S97>'  : 'Micromouse_SIM/PID Controller1/Sum Fdbk/Disabled'
 * '<S98>'  : 'Micromouse_SIM/PID Controller1/Tracking Mode/Disabled'
 * '<S99>'  : 'Micromouse_SIM/PID Controller1/Tracking Mode Sum/Passthrough'
 * '<S100>' : 'Micromouse_SIM/PID Controller1/Tsamp - Integral/TsSignalSpecification'
 * '<S101>' : 'Micromouse_SIM/PID Controller1/Tsamp - Ngain/Passthrough'
 * '<S102>' : 'Micromouse_SIM/PID Controller1/postSat Signal/Forward_Path'
 * '<S103>' : 'Micromouse_SIM/PID Controller1/preSat Signal/Forward_Path'
 * '<S104>' : 'Micromouse_SIM/PID Controller2/Anti-windup'
 * '<S105>' : 'Micromouse_SIM/PID Controller2/D Gain'
 * '<S106>' : 'Micromouse_SIM/PID Controller2/Filter'
 * '<S107>' : 'Micromouse_SIM/PID Controller2/Filter ICs'
 * '<S108>' : 'Micromouse_SIM/PID Controller2/I Gain'
 * '<S109>' : 'Micromouse_SIM/PID Controller2/Ideal P Gain'
 * '<S110>' : 'Micromouse_SIM/PID Controller2/Ideal P Gain Fdbk'
 * '<S111>' : 'Micromouse_SIM/PID Controller2/Integrator'
 * '<S112>' : 'Micromouse_SIM/PID Controller2/Integrator ICs'
 * '<S113>' : 'Micromouse_SIM/PID Controller2/N Copy'
 * '<S114>' : 'Micromouse_SIM/PID Controller2/N Gain'
 * '<S115>' : 'Micromouse_SIM/PID Controller2/P Copy'
 * '<S116>' : 'Micromouse_SIM/PID Controller2/Parallel P Gain'
 * '<S117>' : 'Micromouse_SIM/PID Controller2/Reset Signal'
 * '<S118>' : 'Micromouse_SIM/PID Controller2/Saturation'
 * '<S119>' : 'Micromouse_SIM/PID Controller2/Saturation Fdbk'
 * '<S120>' : 'Micromouse_SIM/PID Controller2/Sum'
 * '<S121>' : 'Micromouse_SIM/PID Controller2/Sum Fdbk'
 * '<S122>' : 'Micromouse_SIM/PID Controller2/Tracking Mode'
 * '<S123>' : 'Micromouse_SIM/PID Controller2/Tracking Mode Sum'
 * '<S124>' : 'Micromouse_SIM/PID Controller2/Tsamp - Integral'
 * '<S125>' : 'Micromouse_SIM/PID Controller2/Tsamp - Ngain'
 * '<S126>' : 'Micromouse_SIM/PID Controller2/postSat Signal'
 * '<S127>' : 'Micromouse_SIM/PID Controller2/preSat Signal'
 * '<S128>' : 'Micromouse_SIM/PID Controller2/Anti-windup/Disabled'
 * '<S129>' : 'Micromouse_SIM/PID Controller2/D Gain/Disabled'
 * '<S130>' : 'Micromouse_SIM/PID Controller2/Filter/Disabled'
 * '<S131>' : 'Micromouse_SIM/PID Controller2/Filter ICs/Disabled'
 * '<S132>' : 'Micromouse_SIM/PID Controller2/I Gain/Disabled'
 * '<S133>' : 'Micromouse_SIM/PID Controller2/Ideal P Gain/Passthrough'
 * '<S134>' : 'Micromouse_SIM/PID Controller2/Ideal P Gain Fdbk/Disabled'
 * '<S135>' : 'Micromouse_SIM/PID Controller2/Integrator/Disabled'
 * '<S136>' : 'Micromouse_SIM/PID Controller2/Integrator ICs/Disabled'
 * '<S137>' : 'Micromouse_SIM/PID Controller2/N Copy/Disabled wSignal Specification'
 * '<S138>' : 'Micromouse_SIM/PID Controller2/N Gain/Disabled'
 * '<S139>' : 'Micromouse_SIM/PID Controller2/P Copy/Disabled'
 * '<S140>' : 'Micromouse_SIM/PID Controller2/Parallel P Gain/Internal Parameters'
 * '<S141>' : 'Micromouse_SIM/PID Controller2/Reset Signal/Disabled'
 * '<S142>' : 'Micromouse_SIM/PID Controller2/Saturation/Passthrough'
 * '<S143>' : 'Micromouse_SIM/PID Controller2/Saturation Fdbk/Disabled'
 * '<S144>' : 'Micromouse_SIM/PID Controller2/Sum/Passthrough_P'
 * '<S145>' : 'Micromouse_SIM/PID Controller2/Sum Fdbk/Disabled'
 * '<S146>' : 'Micromouse_SIM/PID Controller2/Tracking Mode/Disabled'
 * '<S147>' : 'Micromouse_SIM/PID Controller2/Tracking Mode Sum/Passthrough'
 * '<S148>' : 'Micromouse_SIM/PID Controller2/Tsamp - Integral/TsSignalSpecification'
 * '<S149>' : 'Micromouse_SIM/PID Controller2/Tsamp - Ngain/Passthrough'
 * '<S150>' : 'Micromouse_SIM/PID Controller2/postSat Signal/Forward_Path'
 * '<S151>' : 'Micromouse_SIM/PID Controller2/preSat Signal/Forward_Path'
 */

/* user code (bottom of header file) */
extern const int_T gblNumToFiles;
extern const int_T gblNumFrFiles;
extern const int_T gblNumFrWksBlocks;
extern rtInportTUtable *gblInportTUtables;
extern const char *gblInportFileName;
extern const int_T gblNumRootInportBlks;
extern const int_T gblNumModelInputs;
extern const int_T gblInportDataTypeIdx[];
extern const int_T gblInportDims[];
extern const int_T gblInportComplex[];
extern const int_T gblInportInterpoFlag[];
extern const int_T gblInportContinuous[];

#endif                                 /* RTW_HEADER_Micromouse_SIM_h_ */
