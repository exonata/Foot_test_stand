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
 *  @file newMain.c
 *  @brief GUI for controlling foot test stand and allow user input through GUI
 *
 *
 *  $Revision: $1.0
 *  $Date: $06/21/2016
 *
 */


#include "newmain.h"
#include "CLI.c"
#include "state.h"
#include "state.c"
#include "gui.h"


//initialize structs
struct data_t;
struct test_param_t;
struct SPid;


//Global and local variables
data_t *pSamples[MAX_SAMPLE]; //create an array of pointer samples
test_param_t *pParam;
SPid *pid;

//global variables for load cell offsets in volts
float offSetLC1 = 17.3; //for the offsets
float offSetLC2 = 20.6;

//Initialize buffer for ADCs
unsigned int buffer_LOAD_CELL_1[BUFFER_SIZE] = {0};
unsigned int buffer_LOAD_CELL_2[BUFFER_SIZE] = {0};
unsigned int buffer_HEEL_1_ADC[BUFFER_SIZE] = {0};
unsigned int buffer_TOE_1_ADC[BUFFER_SIZE] = {0};
unsigned int buffer_HEEL_2_ADC[BUFFER_SIZE] = {0};
unsigned int buffer_TOE_2_ADC[BUFFER_SIZE] = {0};


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
		pSamples[numS]->bCurrentSensorContact = bSensorContact(pSamples[numS]);
	}
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

/**@brief initate GPIO pins of valves
 *
 */
