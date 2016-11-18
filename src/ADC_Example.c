 #include <stdlib.h>  
 #include <stdio.h>  
 #include <string.h>  
 #include <unistd.h>     //close()  
 #include <fcntl.h>     //define O_WONLY and O_RDONLY  
#include <math.h>
#include "./BBBIOlib/BBBio_lib/BBBiolib.h"
#include "newmain.h"
#define MAX_BUF 64     //This is plenty large
#define X_INTERCEPT_LOAD_CELL_1 2.0671 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define Y_INTERCEPT_LOAD_CELL_1 17.3 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define X_INTERCEPT_LOAD_CELL_2 2.0644 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define Y_INTERCEPT_LOAD_CELL_2 20.6
/*
#define GAIN_TOE_HEEL 2
#define FIVE_V_INPUT 4.8
#define R2_TOE 322
#define R2_HEEL 2298
#define FOOT_SENSOR_INTERNAL_RES 470
#define MAX_SAMPLE 2 //max number of samples
#define ADC_MAX_V 1.8 //Volts
#define X_INTERCEPT_LOAD_CELL_1 2.0671 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define Y_INTERCEPT_LOAD_CELL_1 17.3 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define X_INTERCEPT_LOAD_CELL_2 2.0644 //computed from testing load cell and graphing in excel lbs vs output voltage mV
#define Y_INTERCEPT_LOAD_CELL_2 20.6 //computed from testing load cell and graphing in excel lbs vs output voltage mV
//note that the y intercept is the voltage offset of the load cell: must take into account into calculations
*/

 //Function declarations  
 //int readADC(unsigned int pin);  
 

 //main program  
 int main()  
 {
	 
	 
	 system("echo cape-bone-iio > /sys/devices/bone_capemgr.9/slots");
	 for(int b = 0; b < 10; b++) 
	 {
		 openValve(verticalValve);
		 sleep(250);
		 float adc0 = readADC(0);
		 printf("LC1: The voltage in mV is %f\n", adc0);
		 if (adc0 <= 17.3) {
			 printf("The force measured is: 0 lbs\n");
		 } else {
			 printf("The voltage of Load Cell 1 is: %f\n", adc0);
			 float force = ((adc0 - Y_INTERCEPT_LOAD_CELL_1 ) / X_INTERCEPT_LOAD_CELL_1); //we have a linear equation that maps voltage to lbs
			 printf("The force measured is: %f lbs \n", ceil(force));
		 }
			
		float adc1 = readADC(1);
		 printf("LC2: The voltage in mV is %f\n", adc1);
		 if (adc1 <= 25) {
			 printf("The force measured is: 0 lbs\n");
		 } else {
			 printf("The voltage of Load Cell 2 is: %f\n", adc1);
			 float force = ((adc1 - Y_INTERCEPT_LOAD_CELL_2 ) / X_INTERCEPT_LOAD_CELL_2); //we have a linear equation that maps voltage to lbs
			 printf("The force measured is: %f lbs \n", ceil(force));
		 }
		 sleep(250);		
		 closeValve(verticalValve);
		 sleep(250); 
		 adc0 = readADC(0);
		 printf("LC1: The voltage in mV is %f\n", adc0);
		 if (adc0 <= 17.3) {
			 printf("The force measured is: 0 lbs\n");
		 } else {
			 printf("The voltage of Load Cell 1 is: %f\n", adc0);
			 float force = ((adc0 - Y_INTERCEPT_LOAD_CELL_1 ) / X_INTERCEPT_LOAD_CELL_1); //we have a linear equation that maps voltage to lbs
			 printf("The force measured is: %f lbs \n", ceil(force));
		 }
					
		 adc1 = readADC(1);
		 printf("LC2: The voltage in mV is %f\n", adc1);
		 if (adc1 <= 25) {
			 printf("The force measured is: 0 lbs\n");
		 } else {
			 printf("The voltage of Load Cell 2 is: %f\n", adc1);
			 float force = ((adc1 - Y_INTERCEPT_LOAD_CELL_2 ) / X_INTERCEPT_LOAD_CELL_2); //we have a linear equation that maps voltage to lbs
			 printf("The force measured is: %f lbs \n", ceil(force));
		 }
		 sleep(250);
		 
	 }
		


	

	 /**
      printf("Voltage 2 Heel: %f V\n",adc2);
       printf("Voltage 3 Toe: %f V\n", adc3);

      float actualVoltage1 = adc2 / (float) GAIN_TOE_HEEL;
      printf("Heel voltage cut gain %f\n", actualVoltage1);
      float constRes1 = (actualVoltage1) / FIVE_V_INPUT;
      float r1_resistance = (R2_HEEL - constRes1 * R2_HEEL) / constRes1;
      printf("Res before 470 sub heel %f\n",r1_resistance);
      float resistanceH = r1_resistance - FOOT_SENSOR_INTERNAL_RES;
      
      float actualVoltage2 = adc3 / (float) GAIN_TOE_HEEL;
      printf("Toe voltage cut gain %f\n", actualVoltage2);
      float constRes2 = (actualVoltage2) / FIVE_V_INPUT;
      float r2_resistance = (R2_TOE - constRes2 * R2_TOE) / constRes2;
      printf("Res before 470 sub toe %f\n",r2_resistance);
      float resistanceT = r2_resistance - FOOT_SENSOR_INTERNAL_RES;
	*/
      //printf("Heel: %f ohms\n",resistanceH);
      //printf("ADC 3: %f ohms\n", resistanceT);

	 iolib_free();
      return 0;
 }//end main

/*
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
*/