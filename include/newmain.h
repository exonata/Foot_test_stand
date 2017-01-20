/**
 *  EKSO BIONICS
 *
 *  Copyright 2016, Ekso Bionics, as an unpublished work.
 *  All rights reserved.  This computer software is
 *  PROPRIETARY INFORMATION of Ekso Bionics
 *  and shall not be reproduced, disclosed or used without written
 *  permission of Ekso Bionics.  Multiple Patents Pending.
 *
 */
/**
 *  @file main.h
 *  @brief Main module to control foot test stand and allow for user control through command line input
 *
 *
 *  $Revision: $1.0
 *  $Date: $04/11/2016
 *
 */

#ifndef NEWMAIN_H_
#define NEWMAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "./BBBIOlib/BBBio_lib/BBBiolib.h"
#include <gtk/gtk.h>
#include <gui.h>
#include <fcntl.h>     //define O_WONLY and O_RDONLY
#include <math.h>
#include <time.h> //for delay func


/**Define all  pin numbers for  Foot Test Stand*/
#define HEADER_P8 8
#define HEADER_P9 9
#define LOAD_CELL_1  0  //AIN_0
#define LOAD_CELL_2  1  //AIN_1
#define SOL_VALVE_1 11  //GPIO 11, P8 up-down sample 1
#define SOL_VALVE_2 12  //GPIO 12, P8 up-down sample 1
#define SOL_VALVE_3 15 //GPIO 15, P8 up-down sample 2
#define SOL_VALVE_4 16 //GPIO 16, P8 up-down sample 2
#define SOL_VALVE_5 17 //GPIO 7, P8 rotary sample 1
#define SOL_VALVE_6 19 //GPIO 8, P8 rotary sample 2
#define PRS_REG_1    0  //PWMSS_0A P9 22
#define RTC_SLC     19  //Real time clock SLC pin
#define RTC_SDA     20  // Real time clock SDA pin
#define HEEL_1_ADC   2  //AIN_2
#define TOE_1_ADC    3  //AIN_3
#define HEEL_2_ADC   4  //AIN_4
#define TOE_2_ADC    5  //AIN_5

/**define Buffer size for ADC*/
#define BUFFER_SIZE 100
#define SAMPLE_SIZE 1

/**PWM defines*/
#define PWM_HZ 500

/**--data logging--*/
#define LOGNAME_SIZE 255
#define LOG_TIMER_MS 12 //[msec]
#define LOGNAME_FORMAT_SAMPLE_A  "../log/FTS_SAMPLE_A_%Y-%m-%d_%H%M%S.log"
#define LOGNAME_FORMAT_SAMPLE_B  "../log/FTS_SAMPLE_B_%Y-%m-%d_%H%M%S.log"

/**Set default settings*/
 #define DEFAULT_FORCE "196"
 #define DEFAULT_SAMPLES "1"
 #define DEFAULT_CYCLES "2"
 #define DEFAULT_UP_STEP "500"
 #define DEFAULT_DOWN_STEP "500"
 #define DEFAULT_ROTATING_PLATES "0"

/**Set constants*/
#define LOAD_CELL_CONST 0.0030019 // V/V/#
#define ADC_MAX_V 1.8 //Volts
#define GAIN_LOADCELL 58.55 //gain of op amp
#define V_OUT_MAX 3.3 //volts
#define MAX_PSI 120.0 //PSI
#define MIN_PSI 3.0 //PSI
#define PWM_GAIN 3 //(1 + R2/R1)
#define MAX_V_INPUT_PRESSREG 10 //volts
#define RESOLUTION_ADC 1800 //4096 //the counts in 12 bit ADC 0 - 4095
#define X_INTERCEPT_LOAD_CELL_1 .0020671 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define Y_INTERCEPT_LOAD_CELL_1 17.3 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define X_INTERCEPT_LOAD_CELL_2 .0020644 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define Y_INTERCEPT_LOAD_CELL_2 20.6 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define AREA_FOOT_SENSOR 4.201 //brett found this number in in^2
#define GAIN_TOE_HEEL 2
#define R_TOE_HEEL 10000 //10K
#define FOOT_SENSOR_INTERNAL_RES 470
#define MAX_SAMPLE 2 //max number of samples
#define MAX_BUF 64     //This is plenty large

#define FIVE_V_INPUT 4.8
#define R2_TOE 322
#define R2_HEEL 2298

typedef struct data_t{
	uint16_t sampleNum, currentSubState, nextSubState;
	uint32_t dataCount, lotNum, serialNum;
	long double desiredForce, measuredForce, heelVal, toeVal, baseForce;
	bool bLogCreated, bCurrentSensorContact, bNextSensorContact;
	int8_t fileName[LOGNAME_SIZE];
}data_t;

typedef struct test_param_t{
	uint16_t numSAMPLE, command;
	int16_t currentState, nextState, FORCE_PROF, stateBeforePause;
	uint32_t upStepTime_ms, downStepTime_ms, count, cycle;
	uint64_t stepTime_ms, elapsedTime_ms, currentTime_ms, logTime_ms, rotateDelay;
	float desiredForce;
	bool bUpFlag, bDownFlag, bTurnFlag, bCommandFlag, bLogTrue, bCleanTest;
}test_param_t;

typedef struct SPid{
  float prevVel; //previous velocity term for D-term
  float prevPos;	// Last position input
  float prevI;		// previous integrator state (accumulate over time)
  float iMax, iMin; // Maximum and minimum allowable integrator state
  float	iGain,    	// integral gain
  	  	pGain,    	// proportional gain
        dGain;     	// derivative gain
} SPid;

typedef enum {
	verticalValve,
	turnValve_A,
	turnValve_B
}valveDefine;

typedef enum {
	sample_A,
	sample_B
}sampleNumber;

typedef enum {
	toe,
	heel
}toeHeelID;

typedef enum {
	init,
	downStep,
	upStep,
	hold,
	quit,
	reset
}state;

typedef enum {
	sensorContact,
	noSensorContact
}subState;

void initTest();
void initValve();
void initCLI(text_responses *text_obj);
void openValve(int16_t valveDefine);
void closeValve(int16_t valveDefine);
float getLoadCell(int16_t sampleNum);
double UpdatePID(SPid * pid, double error, double position);
void enableLoadCellADC();
long double getFootVal(int16_t sampleNum, int toeHeel);
void enableFootADC();
void setDesPSI(float psi);
void turnOffPressureReg();
void initADC();
uint64_t getTimestamp_ms();
uint64_t elapsedStepTime(test_param_t *pParam);
void logData();
const char * getStateEnum(int16_t state);
void cleanTest(text_responses *text_obj);
uint64_t paramu64(char *selection, char *setGet, uint64_t value);
uint32_t paramu32(char *selection, char *setGet, uint32_t value);
bool paramBool (char *selection, char * setGet, bool value);
uint16_t paramu16(char *selection, char *setGet, uint16_t value);
void getTimersPrintStates();
int16_t param16(char *selection, char * setGet, int16_t value);
bool dataTData(char *selection, char *setGet, int16_t sample, bool value);
void analyzeContact(int16_t sample);
void freeEverything();
void runPID(int16_t sample);
void delay(int milliseconds);
int readADC(unsigned int pin);


#endif
