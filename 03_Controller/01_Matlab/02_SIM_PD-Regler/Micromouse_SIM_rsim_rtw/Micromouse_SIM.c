/*
 * Micromouse_SIM.c
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

#include "Micromouse_SIM.h"
#include "rtwtypes.h"
#include "Micromouse_SIM_private.h"
#include "Micromouse_SIM_dt.h"

/* user code (top of parameter file) */
const int_T gblNumToFiles = 0;
const int_T gblNumFrFiles = 0;
const int_T gblNumFrWksBlocks = 0;
const char *gblSlvrJacPatternFileName =
  "Micromouse_SIM_rsim_rtw\\Micromouse_SIM_Jpattern.mat";

/* Root inports information  */
const int_T gblNumRootInportBlks = 0;
const int_T gblNumModelInputs = 0;
extern rtInportTUtable *gblInportTUtables;
extern const char *gblInportFileName;
const int_T gblInportDataTypeIdx[] = { -1 };

const int_T gblInportDims[] = { -1 } ;

const int_T gblInportComplex[] = { -1 };

const int_T gblInportInterpoFlag[] = { -1 };

const int_T gblInportContinuous[] = { -1 };

#include "simstruc.h"
#include "fixedpoint.h"

/* Block signals (default storage) */
B rtB;

/* Continuous states */
X rtX;

/* Disabled State Vector */
XDis rtXDis;

/* Block states (default storage) */
DW rtDW;

/* Parent Simstruct */
static SimStruct model_S;
SimStruct *const rtS = &model_S;

/* System initialize for root system: '<Root>' */
void MdlInitialize(void)
{
  /* InitializeConditions for TransferFcn: '<S1>/DC-Motor' */
  rtX.DCMotor_CSTATE = 0.0;

  /* InitializeConditions for Integrator: '<S39>/Integrator' */
  rtX.Integrator_CSTATE = 0.0;

  /* InitializeConditions for Integrator: '<S34>/Filter' */
  rtX.Filter_CSTATE = 0.0;

  /* InitializeConditions for Integrator: '<Root>/Integrator' */
  rtX.Integrator_CSTATE_g = 0.0;

  /* InitializeConditions for TransferFcn: '<S3>/DC-Motor' */
  rtX.DCMotor_CSTATE_h = 0.0;

  /* InitializeConditions for Integrator: '<S87>/Integrator' */
  rtX.Integrator_CSTATE_l = 0.0;

  /* InitializeConditions for Integrator: '<S82>/Filter' */
  rtX.Filter_CSTATE_h = 0.0;

  /* InitializeConditions for TransferFcn: '<S2>/DC-Motor' */
  rtX.DCMotor_CSTATE_p = 0.0;
}

/* Start for root system: '<Root>' */
void MdlStart(void)
{
  MdlInitialize();
}