void initValve()
{
	iolib_setdir(HEADER_P8, SOL_VALVE_1, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_2, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_3, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_4, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_5, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_6, BBBIO_DIR_OUT);
	printf("Valves initiated\n");
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

const char * getStateEnum(int16_t state)
{
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

/** @brief Return raw value of desired load cell
 *
 *
 * @param sampleNum      input desired load cell number to sample from
 *
 * @return return float of load cell raw value.
 */

int updateVals(int8_t sample)
{
	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	alarm(10000000);
	
	pSamples[sample]->measuredForce = getLoadCell(sample);
	pSamples[sample]->toeVal =  getFootVal(sample, toe);
	pSamples[sample]->heelVal =  getFootVal(sample, heel);
	signal(SIGALRM, SIG_IGN);
	
	return 1;
	
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
	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	alarm(10000000);
	float force;
	unsigned int sample;
	BBBIO_ADCTSC_work(SAMPLE_SIZE);
	if (sampleNum == sample_A) {
		sample = buffer_LOAD_CELL_1[0];
		//sample =  readADC(LOAD_CELL_1);
		signal(SIGALRM, SIG_IGN);
		float actualVoltage = (ADC_MAX_V * sample) / RESOLUTION_ADC;
		force = (actualVoltage - offSetLC1) / X_INTERCEPT_LOAD_CELL_1;
		signal(SIGALRM, SIG_IGN);
		//printf("Measurement LC1 is %d, %f\n", sample, force);
	} else if (sampleNum == sample_B) {
		sample = buffer_LOAD_CELL_2[0];
		//sample =  readADC(LOAD_CELL_2);
		signal(SIGALRM, SIG_IGN);
		float actualVoltage = (ADC_MAX_V * sample) / RESOLUTION_ADC;
		force = (actualVoltage - offSetLC2) / X_INTERCEPT_LOAD_CELL_2;
		signal(SIGALRM, SIG_IGN);
		//printf("Measurement LC2 is %d, %f\n", sample, force);
	}

	return(force);
}

//returns the desired force in psi for pressure regulator
double UpdatePID(SPid * pid, double error, double position) {
	double dt = 2.0; //in seconds need to TO  DO: create a timer that tracks
	//time between calls of PID
	double pTerm, dTerm, iTerm;

	// calculate the proportional term
	pTerm = pid->pGain * error;

	// calculate the integral state with appropriate limiting
  	pid->prevI += error;
  	if (pid->prevI > pid->iMax) pid->prevI = pid->iMax;
	else if (pid->prevI < pid->iMin) pid->prevI = pid->iMin;
  	iTerm = pid->iGain * pid->prevI;  // calculate the integral term

  	//calculate the derivative state
  	dTerm = pid->dGain * ((position - pid->prevPos)/(dt - pid->prevVel));
  	pid->prevVel = (position - pid->prevPos)/dt;
  	pid->prevPos = position;

  	//add all together: TO DO does D-term need to be subtracted?
  	float forceSum = pTerm + iTerm - dTerm; //will be in unit pounds
  	return forceSum / AREA_FOOT_SENSOR; //to convert to psi for pressure regulator
}


/**Getting the foot value resistance from ADC value
 *@param sampleNum input desired load cell number to sample from
 *@param toeHeel: either 0 for toe and 1 for heel to decipher heel/toe resistance
 *Under the assumption we are only operating resistance 1k-10k toe and 10k-60k heel
 *Simply a voltage divider circuit with output, with R2 = 10K
 */
float getFootVal(int16_t sampleNum, int toeHeel)
{
	printf("got into foot val\n");
	float resistance;
	unsigned int sample;
	BBBIO_ADCTSC_work(SAMPLE_SIZE);
	if (sampleNum == sample_A) {
		if (toeHeel == toe) {
			sample = buffer_TOE_1_ADC[0];
			//sample =  readADC(TOE_1_ADC);
			float voltageMeasured = (ADC_MAX_V * sample) / RESOLUTION_ADC;
			//printf("voltage measured toe 1: %f\n", voltageMeasured);
			float r1_resistance = (ADC_MAX_V*R_TOE_HEEL - voltageMeasured * R_TOE_HEEL) / voltageMeasured;
			resistance = r1_resistance - FOOT_SENSOR_INTERNAL_RES;
		} else if (toeHeel == heel) {
			sample = buffer_HEEL_1_ADC[0];
			//sample = readADC(HEEL_1_ADC);
			float voltageMeasured = (ADC_MAX_V * sample) / RESOLUTION_ADC;
			//printf("voltage measured heel 1: %f\n", voltageMeasured);
			float r1_resistance = (ADC_MAX_V*R_TOE_HEEL - voltageMeasured * R_TOE_HEEL) / voltageMeasured;
			resistance = r1_resistance - FOOT_SENSOR_INTERNAL_RES;
		}
		else
		{
			printf("Error: toe or heel val not requested\n");
		}
	} 
	else if (sampleNum == sample_B) 
	{
		if (toeHeel == toe) {
			sample = buffer_TOE_2_ADC[0];
			//sample = readADC(TOE_2_ADC);
			float voltageMeasured = (ADC_MAX_V * sample) / RESOLUTION_ADC;
			//printf("voltage measured toe 2: %f\n", voltageMeasured);
			float r1_resistance = (ADC_MAX_V*R_TOE_HEEL - voltageMeasured * R_TOE_HEEL) / voltageMeasured;
			resistance = r1_resistance - FOOT_SENSOR_INTERNAL_RES;
		} else if (toeHeel == heel) {
			sample = buffer_HEEL_2_ADC[0];
			//sample = readADC(HEEL_2_ADC);
			float voltageMeasured = (ADC_MAX_V * sample) / RESOLUTION_ADC;
			//printf("voltage measured heel 1: %f\n", voltageMeasured);
			float r1_resistance = (ADC_MAX_V*R_TOE_HEEL - voltageMeasured * R_TOE_HEEL) / voltageMeasured;
			resistance = r1_resistance - FOOT_SENSOR_INTERNAL_RES;
		}
		else
		{
			printf("Error: toe or heel val not requested\n");
		}
	}
	else
	{
		printf("Error: Sample A or B not requested\n");
	}
	return(resistance);
}


/**@brief Open desired solenoid valve by setting GPIO pins to high/low using BBIOlib
 * To open the valve we define in software that for vertical valves:
 *solenoid 1 and solenoid 3 have to be high, 2 and 4 low
 * @param valveDefine      input desired solenoid valve number to open
 *
 *
 */
void openValve(int16_t valveDefine)
{
	switch(valveDefine)
	{
	case verticalValve:
		pin_high(HEADER_P8,SOL_VALVE_1); //excite valve 1
		pin_low(HEADER_P8,SOL_VALVE_2); //unexcite valve 2
		if(pParam->numSAMPLE + 1 == MAX_SAMPLE)
		{
			pin_high(HEADER_P8,SOL_VALVE_3); //excite valve 3
			pin_low(HEADER_P8,SOL_VALVE_4); //unexcite valve 4
		}		
		break;
	case turnValve_A:
		pin_low(HEADER_P8,SOL_VALVE_5); //rotate valve 5
		break;
	case turnValve_B:
		pin_low(HEADER_P8,SOL_VALVE_6); //rotate valve 6
		break;
	default:
		printf("Valve number not valid\n");
		break;
	}
}
/**@brief Close desired solenoid valve by setting GPIO pins to high/low using BBBIOlib
 *To close the valve we define in software that for the vertical valves:
 *solenoid 1 and solenoid 3 have to be low with solenoid 2 and 4 being high
 * @param valveDefine      input desired solenoid valve number to close
 *
 *
 */
void closeValve(int16_t valveDefine)
{
	switch(valveDefine)
	{
	case verticalValve:
		pin_low(HEADER_P8,SOL_VALVE_1); //unexcite valve 1
		pin_high(HEADER_P8,SOL_VALVE_2); //excite valve 2
		if(pParam->numSAMPLE + 1 == MAX_SAMPLE)
		{
			pin_low(HEADER_P8,SOL_VALVE_3); //unexcite valve 3
			pin_high(HEADER_P8,SOL_VALVE_4); //excite valve 4
		}
		break;
	case turnValve_A:
		pin_high(HEADER_P8,SOL_VALVE_5); //rotate valve 5
		break;
	case turnValve_B:
		pin_high(HEADER_P8,SOL_VALVE_6); //rotate valve 6
		break;
	default:
		printf("Valve number not valid\n");
		break;
	}
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
	BBBIO_PWMSS_Setting(BBBIO_PWMSS0, PWM_HZ, duty, duty);
	BBBIO_ehrPWM_Enable(BBBIO_PWMSS0);
}
/**
 * @brief Disable PWM for pressure reg using BBBioLib
 *
 *
 */
void turnOffPressureReg()
{
	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
}

//this is the method to run the PID, question is where to put this?
//takes in a sample number as a parameter
void runPID(int16_t sample) {
	printf("in pid loop\n");
	//pSamples[sample]->bCurrentSensorContact = pSamples[sample]->bNextSensorContact;
	pSamples[sample]->measuredForce = getLoadCell(sample);
	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	//printf("This is the load cell measurement: %f lbs.\n", pSamples[sample]->measuredForce);
	float error = pParam->desiredForce - pSamples[sample]->measuredForce;
	float psiVal = UpdatePID(pid, error, pSamples[sample]->measuredForce);
	setDesForce(psiVal);
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
						printf("In outfile segment1");
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
					
				//updateVals(sample);	
				printf("In outfile segment");
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

/**
 * @brief initiliw
 *
 *  Check flags to log data, if log hasn't been created print out headers first.
 *
 *
 */
void cleanTest(text_responses *text_obj) {
	for(int16_t numS = sample_A; numS < MAX_SAMPLE; numS ++)
		{
			pSamples[numS] = (struct data_t*) malloc(sizeof(struct data_t));
		}
	pParam = (struct test_param_t*) malloc(sizeof(struct test_param_t));
	pid = (struct SPid*) malloc(sizeof(struct SPid));

	initCLI(text_obj);

	//back to clean test code
	openValve(verticalValve); 	//open linear cylinder valve
	if(pParam->bTurnFlag) {
		openValve(turnValve_A);  //open sample A turn plate valves
		if (pParam->numSAMPLE == MAX_SAMPLE) {
			openValve(turnValve_B); //if there are two samples, open both valves
		}
	}
	float psiForce = pParam->desiredForce / AREA_FOOT_SENSOR; //need to convert pounds to psi for p regululator
	setDesForce(psiForce);
	for(int16_t sample = sample_A; sample < MAX_SAMPLE; sample++) {
		pSamples[sample]->baseForce = getLoadCell(sample);
	}

	pParam->count = 0;
	pParam->FORCE_PROF = 0;
	pParam->bTurnFlag = true;
	pParam->bLogTrue = true;
	pParam->stateBeforePause = init;
	pParam->bCommandFlag = false;

	//PID field inits
	pid->prevVel = 0.0;
	pid->prevPos = 0.0;
	pid->prevI = 0.0;
	pid->iGain = 0.0;
	pid->pGain = 0.1;
	pid->dGain = 0.0;
	pid->iMax = 580.0;
	pid->iMin = 15.0;

	//intialize offsets
	//get offsets for load cells in volts
	offSetLC1 = (ADC_MAX_V * getLoadCell(sample_A)) / RESOLUTION_ADC;
	offSetLC2 = (ADC_MAX_V * getLoadCell(sample_B)) / RESOLUTION_ADC;
}

//grab the data from the gui to input into our struct, also initializes the log
void initCLI(text_responses *text_obj) {
	//replaced original initCLI with this code
	const gchar *choice1 = gtk_entry_get_text ((GtkEntry *)text_obj->entry1);
	gchar *choice2 = gtk_combo_box_text_get_active_text ((GtkComboBoxText *)text_obj->entry2);
	const gchar *choice3 = gtk_entry_get_text ((GtkEntry *)text_obj->entry3);
	const gchar *choice4 = gtk_entry_get_text ((GtkEntry *)text_obj->entry4);
	const gchar *choice5 = gtk_entry_get_text ((GtkEntry *)text_obj->entry5);
	const gchar *choice6 = gtk_entry_get_text ((GtkEntry *)text_obj->entry6);
	gchar *choice7 = gtk_combo_box_text_get_active_text ((GtkComboBoxText *)text_obj->entry7);
	printf("Input Number of Samples 1 or 2: %s \n"
           "--------------------------------------\n"
           "Input Desired Force in PSI (127 - 500 lbs): %s \n"
           "--------------------------------------\n"
           "Input Up-Step in milliseconds: %s \n"
           "--------------------------------------\n"
           "Input Down-Step in milliseconds: %s \n"
           "--------------------------------------\n"
           "Input Number of Cycles: %s \n"
           "--------------------------------------\n"
           "Y or N on turning plates: %s \n"
           "--------------------------------------\n"
           "Rotating Plate delay (milliseconds): %s \n"
           "--------------------------------------\n"
           "                                       \n",
           choice2, choice3, choice4, choice5, choice6, choice7, choice1);
	int numSamples = atoll(choice2);
	float desiredForce = atof(choice3); //returns 0.0 if unsucessful
	int upStep = atoll(choice4); //0 if unsuccessful?
	int downStep = atoll(choice5);
	int numCycles = atoll(choice6);
	int turningPlates = atoll(choice7);
	int rotatingPlates = atoi(choice1); //milliseconds

	//assign the pParams
	pParam->desiredForce = desiredForce;
	pParam->upStepTime_ms = upStep;
	pParam->downStepTime_ms = downStep;
	pParam->cycle = numCycles;
	pParam->numSAMPLE = numSamples;
	pParam->bTurnFlag = turningPlates;
	pParam->rotateDelay = rotatingPlates;

	//mark the clean test as true and current State as init
	pParam->bCleanTest = true;
	pParam->currentState = init;
	time_t now = time(0);

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
		//printf("got hereeee\n");
		printf(filename_1);
		printf("\n");
	}

}

//gets/sets all the structs that are uint64_t type: returns -1 if we set
//and returns the value is we get, first param is the name of the field,
//the second is "Set" or "Get" depending on what we want to do, and the
//thirs field is what we want to set the field to if we choose set
//if you want to choose get then just put in "0" for the last field
uint64_t paramu64(char *selection, char *setGet, uint64_t value) {
	if (strcmp(setGet, "Set") == 0) {
		if (strcmp(selection, "stepTime_ms") == 0) {
			pParam-> stepTime_ms = value;
		} else if (strcmp(selection, "elapsedTime_ms") == 0) {
			pParam->elapsedTime_ms = value;
		} else if (strcmp(selection, "currentTime_ms") == 0) {
			pParam->currentTime_ms = value;
		} else if (strcmp(selection,"logTime_ms") == 0) {
			pParam->logTime_ms = value;
		}
		return -1;
	} else if (strcmp(setGet, "Get") == 0) {
		if (strcmp(selection, "stepTime_ms") == 0) {
			return pParam-> stepTime_ms;
		} else if (strcmp(selection, "elapsedTime_ms") == 0) {
			return pParam->elapsedTime_ms;
		} else if (strcmp(selection, "currentTime_ms") == 0) {
			return pParam->currentTime_ms;
		} else if (strcmp(selection,"logTime_ms") == 0) {
			return pParam->logTime_ms;
		}
		return -2; //if for some reason we didn't choose the correct one
	}
}

//gets/sets all the structs that are uint32_t type: returns -1 if we set
//and returns the value is we get, first param is the name of the field,
//the second is "Set" or "Get" depending on what we want to do, and the
//thirs field is what we want to set the field to if we choose set
//if you want to choose get then just put in "0" for the last field
uint32_t paramu32(char *selection, char *setGet, uint32_t value) {
	if (strcmp(setGet, "Set") == 0) {
		if (strcmp(selection, "upStepTime_ms") == 0) {
			pParam-> upStepTime_ms = value;
		} else if (strcmp(selection, "downStepTime_ms") == 0) {
			pParam->downStepTime_ms = value;
		} else if (strcmp(selection, "count") == 0) {
			pParam->count = value;
		} else if (strcmp(selection,"cycle") == 0) {
			pParam->cycle = value;
		}
		return -1;
	} else if (strcmp(setGet, "Get") == 0) {
		if (strcmp(selection, "upStepTime_ms") == 0) {
			return pParam-> upStepTime_ms;
		} else if (strcmp(selection, "downStepTime_ms") == 0) {
			return pParam->downStepTime_ms;
		} else if (strcmp(selection, "count") == 0) {
			return pParam->count;
		} else if (strcmp(selection,"cycle") == 0) {
			return pParam->cycle;
		}
		return -2; //if for some reason we didn't choose the correct one
	}

}

//gets/sets all the structs that are bool type: returns -1 if we set
//and returns the value is we get, first param is the name of the field,
//the second is "Set" or "Get" depending on what we want to do, and the
//thirs field is what we want to set the field to if we choose set
//if you want to choose get then just put in "false" for the last field
bool paramBool (char *selection, char * setGet, bool value) {
	if (strcmp(setGet, "Set") == 0) {
			if (strcmp(selection, "bUpFlag") == 0) {
				pParam-> bUpFlag = value;
			} else if (strcmp(selection, "bDownFlag") == 0) {
				pParam->bDownFlag = value;
			} else if (strcmp(selection, "bTurnFlag") == 0) {
				pParam->bTurnFlag = value;
			} else if (strcmp(selection,"bCommandFlag") == 0) {
				pParam->bCommandFlag = value;
			} else if (strcmp(selection,"bLogTrue") == 0) {
				pParam->bLogTrue = value;
			} else if (strcmp(selection,"bCleanTest") == 0) {
				pParam->bCleanTest = value;
			}
			return -1;
	} else if (strcmp(setGet, "Get") == 0) {
		if (strcmp(selection, "bUpFlag") == 0) {
			return pParam-> bUpFlag;
		} else if (strcmp(selection, "bDownFlag") == 0) {
			return pParam->bDownFlag;
		} else if (strcmp(selection, "bTurnFlag") == 0) {
			return pParam->bTurnFlag;
		} else if (strcmp(selection,"bCommandFlag") == 0) {
			return pParam->bCommandFlag;
		} else if (strcmp(selection,"bLogTrue") == 0) {
			return pParam->bLogTrue;
		} else if (strcmp(selection,"bCleanTest") == 0) {
			return pParam->bCleanTest;
		}
		return -2; //if for some reason we didn't choose the correct one
	}
}

int16_t param16(char *selection, char * setGet, int16_t value) {
	if (strcmp(setGet, "Set") == 0) {
		if (strcmp(selection, "currentState") == 0) {
			pParam-> currentState = value;
		} else if (strcmp(selection, "nextState") == 0) {
			pParam->nextState = value;
		} else if (strcmp(selection, "FORCE_PROF") == 0) {
			pParam->bTurnFlag = value;
		} else if (strcmp(selection, "stateBeforePause") == 0) {
			pParam->stateBeforePause = value;
		}
		return -1;
	} else if (strcmp(setGet, "Get") == 0) {
	if (strcmp(selection, "currentState") == 0) {
		return pParam->currentState;
	} else if (strcmp(selection, "nextState") == 0) {
		return pParam->nextState;
	} else if (strcmp(selection, "FORCE_PROF") == 0) {
		return pParam->FORCE_PROF;
	} else if (strcmp(selection, "stateBeforePause") == 0) {
		return pParam->stateBeforePause;
	}
	return -2; //if for some reason we didn't choose the correct one
	}
}


uint16_t paramu16(char *selection, char *setGet, uint16_t value) {
	if (strcmp(setGet, "Set") == 0) {
		if (strcmp(selection, "numSAMPLE") == 0) {
			pParam-> numSAMPLE = value;
		} else if (strcmp(selection, "command") == 0) {
			pParam->command = value;
		}
		return -1;
	} else if (strcmp(setGet, "Get") == 0) {
		if (strcmp(selection, "numSAMPLE") == 0) {
			return pParam-> numSAMPLE;
		} else if (strcmp(selection, "command") == 0) {
			return pParam->command;
		}
		return -2; //if for some reason we didn't choose the correct one
	}
}

bool dataTData(char *selection, char *setGet, int16_t sample, bool value) {
	if (strcmp(setGet, "Set") == 0) {
			if (strcmp(selection, "bCurrentSensorContact") == 0) {
				pSamples[sample]-> bCurrentSensorContact = value;
			} else if (strcmp(selection, "bNextSensorContact") == 0) {
				pSamples[sample]->bNextSensorContact = value;
			}
			return -1;
	} else if (strcmp(setGet, "Get") == 0) {
		if (strcmp(selection, "bCurrentSensorContact") == 0) {
			return pSamples[sample]-> bCurrentSensorContact;
		} else if (strcmp(selection, "bNextSensorContact") == 0) {
			return pSamples[sample]->bNextSensorContact;
		}
		return -2; //if for some reason we didn't choose the correct one
	}
}

//gets the current time, elapsed time, this state, next state, and prints out stats
void getTimersPrintStates() {
	pParam->currentTime_ms = getTimestamp_ms();
	pParam->elapsedTime_ms = elapsedStepTime(pParam);
	//access state machine, get next state
	int16_t next =  stateMachine(pParam);
	pParam->nextState = next;
	//pParam->bCommandFlag = false;
	/*printf("Count: %d         			Current State: %s \n"
			"Next State: %s    Elapsed Time: %lld\n"
			"Command flag: %d\n"
			"--------------------------------------\n",
			pParam->count,  getStateEnum(pParam->currentState),
			getStateEnum(pParam->nextState), pParam->elapsedTime_ms, pParam->bCommandFlag); */
}

//we have made sensor contact so time to set params, rotate if need be, and run the PID
void analyzeContact(int16_t sample) {
	pSamples[sample]->bNextSensorContact =bSensorContact(pSamples[sample]);
//KAS- dont' know if we need both sensorcontact variables. should be able to use substate for current
	//RS-It checks to see if it is changing state so that it knows to open with a delay or not/close valve or to just use the previous command to that valve
	if(pSamples[sample]->bNextSensorContact != pSamples[sample]->bCurrentSensorContact)
	{
		if(pParam->bTurnFlag)
		{
			if(!pSamples[sample]->bNextSensorContact)
			{
				printf("starting turning logic!\n");
				delay(pParam->rotateDelay); //delay before rotation
				openValve(sample);
			}
			else
			{
				delay(pParam->rotateDelay); //delay before rotation
				closeValve(sample);
			}
		}
	}
//KAS- pid should run when foot is in contact only
	//update force sensing with PID, and feed to pressure regulator
	
	if(pSamples[sample]->bCurrentSensorContact)
	{
		runPID(sample);
	}
	
	pSamples[sample]->bCurrentSensorContact = pSamples[sample]->bNextSensorContact;
}

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock(); 
}

//free the ios, and reset back for next test
void freeEverything() {
	iolib_free();
	pParam->currentState = init;
	pParam->nextState = downStep;
	printf("free everything \n");
	//exit(EXIT_SUCCESS);
}

//Function definitions  
int readADC(unsigned int pin)  
{  
     int fd;          //file pointer  
     char buf[MAX_BUF];     //file buffer  
     char val[4];     //holds up to 4 digits for ADC value  
     
     //Create the file path by concatenating the ADC pin number to the end of the string  
     //Stores the file path name string into "buf"  
     snprintf(buf, sizeof(buf), "/sys/devices/ocp.3/helper.15/AIN%d", pin);     //Concatenate ADC file name
     
     fd = open(buf, O_RDONLY);     //open ADC as read only  
     
     //Will trigger if the ADC is not enabled  
     if (fd < 0) {  
          perror("ADC - problem opening ADC");  
     }//end if  
     
     read(fd, &val, 4);     //read ADC ing val (up to 4 digits 0-1799)  
     close(fd);     //close file and stop reading  
     
     return atoi(val);     //returns an integer value (rather than ascii)  
}//end read ADC()



