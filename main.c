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
 *  @file main.c
 *  @brief Main module to control foot test stand and allow for user control through command line input
 *
 *
 *  $Revision: $1.0
 *  $Date: $04/11/2016
 *
 */


#include "main.h"
#include "CLI.c"
#include "state.c"


//initialize structs
struct data_t;
struct test_param_t;
struct SPid;

//Initialize buffer for ADCs
unsigned int buffer_LOAD_CELL_1[BUFFER_SIZE] = {0};
unsigned int buffer_LOAD_CELL_2[BUFFER_SIZE] = {0};
unsigned int buffer_HEEL_1_ADC[BUFFER_SIZE] = {0};
unsigned int buffer_TOE_1_ADC[BUFFER_SIZE] = {0};
unsigned int buffer_HEEL_2_ADC[BUFFER_SIZE] = {0};
unsigned int buffer_TOE_2_ADC[BUFFER_SIZE] = {0};

//Global and local variables
data_t *pSamples[MAX_SAMPLE]; //create an array of pointer samples
test_param_t *pParam;
SPid *pid;

//Initialze file name buffers
FILE* outfile[MAX_SAMPLE];
int8_t filename_1[LOGNAME_SIZE];

/** ADC clock variables
 * Derived based on BBBio documentation
 * Clock Divider : 3200 ,  Open Dly : 0 ,  Sample Average : 1 ,    Sample Dly : 1
   */
const int16_t clk_div = 3200;
const int16_t open_dly = 0;
const int16_t sample_dly = 1;

/**@brief Main control loop.
 *
 *	Call state machine and command test stand based on current and next states *
 *
 */
int main(int argc, char *argv[])
{

	iolib_init(); 				//initiate GPIO library
	initValve();   				//initiate valve pins
	initADC();    				//set up ADCs
	enableLoadCellADC(); 	 	//enable load cell pins
	enableFootADC();  			//enable foot adc pins

	cleanTest();				//initiate a clean test

	while(pParam->count <= pParam->cycle)
	{
		//set timers
		pParam->currentTime_ms = getTimestamp_ms();
		pParam->elapsedTime_ms = elapsedStepTime(pParam);

		//check for command flags from user
		/*
		pParam->bCommandFlag = scan_args();
		if(pParam->bCommandFlag)
		{
			pParam->command = returnCommand();
			printf("Command flag caught\n");
		}
		*/

		//access state machine, get next state
		pParam->nextState = stateMachine(pParam);

		printf("Count: %d         Current State: %s \n"
				"Next State: %s    Elapsed Time: %lld\n"
				"--------------------------------------\n",
				pParam->count,  getStateEnum(pParam->currentState),
				getStateEnum(pParam->nextState), pParam->elapsedTime_ms);

		switch(pParam->currentState)
		{
			case init:
				initTest();
				break;
			case downStep:
				if(pParam->currentState != pParam->nextState )
				{
					if(pParam->nextState == upStep)
					{
						openValve(verticalValve);
						pParam->stepTime_ms = getTimestamp_ms();
						pParam->bDownFlag = true;
					}
					else if(pParam->nextState == hold || pParam->nextState == reset || pParam->nextState == quit)
					{
						//open all valves and hold in safety position during hold, reset, or quit
						openValve(verticalValve);  //open vertical valves
						pParam->bLogTrue = false;  //don't log during interlude
						if(pParam->bTurnFlag)  //if turnflag on
						{
							openValve(turnValve_A);  //open sample A turn plate valves
							if(pParam->numSAMPLE == MAX_SAMPLE)
							{
								openValve(turnValve_B); //if there are two samples, open both valves
							}
						}
					}
					else
					{
						printf("Invalid state change\n");
					}

				}
				break;
			case upStep:
				if(pParam->currentState != pParam->nextState)
				{
					if(pParam->nextState == downStep)
					{
						closeValve(verticalValve);
						pParam->stepTime_ms = getTimestamp_ms();
						pParam->bUpFlag = true;
					}
					else if(pParam->nextState == hold || pParam->nextState == reset || pParam->nextState == quit)
					{
						openValve(verticalValve);
						pParam->bLogTrue = false;
						if(pParam->bTurnFlag)  //if turnflag on
						{
							openValve(turnValve_A);  //open sample A turn plate valves
							if(pParam->numSAMPLE == MAX_SAMPLE)
							{
								openValve(turnValve_B); //if there are two samples, open both valves
							}
						}
					}
					else
					{
						printf("Invalid state change\n");
					}
				}
				break;
			case hold:
				if(pParam->currentState != pParam->nextState)
				{
					if(pParam->nextState == downStep)
					{
						initTest();
						closeValve(verticalValve);
					}
					else if(pParam->nextState == init)
					{
						printf("Restarting test\n");
						sleep(3);
					}
					else if(pParam->nextState == reset || pParam->nextState == quit)
					{

					}
					else
					{
						printf("Invalid state change\n");
					}
				}
				break;
			case reset:
				if(pParam->currentState != pParam->nextState)
				{
					if(pParam->nextState == init)
					{
						cleanTest();
					}
				}
				break;
			case quit:
				for(int16_t numS = sample_A; numS < MAX_SAMPLE; numS++)
				{
					fclose(outfile[numS]);
				}
			default:
				break;
		}

		//check sample substate, see if contact of sensor is made
		for (int16_t sample = sample_A; sample < pParam->numSAMPLE; sample++)
		{
			pSamples[sample]->bNextSensorContact = bSubStateMachine(pSamples[sample]);
			if(pSamples[sample]->bNextSensorContact != pSamples[sample]->bCurrentSensorContact)
			{
				if(pParam->bTurnFlag)
				{
					if(!pSamples[sample]->bNextSensorContact)
					{
						openValve(sample);
					}
					else
					{
						closeValve(sample);
					}
				}
			}
			//update sample values
			pSamples[sample]->bCurrentSensorContact = pSamples[sample]->bNextSensorContact;
			pSamples[sample]->measuredForce = getLoadCell(sample);

		}
		//check if full cycle completed
		if(pParam->bUpFlag && pParam->bDownFlag)
		{
			printf("Cycle complete\n");
			pParam->count += 1;
			pParam->bUpFlag = false;
			pParam->bDownFlag = false;
		}
		//log data
		logData();
		if(pParam->currentState == quit)
		{

		}
		//set up for next loop
		pParam->currentState = pParam->nextState;


	}
	iolib_free();
	exit(EXIT_SUCCESS);
}