/* Outputs for root system: '<Root>' */
void MdlOutputs(int_T tid)
{
  real_T currentTime;

  /* Step: '<Root>/Step' */
  currentTime = ssGetTaskTime(rtS,0);
  if (currentTime < 1.0) {
    /* Step: '<Root>/Step' */
    rtB.Step = 0.0;
  } else {
    /* Step: '<Root>/Step' */
    rtB.Step = 500.0;
  }

  /* End of Step: '<Root>/Step' */

  /* TransferFcn: '<S1>/DC-Motor' */
  rtB.Speedroundss = 44.892307692307689 * rtX.DCMotor_CSTATE;

  /* Gain: '<S1>/Gain' */
  rtB.Speedmms = 1.3088333333333333 * rtB.Speedroundss;

  /* Sum: '<Root>/Sum' */
  rtB.Sum = rtB.Step - rtB.Speedmms;

  /* Gain: '<S44>/Proportional Gain' */
  rtB.ProportionalGain = 56.842501092839377 * rtB.Sum;

  /* Integrator: '<S39>/Integrator' */
  rtB.Integrator = rtX.Integrator_CSTATE;

  /* Gain: '<S33>/Derivative Gain' */
  rtB.DerivativeGain = 1.522569274454127 * rtB.Sum;

  /* Integrator: '<S34>/Filter' */
  rtB.Filter = rtX.Filter_CSTATE;

  /* Sum: '<S34>/SumD' */
  rtB.SumD = rtB.DerivativeGain - rtB.Filter;

  /* Gain: '<S42>/Filter Coefficient' */
  rtB.FilterCoefficient = 0.0 * rtB.SumD;

  /* Sum: '<S48>/Sum' */
  rtB.Sum_b = (rtB.ProportionalGain + rtB.Integrator) + rtB.FilterCoefficient;

  /* Saturate: '<S46>/Saturation' */
  currentTime = rtB.Sum_b;
  if (currentTime > 255.0) {
    /* Saturate: '<S46>/Saturation' */
    rtB.Saturation = 255.0;
  } else if (currentTime < -255.0) {
    /* Saturate: '<S46>/Saturation' */
    rtB.Saturation = -255.0;
  } else {
    /* Saturate: '<S46>/Saturation' */
    rtB.Saturation = currentTime;
  }

  /* End of Saturate: '<S46>/Saturation' */
  if (ssIsSampleHit(rtS, 1, 0)) {
  }

  /* Gain: '<S36>/Integral Gain' */
  rtB.IntegralGain = 0.1 * rtB.Sum;

  /* Step: '<S7>/Step' */
  currentTime = ssGetTaskTime(rtS,0);
  if (currentTime < 1.0) {
    /* Step: '<S7>/Step' */
    rtB.Step_g = 0.0;
  } else {
    /* Step: '<S7>/Step' */
    rtB.Step_g = 600.0;
  }

  /* End of Step: '<S7>/Step' */

  /* Clock: '<S7>/Clock' */
  rtB.Clock = ssGetT(rtS);

  /* Sum: '<S7>/Sum' incorporates:
   *  Constant: '<S7>/Constant'
   */
  rtB.Sum_m = rtB.Clock - 1.0;

  /* Product: '<S7>/Product' */
  rtB.Product = rtB.Step_g * rtB.Sum_m;

  /* Sum: '<S7>/Output' */
  rtB.Output = rtB.Product;

  /* Saturate: '<Root>/Saturation' */
  currentTime = rtB.Output;
  if (currentTime > 1000.0) {
    /* Saturate: '<Root>/Saturation' */
    rtB.Saturation_d = 1000.0;
  } else if (currentTime < 0.0) {
    /* Saturate: '<Root>/Saturation' */
    rtB.Saturation_d = 0.0;
  } else {
    /* Saturate: '<Root>/Saturation' */
    rtB.Saturation_d = currentTime;
  }

  /* End of Saturate: '<Root>/Saturation' */

  /* Integrator: '<Root>/Integrator' */
  rtB.Integrator_f = rtX.Integrator_CSTATE_g;

  /* Sum: '<Root>/Sum2' */
  rtB.Sum2 = rtB.Saturation_d - rtB.Integrator_f;

  /* Gain: '<S140>/Proportional Gain' */
  rtB.ProportionalGain_b = 30.0 * rtB.Sum2;

  /* Saturate: '<Root>/SetSpeed' */
  currentTime = rtB.ProportionalGain_b;
  if (currentTime > 1000.0) {
    /* Saturate: '<Root>/SetSpeed' */
    rtB.SetSpeed = 1000.0;
  } else if (currentTime < 0.0) {
    /* Saturate: '<Root>/SetSpeed' */
    rtB.SetSpeed = 0.0;
  } else {
    /* Saturate: '<Root>/SetSpeed' */
    rtB.SetSpeed = currentTime;
  }

  /* End of Saturate: '<Root>/SetSpeed' */

  /* TransferFcn: '<S3>/DC-Motor' */
  rtB.Speedroundss_b = 44.892307692307689 * rtX.DCMotor_CSTATE_h;

  /* Gain: '<S3>/Gain' */
  rtB.Speedmms_n = 1.3088333333333333 * rtB.Speedroundss_b;

  /* Sum: '<Root>/Sum1' */
  rtB.Sum1 = rtB.SetSpeed - rtB.Speedmms_n;

  /* Gain: '<S92>/Proportional Gain' */
  rtB.ProportionalGain_by = 56.842501092839377 * rtB.Sum1;

  /* Integrator: '<S87>/Integrator' */
  rtB.Integrator_o = rtX.Integrator_CSTATE_l;

  /* Gain: '<S81>/Derivative Gain' */
  rtB.DerivativeGain_k = 1.522569274454127 * rtB.Sum1;

  /* Integrator: '<S82>/Filter' */
  rtB.Filter_f = rtX.Filter_CSTATE_h;

  /* Sum: '<S82>/SumD' */
  rtB.SumD_i = rtB.DerivativeGain_k - rtB.Filter_f;

  /* Gain: '<S90>/Filter Coefficient' */
  rtB.FilterCoefficient_g = 0.0 * rtB.SumD_i;

  /* Sum: '<S96>/Sum' */
  rtB.Sum_j = (rtB.ProportionalGain_by + rtB.Integrator_o) +
    rtB.FilterCoefficient_g;

  /* Saturate: '<S94>/Saturation' */
  currentTime = rtB.Sum_j;
  if (currentTime > 255.0) {
    /* Saturate: '<S94>/Saturation' */
    rtB.Saturation_f = 255.0;
  } else if (currentTime < -255.0) {
    /* Saturate: '<S94>/Saturation' */
    rtB.Saturation_f = -255.0;
  } else {
    /* Saturate: '<S94>/Saturation' */
    rtB.Saturation_f = currentTime;
  }

  /* End of Saturate: '<S94>/Saturation' */
  if (ssIsSampleHit(rtS, 1, 0)) {
  }

  /* Gain: '<S84>/Integral Gain' */
  rtB.IntegralGain_j = 0.1 * rtB.Sum1;

  /* TransferFcn: '<S2>/DC-Motor' */
  rtB.Speedroundsmin = 44.892307692307689 * rtX.DCMotor_CSTATE_p;

  /* Gain: '<S2>/Gain' */
  rtB.Speedmms_h = 1.3088333333333333 * rtB.Speedroundsmin;
  if (ssIsSampleHit(rtS, 1, 0)) {
  }

  /* Step: '<Root>/PWM' */
  currentTime = ssGetTaskTime(rtS,0);
  if (currentTime < 0.0) {
    /* Step: '<Root>/PWM' */
    rtB.PWM = 0.0;
  } else {
    /* Step: '<Root>/PWM' */
    rtB.PWM = 255.0;
  }

  /* End of Step: '<Root>/PWM' */
  UNUSED_PARAMETER(tid);
}

