#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "./BBBIOlib/BBBio_lib/BBBiolib.h"

//CONSTANTS
#define HEADER_P8 8
#define HEADER_P9 9
#define SOL_VALVE_1 11  //GPIO 11, P8 up-down sample 1
#define SOL_VALVE_2 12  //GPIO 12, P8 up-down sample 1
#define SOL_VALVE_3 15 //GPIO 15, P8 up-down sample 2
#define SOL_VALVE_4 16 //GPIO 16, P8 up-down sample 2
#define SOL_VALVE_5 17 //GPIO 7, P8 rotary sample 1
#define SOL_VALVE_6 19 //GPIO 8, P8 rotary sample 2
 int main(void)  
 {
 	printf("Initiating Valves......\n");
 	iolib_setdir(HEADER_P8, SOL_VALVE_1, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_2, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_3, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_4, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_5, BBBIO_DIR_OUT);
	iolib_setdir(HEADER_P8, SOL_VALVE_6, BBBIO_DIR_OUT);
	printf("Valves initiated\n");

	printf("----------------------------------------\n");
	printf("Starting vertical valve tests:\n");

	//VERTICAL VALVE TEST
	//make sure the up down valves in hold: 0 0 
	printf("high\n");
	printf("Set the first sample valve in hold.\n");
	pin_low(HEADER_P8,SOL_VALVE_1);
	pin_low(HEADER_P8,SOL_VALVE_2);
	printf("Set the second valve in hold. \n");
	pin_low(HEADER_P8,SOL_VALVE_3);
	pin_low(HEADER_P8,SOL_VALVE_4);
	sleep(20); //let delay for 20 seconds
	printf("Now set valve 1 and 3 high and leave 2 and 4 low\n");
	pin_high(HEADER_P8,SOL_VALVE_1);
	pin_high(HEADER_P8,SOL_VALVE_3);
	sleep(20);
	printf("Now set valve 1 and 3 low and set 2 and 4 high\n");
	pin_low(HEADER_P8,SOL_VALVE_1);
	pin_low(HEADER_P8,SOL_VALVE_3);
	pin_high(HEADER_P8,SOL_VALVE_2);
	pin_high(HEADER_P8,SOL_VALVE_4);
	sleep(20);
	printf("Set the first sample valve in hold.\n");
	pin_low(HEADER_P8,SOL_VALVE_2);
	printf("Set the second valve in hold. \n");
	pin_low(HEADER_P8,SOL_VALVE_4);
	sleep(20);
 }