/*--------------------------------------------------------------------------------*/
/**@brief init test to zero settings
 *
 *
 */
void initTest()
{
	pParam->logTime_ms = getTimestamp_ms();
	pParam->stepTime_ms = getTimestamp_ms();
	pParam->bLogTrue = true;
	pParam->bUpFlag = false;
	pParam->bDownFlag = false;
	for(int16_t numS = sample_A; numS < MAX_SAMPLE; numS ++)
	{
		pSamples[numS]->bCurrentSensorContact = bSubStateMachine(pSamples[numS]);
	}
}
/** @brief if user wants to restart test from beginning
 *
 *reset count, turn flags, log flags, stops current log and closes
 */
void cleanTest()
{
	for(int16_t numS = sample_A; numS < MAX_SAMPLE; numS ++)
	{
		pSamples[numS] = (struct data_t*) malloc(sizeof(struct data_t));
	}
	pParam = (struct test_param_t*) malloc(sizeof(struct test_param_t));
	pid = (struct SPid*) malloc(sizeof(struct SPid));
	initCLI();

	openValve(verticalValve); 	//open linear cylinder valve
	if(pParam->bTurnFlag)
	{
		openValve(turnValve_A);  //open sample A turn plate valves
		if(pParam->numSAMPLE == MAX_SAMPLE)
		{
				openValve(turnValve_B); //if there are two samples, open both valves
		}
	}

	setDesForce(pParam->desiredForce);
	for(int16_t sample = sample_A; sample < MAX_SAMPLE; sample++)
	{
		pSamples[sample]->baseForce = getLoadCell(sample);
	}

	pParam->count = 0;
	pParam->FORCE_PROF = 0;
	pParam->bTurnFlag = true;
	pParam->bLogTrue = true;
}
/**@brief initate GPIO pins of valves
 *
 */