/* Update for root system: '<Root>' */
void MdlUpdate(int_T tid)
{
  UNUSED_PARAMETER(tid);
}

/* Derivatives for root system: '<Root>' */
void MdlDerivatives(void)
{
  XDot *_rtXdot;
  _rtXdot = ((XDot *) ssGetdX(rtS));

  /* Derivatives for TransferFcn: '<S1>/DC-Motor' */
  _rtXdot->DCMotor_CSTATE = -3.0769230769230766 * rtX.DCMotor_CSTATE;
  _rtXdot->DCMotor_CSTATE += rtB.Saturation;

  /* Derivatives for Integrator: '<S39>/Integrator' */
  _rtXdot->Integrator_CSTATE = rtB.IntegralGain;

  /* Derivatives for Integrator: '<S34>/Filter' */
  _rtXdot->Filter_CSTATE = rtB.FilterCoefficient;

  /* Derivatives for Integrator: '<Root>/Integrator' */
  _rtXdot->Integrator_CSTATE_g = rtB.Speedmms_n;

  /* Derivatives for TransferFcn: '<S3>/DC-Motor' */
  _rtXdot->DCMotor_CSTATE_h = -3.0769230769230766 * rtX.DCMotor_CSTATE_h;
  _rtXdot->DCMotor_CSTATE_h += rtB.Saturation_f;

  /* Derivatives for Integrator: '<S87>/Integrator' */
  _rtXdot->Integrator_CSTATE_l = rtB.IntegralGain_j;

  /* Derivatives for Integrator: '<S82>/Filter' */
  _rtXdot->Filter_CSTATE_h = rtB.FilterCoefficient_g;

  /* Derivatives for TransferFcn: '<S2>/DC-Motor' */
  _rtXdot->DCMotor_CSTATE_p = -3.0769230769230766 * rtX.DCMotor_CSTATE_p;
  _rtXdot->DCMotor_CSTATE_p += rtB.PWM;
}