void initValve()
{
	iolib_setdir(HEADER_P8, SOL_VALVE_1, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_2, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_3, BBBIO_DIR_OUT);
	printf("Valves initiated\n");
}

/** @brief Return raw value of desired load cell
 *
 *
 * @param sampleNum      input desired load cell number to sample from
 *
 * @return return float of load cell raw value.
 */
float getLoadCell(int16_t sampleNum)
{
	float force;
	uint16_t sample;
	BBBIO_ADCTSC_work(SAMPLE_SIZE);
	if(sampleNum == sample_A)
	{
	sample = buffer_LOAD_CELL_1[1];
	force = (float)GAIN_LOADCELL* ADC_MAX_V * sample;
	}
	else if (sampleNum == sample_B)
	{
	sample = buffer_LOAD_CELL_2[1];
	force = (float)GAIN_LOADCELL* ADC_MAX_V * sample;
	}

    return(force);
}

/**@brief enable load cell ADC channels using BBBioLib
 *
 *
 */
void enableLoadCellADC()
{
	BBBIO_ADCTSC_channel_enable(LOAD_CELL_1);
	BBBIO_ADCTSC_channel_enable(LOAD_CELL_2);

	printf("Load cell channels enabled\n");
}

/**Getting the foot value resistance from ADC value
 *@param sampleNum input desired load cell number to sample from
 *@param toeHeel: either 0 for toe and 1 for heel to decipher heel/toe resistance
 */
float getFootVal(int16_t sampleNum, int toeHeel)
{
	float resistance;
	uint16_t sample;
	float constRes = (ADC_MAX_V / 2) / FIVE_V_INPUT;
	BBBIO_ADCTSC_work(SAMPLE_SIZE);
	if (sampleNum == sample_A) {
		if (toeHeel == toe) {
			sample = buffer_TOE_1_ADC[1];
			float voltageMeasured = (ADC_MAX_V * sample) / RESOLUTION_ADC;
			float actualVoltage = voltageMeasured / (float) GAIN_TOE_HEEL;
			float r1_resistance = (R2_TOE - constRes * R2_TOE) / constRes;
			resistance = r1_resistance - FOOT_SENSOR_INTERNAL_RES;
		} else if (toeHeel == heel) {
			sample = buffer_HEEL_1_ADC[1];
			float voltageMeasured = (ADC_MAX_V * sample) / RESOLUTION_ADC;
			float actualVoltage = voltageMeasured / (float) GAIN_TOE_HEEL;
			float r1_resistance = (R2_HEEL - constRes * R2_HEEL) / constRes;
			resistance = r1_resistance - FOOT_SENSOR_INTERNAL_RES;
		}
	} else if (sampleNum == sample_B) {
		if (toeHeel == toe) {
			sample = buffer_TOE_2_ADC[1];
			float voltageMeasured = (ADC_MAX_V * sample) / RESOLUTION_ADC;
			float actualVoltage = voltageMeasured / (float) GAIN_TOE_HEEL;
			float r1_resistance = (R2_TOE - constRes * R2_TOE) / constRes;
			resistance = r1_resistance - FOOT_SENSOR_INTERNAL_RES;
		} else if (toeHeel == heel) {
			sample = buffer_HEEL_2_ADC[1];
			float voltageMeasured = (ADC_MAX_V * sample) / RESOLUTION_ADC;
			float actualVoltage = voltageMeasured / (float) GAIN_TOE_HEEL;
			float r1_resistance = (R2_HEEL - constRes * R2_HEEL) / constRes;
			resistance = r1_resistance - FOOT_SENSOR_INTERNAL_RES;
		}
	}
	return(resistance);

}
/**@brief enable foot sensor ADC channels using BBBioLib
 *
 *
 */
void enableFootADC()
{
	BBBIO_ADCTSC_channel_enable(HEEL_1_ADC);
	BBBIO_ADCTSC_channel_enable(HEEL_2_ADC);
	BBBIO_ADCTSC_channel_enable(TOE_1_ADC);
	BBBIO_ADCTSC_channel_enable(TOE_2_ADC);
	printf("Foot ADC channel enabled\n");
}

/**@brief Open desired solenoid valve by setting GPIO pins to low using BBBIOlib
 *
 *
 * @param valveDefine      input desired solenoid valve number to open
 *
 *
 */
void openValve(int16_t valveDefine)
{
	switch(valveDefine)
	{
	case verticalValve:
		pin_low(HEADER_P8,SOL_VALVE_1);
		break;
	case turnValve_A:
		pin_low(HEADER_P8,SOL_VALVE_2);
		break;
	case turnValve_B:
		pin_low(HEADER_P8,SOL_VALVE_3);
		break;
	default:
		printf("Valve number not valid\n");
		break;
	}
}
/**@brief Close desired solenoid valve by setting GPIO pins to high using BBBIOlib
 *
 *
 * @param valveDefine      input desired solenoid valve number to close
 *
 *
 */
void closeValve(int16_t valveDefine)
{
	switch(valveDefine)
	{
	case verticalValve:
		pin_high(HEADER_P8,SOL_VALVE_1);
		break;
	case turnValve_A:
		pin_high(HEADER_P8,SOL_VALVE_2);
		break;
	case turnValve_B:
		pin_high(HEADER_P8,SOL_VALVE_3);
		break;
	default:
		printf("Valve number not valid\n");
		break;
	}
}

/**
 * @brief set up adc channels and buffers using BBBioLib
 *
 *
 */
void initADC()
{
	BBBIO_ADCTSC_module_ctrl(BBBIO_ADC_WORK_MODE_TIMER_INT, clk_div);
	BBBIO_ADCTSC_channel_ctrl(LOAD_CELL_1, BBBIO_ADC_STEP_MODE_SW_CONTINUOUS, open_dly, sample_dly, BBBIO_ADC_STEP_AVG_1, buffer_LOAD_CELL_1, BUFFER_SIZE);
	BBBIO_ADCTSC_channel_ctrl(LOAD_CELL_2, BBBIO_ADC_STEP_MODE_SW_CONTINUOUS, open_dly, sample_dly, BBBIO_ADC_STEP_AVG_1, buffer_LOAD_CELL_2, BUFFER_SIZE);
	BBBIO_ADCTSC_channel_ctrl(HEEL_1_ADC, BBBIO_ADC_STEP_MODE_SW_CONTINUOUS, open_dly, sample_dly,  BBBIO_ADC_STEP_AVG_1, buffer_HEEL_1_ADC, BUFFER_SIZE);
	BBBIO_ADCTSC_channel_ctrl(TOE_1_ADC, BBBIO_ADC_STEP_MODE_SW_CONTINUOUS, open_dly, sample_dly,  BBBIO_ADC_STEP_AVG_1, buffer_TOE_1_ADC, BUFFER_SIZE);
	BBBIO_ADCTSC_channel_ctrl(HEEL_2_ADC, BBBIO_ADC_STEP_MODE_SW_CONTINUOUS, open_dly, sample_dly,  BBBIO_ADC_STEP_AVG_1, buffer_HEEL_2_ADC, BUFFER_SIZE);
	BBBIO_ADCTSC_channel_ctrl(TOE_2_ADC, BBBIO_ADC_STEP_MODE_SW_CONTINUOUS, open_dly, sample_dly,  BBBIO_ADC_STEP_AVG_1, buffer_TOE_2_ADC, BUFFER_SIZE);
	BBBIO_ADCTSC_work(SAMPLE_SIZE);

	printf("ADC init\n");

}
/** @brief Set desired force to pressure regulator.
 *
 *	In future have PID control and cases for diff force profiles, set by user
 *	Uses PWM signals to control input voltage
 *
 * @param f      input desired force set by user
 *
 *
 */
void setDesForce(float force)
{
   	float duty;
   	duty = (MAX_V_INPUT_PRESSREG*(force - MIN_PSI))/(V_OUT_MAX*PWM_GAIN*(MAX_PSI-MIN_PSI));
	BBBIO_PWMSS_Setting(BBBIO_PWMSS1, PWM_HZ, duty, duty);
	BBBIO_ehrPWM_Enable(BBBIO_PWMSS1);
}
/**
 * @brief Disable PWM for pressure reg using BBBioLib
 *
 *
 */
void turnOffPressureReg()
{
	BBBIO_ehrPWM_Disable(BBBIO_PWMSS1);
}