/* Projection for root system: '<Root>' */
void MdlProjection(void)
{
}

/* Termination for root system: '<Root>' */
void MdlTerminate(void)
{
}

/* Function to initialize sizes */
void MdlInitializeSizes(void)
{
  ssSetNumContStates(rtS, 8);          /* Number of continuous states */
  ssSetNumPeriodicContStates(rtS, 0); /* Number of periodic continuous states */
  ssSetNumY(rtS, 0);                   /* Number of model outputs */
  ssSetNumU(rtS, 0);                   /* Number of model inputs */
  ssSetDirectFeedThrough(rtS, 0);      /* The model is not direct feedthrough */
  ssSetNumSampleTimes(rtS, 2);         /* Number of sample times */
  ssSetNumBlocks(rtS, 49);             /* Number of blocks */
  ssSetNumBlockIO(rtS, 38);            /* Number of block outputs */
}

/* Function to initialize sample times. */
void MdlInitializeSampleTimes(void)
{
  /* task periods */
  ssSetSampleTime(rtS, 0, 0.0);
  ssSetSampleTime(rtS, 1, 0.2);

  /* task offsets */
  ssSetOffsetTime(rtS, 0, 0.0);
  ssSetOffsetTime(rtS, 1, 0.0);
}

/* Function to register the model */
/* Turns off all optimizations on Windows because of issues with VC 2015 compiler.
   This function is not performance-critical, hence this is not a problem.
 */
#if defined(_MSC_VER)

#pragma optimize( "", off )

#endif