/**
 * @brief Return raw value of current timestamp in milliseconds
 *
 *
 *
 * @return return long long int of current time in msec
 */
uint64_t getTimestamp_ms()
{
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    uint64_t milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
}

/**
 * @brief calculate elapsed time based on
 *
 * @param i desired load cell to sample from
 *
 *@return return true if contact made, false if no contact
 */
uint64_t elapsedStepTime(test_param_t *pParam_t)
{
	//printf("The current time is %s and the step time is %d\n", pParam_t->currentTime_ms, pParam_t->stepTime_ms);
	return pParam_t->currentTime_ms - pParam_t->stepTime_ms;
}
/**
 * @brief Log data in a format readable by CSV
 *
 *  Check flags to log data, if log hasn't been created print out headers first.
 *
 *
 */
void logData()
{
	for (int16_t sample = sample_A; sample < pParam->numSAMPLE; sample++)
	{
		if (pParam->bLogTrue)
		{
				if (pParam->currentTime_ms - pParam->logTime_ms >= LOG_TIMER_MS)
				{
					if(!pSamples[sample]->bLogCreated)
					{
						fprintf(outfile[sample],
							"Serial_number: %d \n"
							"Time "
							"Data_Count "
							"Cycle "
							"State "
							"Desired_force_(psi) "
							"Measured_force_(psi) "
							"Toe_sensor "
							"Heel_Sensor\n", pSamples[sample]->serialNum);

						pSamples[sample]->bLogCreated = true;
					}
		//
				fprintf(outfile[sample],
					"%lld "
					"%d "
					"%d "
					"%d "
					"%.3f "
					"%.3f "
					"%.2f "
					"%.2f\n",
					pParam->currentTime_ms,
					pSamples[sample]->dataCount ,
					pParam->count,
					pParam->currentState,
					pSamples[sample]->desiredForce,
					pSamples[sample]->measuredForce,
					pSamples[sample]->toeVal,
					pSamples[sample]->heelVal);

				pSamples[sample]->dataCount += 1;
			}
		}
	}

}

const char * getStateEnum(int16_t state)
{
	//const char *x = "abcstring";
	//return x
	switch (state)
	{
	case init: ;
		char *a = "Init";
		return a;
	case downStep: ;
		char *b = "Down Step";
		return b;
	case upStep: ;
		char *c = "Up Step";
		return c;
	case hold: ;
		char *d = "Hold";
		return d;
	case reset: ;
		char *e = "Reset";
		return e;
	case quit: ;
		char *f = "Quit";
		return f;
	}
}

/**
 * @brief initiliw
 *
 *  Check flags to log data, if log hasn't been created print out headers first.
 *
 *
 */