SimStruct * Micromouse_SIM(void)
{
  static struct _ssMdlInfo mdlInfo;
  static struct _ssBlkInfo2 blkInfo2;
  static struct _ssBlkInfoSLSize blkInfoSLSize;
  (void) memset((char_T *)rtS, 0,
                sizeof(SimStruct));
  (void) memset((char_T *)&mdlInfo, 0,
                sizeof(struct _ssMdlInfo));
  (void) memset((char_T *)&blkInfo2, 0,
                sizeof(struct _ssBlkInfo2));
  (void) memset((char_T *)&blkInfoSLSize, 0,
                sizeof(struct _ssBlkInfoSLSize));
  ssSetBlkInfo2Ptr(rtS, &blkInfo2);
  ssSetBlkInfoSLSizePtr(rtS, &blkInfoSLSize);
  ssSetMdlInfoPtr(rtS, &mdlInfo);

  /* timing info */
  {
    static time_T mdlPeriod[NSAMPLE_TIMES];
    static time_T mdlOffset[NSAMPLE_TIMES];
    static time_T mdlTaskTimes[NSAMPLE_TIMES];
    static int_T mdlTsMap[NSAMPLE_TIMES];
    static int_T mdlSampleHits[NSAMPLE_TIMES];

    {
      int_T i;
      for (i = 0; i < NSAMPLE_TIMES; i++) {
        mdlPeriod[i] = 0.0;
        mdlOffset[i] = 0.0;
        mdlTaskTimes[i] = 0.0;
        mdlTsMap[i] = i;
        mdlSampleHits[i] = 1;
      }
    }

    ssSetSampleTimePtr(rtS, &mdlPeriod[0]);
    ssSetOffsetTimePtr(rtS, &mdlOffset[0]);
    ssSetSampleTimeTaskIDPtr(rtS, &mdlTsMap[0]);
    ssSetTPtr(rtS, &mdlTaskTimes[0]);
    ssSetSampleHitPtr(rtS, &mdlSampleHits[0]);
  }

  ssSetSolverMode(rtS, SOLVER_MODE_SINGLETASKING);

  /*
   * initialize model vectors and cache them in SimStruct
   */

  /* block I/O */
  {
    ssSetBlockIO(rtS, ((void *) &rtB));
    (void) memset(((void *) &rtB), 0,
                  sizeof(B));
  }

  /* states (continuous)*/
  {
    real_T *x = (real_T *) &rtX;
    ssSetContStates(rtS, x);
    (void) memset((void *)x, 0,
                  sizeof(X));
  }

  /* states (dwork) */
  {
    void *dwork = (void *) &rtDW;
    ssSetRootDWork(rtS, dwork);
    (void) memset(dwork, 0,
                  sizeof(DW));
  }

  /* data type transition information */
  {
    static DataTypeTransInfo dtInfo;
    (void) memset((char_T *) &dtInfo, 0,
                  sizeof(dtInfo));
    ssSetModelMappingInfo(rtS, &dtInfo);
    dtInfo.numDataTypes = 23;
    dtInfo.dataTypeSizes = &rtDataTypeSizes[0];
    dtInfo.dataTypeNames = &rtDataTypeNames[0];

    /* Block I/O transition table */
    dtInfo.BTransTable = &rtBTransTable;
  }

  /* Model specific registration */
  ssSetRootSS(rtS, rtS);
  ssSetVersion(rtS, SIMSTRUCT_VERSION_LEVEL2);
  ssSetModelName(rtS, "Micromouse_SIM");
  ssSetPath(rtS, "Micromouse_SIM");
  ssSetTStart(rtS, 0.0);
  ssSetTFinal(rtS, 10.0);
  ssSetStepSize(rtS, 0.2);
  ssSetFixedStepSize(rtS, 0.2);

  /* Setup for data logging */
  {
    static RTWLogInfo rt_DataLoggingInfo;
    rt_DataLoggingInfo.loggingInterval = (NULL);
    ssSetRTWLogInfo(rtS, &rt_DataLoggingInfo);
  }

  /* Setup for data logging */
  {
    rtliSetLogXSignalInfo(ssGetRTWLogInfo(rtS), (NULL));
    rtliSetLogXSignalPtrs(ssGetRTWLogInfo(rtS), (NULL));
    rtliSetLogT(ssGetRTWLogInfo(rtS), "tout");
    rtliSetLogX(ssGetRTWLogInfo(rtS), "");
    rtliSetLogXFinal(ssGetRTWLogInfo(rtS), "");
    rtliSetLogVarNameModifier(ssGetRTWLogInfo(rtS), "rt_");
    rtliSetLogFormat(ssGetRTWLogInfo(rtS), 4);
    rtliSetLogMaxRows(ssGetRTWLogInfo(rtS), 0);
    rtliSetLogDecimation(ssGetRTWLogInfo(rtS), 1);
    rtliSetLogY(ssGetRTWLogInfo(rtS), "");
    rtliSetLogYSignalInfo(ssGetRTWLogInfo(rtS), (NULL));
    rtliSetLogYSignalPtrs(ssGetRTWLogInfo(rtS), (NULL));
  }

  {
    static struct _ssStatesInfo2 statesInfo2;
    ssSetStatesInfo2(rtS, &statesInfo2);
  }

  {
    static ssPeriodicStatesInfo periodicStatesInfo;
    ssSetPeriodicStatesInfo(rtS, &periodicStatesInfo);
  }

  {
    static ssJacobianPerturbationBounds jacobianPerturbationBounds;
    ssSetJacobianPerturbationBounds(rtS, &jacobianPerturbationBounds);
  }

  ssSetChecksumVal(rtS, 0, 1891552475U);
  ssSetChecksumVal(rtS, 1, 3064122531U);
  ssSetChecksumVal(rtS, 2, 4179513103U);
  ssSetChecksumVal(rtS, 3, 2147734223U);
  return rtS;
}

/* When you use the on parameter, it resets the optimizations to those that you
   specified with the /O compiler option. */
#if defined(_MSC_VER)

#pragma optimize( "", on )

#endif