void initCLI()
{
	int8_t input;
	int16_t temp_input;
	float temp_float;
	int8_t temp_char;
	bool bValidInput = false;
	bool bValidInput_ = false;
	while(!bValidInput_)
	{
		printf("Default settings? Y or N: ");
		scanf(" %c", &input);
		printf("\n");
		if (input == 'Y' || input == 'y')
		{
			bValidInput_ = true;
			//set default settings
			pParam->desiredForce = DEFAULT_DES_FORCE;
			pParam->upStepTime_ms = DEFAULT_UP_STEP;
			pParam->downStepTime_ms = DEFAULT_DOWN_STEP;
			pParam->cycle = DEFAULT_DES_STEPS;
			pParam->numSAMPLE = DEFAULT_NUM_SAMPLES;
			pParam->currentState = init;
			pParam->bLogTrue = true;
			pParam->bTurnFlag = DEFAULT_TURN_PLATES;

			pSamples[sample_A]->sampleNum = sample_A + 1;
			pSamples[sample_A]->desiredForce = pParam->desiredForce;
			pSamples[sample_A]->dataCount = 0;
			pSamples[sample_A]->bLogCreated = false;
			pSamples[sample_A]->currentSubState = noSensorContact;

			time_t now = time(0);
			strftime(filename_1, sizeof(filename_1), LOGNAME_FORMAT_SAMPLE_A, localtime(&now));
			outfile[sample_A] = fopen(filename_1, "w");
			strcpy(pSamples[sample_A]->fileName, filename_1);
			printf("Saving sample #1 data to file: ");
			printf(filename_1);
			printf("\n");
		}
		//Have user input the desired
		else if(input == 'N' || input == 'n')
		{
			bValidInput_ = true;
			bValidInput = false;
			while(!bValidInput)
			{
				printf("Input number of samples (1 or 2): ");
				scanf(" %d", &temp_input);
				printf("\n");
				if(temp_input <= MAX_SAMPLE && temp_input > 0)
				{
					pParam->numSAMPLE = temp_input;
					bValidInput = true;
				}
				else
				{
					printf("Invalid number of samples, please re-input\n");
				}
			}

			bValidInput = false;
			while(!bValidInput)
			{
				printf("Input desired force in PSI (3.0-120.0 psi): ");
				scanf(" %f", &temp_float);
				printf("\n");
				if(temp_float <= MAX_PSI && temp_float >= MIN_PSI)
				{
					pParam->desiredForce = temp_float;
					bValidInput = true;

				}
				else
				{
					printf("Invalid desired PSI, please re-input\n");
				}
			}

			bValidInput = false;
			while(!bValidInput)
			{
				printf("Input up step in milliseconds: ");
				scanf(" %d", &temp_input);
				printf("\n");
				if(temp_input > 99 && temp_input <= 10000)
				{
					pParam->upStepTime_ms = temp_input;
					bValidInput = true;
				}
				else
				{
					printf("Invalid desired up step time, out of range. Please re-input\n");
				}
			}

			bValidInput = false;
			while(!bValidInput)
			{
				printf("Input down step in milliseconds: ");
				scanf(" %d", &temp_input);
				printf("\n");
				if(temp_input > 99 && temp_input <= 10000)
				{
					pParam->downStepTime_ms = temp_input;
					bValidInput = true;
				}
				else
				{
					printf("Invalid desired down step time, out of range. Please re-input\n");
				}
			}

			bValidInput = false;
			while(!bValidInput)
			{
				printf("Input number of cycles: ");
				scanf(" %d", &temp_input);
				printf("\n");
				if(temp_input > 0)
				{
					pParam->cycle = temp_input;
					bValidInput = true;
				}
				else
				{
					printf("Invalid negative cycle number. Please re-input\n");
				}
			}

			bValidInput = false;
			while(!bValidInput)
			{
				printf("Y or N, turn on rotating plates: ");
				scanf(" %c", &temp_char);
				printf("\n");

				if(temp_char == 'Y' || temp_char == 'y')
				{
					pParam->bTurnFlag = true;
					bValidInput = true;
				}
				else if(temp_char == 'N' || temp_char == 'n')
				{
					pParam->bTurnFlag = false;
					bValidInput = true;
				}
				else
				{
					printf("Invalid entry.\n");
				}
			}
			pParam->currentState = init;
			pParam->bCleanTest = true;

			for (int16_t sample = sample_A; sample < pParam->numSAMPLE; sample++)
			{
				pSamples[sample]->sampleNum = sample+ 1;
				pSamples[sample]->desiredForce = pParam->desiredForce;
				pSamples[sample]->dataCount = 0;
				pSamples[sample]->bLogCreated = false;
				pSamples[sample]->currentSubState = noSensorContact;

				time_t now = time(0);
				if(sample == sample_A)
				{
					strftime(filename_1, sizeof(filename_1), LOGNAME_FORMAT_SAMPLE_A, localtime(&now));
				}
				else
				{
					strftime(filename_1, sizeof(filename_1), LOGNAME_FORMAT_SAMPLE_B, localtime(&now));
				}
				outfile[sample] = fopen(filename_1, "w");
				strcpy(pSamples[sample]->fileName, filename_1);
				printf("Saving sample #%d data to file: ", sample+1);
				printf(filename_1);
				printf("\n");

			}

		}
	}

	printf("*-----------------------------------------------------*");
	printf("Desired force: %f (psi)      Number of cycles: %d\n", pParam->desiredForce,  pParam->cycle);
	printf("Up step time: %d (ms)      Down Step time: %d (ms) \n",pParam->upStepTime_ms,pParam->downStepTime_ms);
	printf("Starting test in 3 seconds...\n");
	sleep(3);
}

