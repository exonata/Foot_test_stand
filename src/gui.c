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
 *  @file example-0.h
 *  @brief GUI for controlling foot test stand and allow user input through GUI
 *
 *
 *  $Revision: $1.0
 *  $Date: $06/21/2016
 *
 */
#include "newmain.h"
#include "gui.h"
#include "newmain.c"

//need an array to store the theads
pthread_t tid[2];
//global variables
int startTest = 0; //int for checking if we have started a test or not
//struct to hold all the info we gather from the gui
struct text_responses;
text_responses *text_holder;

//for debugging button callback, useless otherwise
static void
print_hello (GtkWidget *widget,
             gpointer   data)
{
  g_print ("Hello World\n");
}

//the button callback for pause, will pause test if test running but if not will pop up a warning window
void
on_pause (GtkWidget *widget,
             gpointer   data, text_responses *text_obj)
{
	if (startTest == 0) {
			createWarningWindow("Fill out test specifications first!\nor you haven't started the test yet!", (GtkWindow *)text_obj->parent);
			return;
	}
	if (paramBool ("bCommandFlag", "Get", false)) {
		createWarningWindow("You cannot press this button. \nYou already pressed pause!\n", (GtkWindow *)text_obj->parent);
		return;
	}
	//keep track of current state
	param16("stateBeforePause", "Set", param16("currentState", "Get", 0));
	//set command flag to true: i.e have received it!
	paramBool ("bCommandFlag", "Set", true);
	//set state to hold
	paramu16("command", "Set", hold);
}

//the button callback for resume, will resume if pause has been pressed but otherwise will pop up a warning window
void
on_resume (GtkWidget *widget, gpointer   data, text_responses *text_obj) {
	if (startTest == 0) {
		createWarningWindow("Fill out test specifications first!\nor you haven't started the test yet!", (GtkWindow *)text_obj->parent);
		return;
	}
	if (!paramBool ("bCommandFlag", "Get", false)) {
		createWarningWindow("You cannot press this button if \nyou haven't pressed pause first!\n", (GtkWindow *)text_obj->parent);
		return;
	}
	//set command flag to false
	paramBool ("bCommandFlag", "Set", false);
	//set the command to the state before pause
	paramu16("command", "Set", param16("stateBeforePause", "Get", 0)); //set command to state before pause
	param16("nextState", "Set", param16("stateBeforePause", "Get", 0)); //set current state as saved state

}

//the button callback for abort, will abort the test and put valves into safety position
void
on_abort (GtkWidget *widget, gpointer   data, text_responses *text_obj) {
	if (startTest == 0) {
		createWarningWindow("Fill out test specifications first!\nor you haven't started the test yet!", (GtkWindow *)text_obj->parent);
		return;
	}
	//set state to reset
	paramu16("command", "Set", reset);
	param16("nextState", "Set", reset); //set current state as saved state
}

//fills the responses with the default values for the test
static void
fill_responses(GtkButton *widget,
             text_responses *text_obj) {
  gtk_entry_set_text((GtkEntry *)text_obj->entry3, DEFAULT_FORCE);
  gtk_entry_set_text((GtkEntry *)text_obj->entry4, DEFAULT_UP_STEP);
  gtk_entry_set_text((GtkEntry *)text_obj->entry5, DEFAULT_DOWN_STEP);
  gtk_entry_set_text((GtkEntry *)text_obj->entry6, DEFAULT_CYCLES);
  gtk_entry_set_text((GtkEntry *)text_obj->entry1, DEFAULT_ROTATING_PLATES);
}

//the button call back for start test: will throw a warning if you try to hit the button again during a test,
//will check to see if all entries to the GUI are legal and will notify you with a warning screen if
//that is the case, if all is good, then the method will fork a new thread to begin the test
static void
input_collect_and_validate (GtkButton *widget, text_responses *text_obj)
{
	if (startTest == 1) {
		createWarningWindow("Test is already in progress!", (GtkWindow *)text_obj->parent);
		return;
	}
  const gchar *choice1 = gtk_entry_get_text ((GtkEntry *)text_obj->entry1);
  gchar *choice2 = gtk_combo_box_text_get_active_text ((GtkComboBoxText *)text_obj->entry2);
  const gchar *choice3 = gtk_entry_get_text ((GtkEntry *)text_obj->entry3);
  const gchar *choice4 = gtk_entry_get_text ((GtkEntry *)text_obj->entry4);
  const gchar *choice5 = gtk_entry_get_text ((GtkEntry *)text_obj->entry5);
  const gchar *choice6 = gtk_entry_get_text ((GtkEntry *)text_obj->entry6);
  gchar *choice7 = gtk_combo_box_text_get_active_text ((GtkComboBoxText *)text_obj->entry7);

//change values to integers
  float delaySec = atof(choice1);
  float desiredForce = atof(choice3); //returns 0.0 if unsucessful
  int upStep = atoll(choice4); //0 if unsuccessful?
  int downStep = atoll(choice5);
  int numCycles = atoll(choice6);
  if (desiredForce < MIN_LBS || desiredForce > MAX_LBS) {
    gchar *s;
    if (strcmp("", choice3) == 0) { //means the person left it blank
      s = "Desired force has to be between 15 - 580 lbs.\nYou left the entry blank.\n";
    } else {
      s = "Desired force has to be between 15 - 580 lbs.\nYou chose an invalid input.";
    }
    createWarningWindow(s, (GtkWindow *)text_obj->parent);

  } else if (upStep < MIN_STEP || upStep > MAX_STEP) {
    gchar *s;
    if (strcmp("", choice4) == 0) { //means the person left it blank
      s = "Up step has to between 100 - 10000 milliseconds.\nYou left the entry blank.\n";
    } else {
      s = "Up step has to between 100 - 10000 milliseconds.\nYou chose an invalid input.";
    }
    createWarningWindow(s, (GtkWindow *)text_obj->parent);
  } else if (downStep < MIN_STEP || downStep > MAX_STEP) {
    gchar *s;
    if (strcmp("", choice5) == 0) { //means the person left it blank
      s = "Down step has to between 100 - 10000 milliseconds.\nYou left the entry blank.\n";
    } else {
      s = "Down step has to between 100 - 10000 milliseconds.\nYou chose an invalid input.";
    }
    createWarningWindow(s, (GtkWindow *)text_obj->parent);
  } else if (numCycles < 1) {
    gchar *s;
    if (strcmp("", choice6) == 0) { //means the person left it blank
      s = "Number of cycles has to be greater than zero.\nYou left the entry blank.\n";
    } else {
      s = "Number of cycles has to be greater than zero.\nYou chose an invalid input.";
    }
    createWarningWindow(s, (GtkWindow *)text_obj->parent);
  } else if (delaySec < 0.0 || delaySec > 10000) { //greater than 10 seconds no to large
    gchar *s;
    int val = strcmp("", choice1);
    printf("This is the value: %d\n",val);
    if (strcmp("", choice1) == 0) { //means the person left it blank
      s = "Rotating plate delay has to be greater than zero.\nYou left the entry blank.\n";
    } else {
      s = "Rotating plate delay has to be greater than zero.\nYou chose an invalid input.";
    }
    createWarningWindow(s, (GtkWindow *)text_obj->parent);
  } else {

    //we all good and we can start the tests
    printf("All good to start tests\n");
    cleanTest(text_obj);				//initiate a clean test
    startTest = 1;
	signal(SIGALRM, SIG_IGN); //sto alarm from killing all processes
    int err = pthread_create(&(tid[0]), NULL, doSomeThing, NULL);
    if (err != 0)
    	printf("\ncan't create thread :[%s]", strerror(err));
    else
    	printf("\n Thread created successfully\n");

  }
}

//this is the thread call for spawning the thread that runs the test
//does not accept any parameters, all it does is runs the test
void *doSomeThing(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    if(pthread_equal(id,tid[0]))
    {

    	signal(SIGALRM, SIG_IGN);
    	//alarm(100000);
		//printf("Going into while loop\n");
		runTest();
		signal(SIGALRM, SIG_IGN);
    }
    signal(SIGALRM, SIG_IGN);
	printf("TEST COMPLETE!\n");
	//freeEverything();
	startTest = 0; //reset start test to zero
    return NULL;
}

//creates a warning window with the specified text for the specific error
//takes a string pointer and a parent window pointer as a parameter
void createWarningWindow(gchar *message, GtkWindow *window) {
  GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
  GtkWidget *dialog = gtk_message_dialog_new (window,
                                 flags,
                                 GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_CLOSE,
                                 "%s", message);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

//the method responsible for creating the GUI with buttons using GTK
static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  
  //allocate space for the struct
  text_holder = (struct text_responses*) malloc(sizeof(struct text_responses));

  //assign values to the members of the struct

  text_holder->entry1 = gtk_entry_new();
  //gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(text_holder->entry1), NULL, "YES");
  //gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(text_holder->entry1), NULL, "NO");
  //gtk_combo_box_set_active(GTK_COMBO_BOX(text_holder->entry1), 1);
  text_holder->entry2 = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(text_holder->entry2), NULL, "1");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(text_holder->entry2), NULL, "2");
  gtk_combo_box_set_active(GTK_COMBO_BOX(text_holder->entry2), 0);
  text_holder->entry3 = gtk_entry_new();
  text_holder->entry4 = gtk_entry_new();
  text_holder->entry5 = gtk_entry_new();
  text_holder->entry6 = gtk_entry_new();
  text_holder->entry7 = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(text_holder->entry7), NULL, "YES");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(text_holder->entry7), NULL, "NO");
  gtk_combo_box_set_active(GTK_COMBO_BOX(text_holder->entry7), 1);
  
 
  //GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *buttonStart, *buttonPause, *buttonAbort, *buttonResume, *buttonDefault;
  GtkWidget *label1, *label2, *label3, *label4, *label5, *label6, *label7, *label8;
  int startx = 0;
  int starty = 0;

  /* create a new window, and set its title */
  text_holder->parent = gtk_application_window_new (app);
  //gtk_window_set_default_size (GTK_WINDOW (window), 700, 700); //Set default size for the window
  gtk_window_set_title (GTK_WINDOW (text_holder->parent), "Ekso Bionics Foot Stomp Tester");
  gtk_container_set_border_width (GTK_CONTAINER (text_holder->parent), 10);

  /* Here we construct the container that is going pack our buttons */
  grid = gtk_grid_new ();

  /* Pack the container in the window */
  gtk_container_add (GTK_CONTAINER (text_holder->parent), grid);

  /*QUESTION 1: Default settings*/
  /* create a new label. */
  label1 = gtk_label_new ("Default Settings: " );
  //layout on the grid: prompt
  gtk_grid_attach (GTK_GRID (grid), label1, startx, starty, 1, 1);
  //layout on the grid: response
  
  buttonDefault = gtk_button_new_with_label ("YES");
  gtk_grid_attach (GTK_GRID (grid), buttonDefault, startx + 1, starty, 1, 1);
  g_signal_connect(buttonDefault, "clicked", G_CALLBACK(fill_responses), text_holder);

  //gtk_grid_attach (GTK_GRID (grid), text_holder->entry1, startx + 1, starty, 1, 1);


  /*QUESTION 2: Input samples*/
  /* create a new label. */
  label2 = gtk_label_new ("Input Number of Samples: " );
  //layout on the grid: prompt
  gtk_grid_attach (GTK_GRID (grid), label2, startx, starty + 1, 1, 1);
  //layout on the grid: response
  gtk_grid_attach (GTK_GRID (grid), text_holder->entry2 , startx + 1, starty + 1, 1, 1);

  /*QUESTION 3: Input Desired Force*/
  /* create a new label. */
  label3 = gtk_label_new ("Input Desired Force(15 - 580 lbs):");
  //layout on the grid: prompt
  gtk_grid_attach (GTK_GRID (grid), label3, startx, starty + 2, 1, 1);
  //layout on the grid: response
  gtk_grid_attach (GTK_GRID (grid), text_holder->entry3, startx + 1, starty + 2, 1, 1);

  /*QUESTION 4: Input Up-Step*/
  /* create a new label. */
  label4 = gtk_label_new ("Input Up Step (100 - 10000 milliseconds): ");
  //layout on the grid: prompt
  gtk_grid_attach (GTK_GRID (grid), label4, startx, starty + 3, 1, 1);
  //layout on the grid: response
  gtk_grid_attach (GTK_GRID (grid), text_holder->entry4, startx + 1, starty + 3, 1, 1);

   /*QUESTION 5: Input Down-Step*/
  /* create a new label. */
  label5 = gtk_label_new ("Input Down Step (100 - 10000 milliseconds): " );
  //layout on the grid: prompt
  gtk_grid_attach (GTK_GRID (grid), label5, startx, starty + 4, 1, 1);
  //layout on the grid: response
  gtk_grid_attach (GTK_GRID (grid), text_holder->entry5, startx + 1, starty + 4, 1, 1);

   /*QUESTION 6: Input Number of Cycles*/
  /* create a new label. */
  label6 = gtk_label_new ("Input Number of Cycles: " );
  //layout on the grid: prompt
  gtk_grid_attach (GTK_GRID (grid), label6, startx, starty + 5, 1, 1);
  //layout on the grid: response
  gtk_grid_attach (GTK_GRID (grid), text_holder->entry6, startx + 1, starty + 5, 1, 1);
  
   /*QUESTION 7: Do you Need Rotating Plates? (Y or N): */
  /* create a new label. */
  label7 = gtk_label_new ("Do you Need Rotating Plates?: " );
  //gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
  gtk_grid_attach (GTK_GRID (grid), label7, startx, starty + 6, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), text_holder->entry7, startx + 1, starty + 6, 1, 1);

  /*QUESTION 7: Rotating Plate Delay*/
  /* create a new label. */
  label8 = gtk_label_new ("Rotating Plate Delay: (ms)" );
  //gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
  gtk_grid_attach (GTK_GRID (grid), label8, startx, starty + 7, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), text_holder->entry1, startx + 1, starty + 7, 1, 1);
  
 


  buttonStart = gtk_button_new_with_label ("Start Test");
  /* Place the Submit button in the grid cell (0, 1), and make it
   * span 4 columns.
   */
  gtk_grid_attach (GTK_GRID (grid), buttonStart, startx, starty + 8, 4, 1);
  g_signal_connect(buttonStart, "clicked", G_CALLBACK(input_collect_and_validate), text_holder);

  //make a button for pause
  buttonPause = gtk_button_new_with_label ("Pause");
  /* Place the Submit button in the grid cell (0, 1), and make it
   * span 4 columns.
   */
  gtk_grid_attach (GTK_GRID (grid), buttonPause, startx, starty + 9, 4, 1);
  g_signal_connect(buttonPause, "clicked", G_CALLBACK(on_pause), text_holder);

  //make a button for pause
  buttonResume = gtk_button_new_with_label ("Resume Test");
  /* Place the Submit button in the grid cell (0, 1), and make it
   * span 4 columns.
   */
  gtk_grid_attach (GTK_GRID (grid), buttonResume, startx, starty + 10, 5, 1);
  g_signal_connect(buttonResume, "clicked", G_CALLBACK(on_resume), text_holder);


  //make a button for Abort all 
  buttonAbort = gtk_button_new_with_label ("Abort All");
  /* Place the Submit button in the grid cell (0, 1), and make it
   * span 4 columns.
   */
  gtk_grid_attach (GTK_GRID (grid), buttonAbort, startx, starty + 11, 4, 1);
  g_signal_connect(buttonAbort, "clicked", G_CALLBACK(on_abort), text_holder);


  /* Now that we are done packing our widgets, we show them all
   * in one go, by calling gtk_widget_show_all() on the window.
   * This call recursively calls gtk_widget_show() on all widgets
   * that are contained in the window, directly or indirectly.
   */
  gtk_widget_show_all (text_holder->parent);

}
/**
 * Open all valves and hold in safety position during hold, reset, and quit
 */
void configPauseReset() {
	printf("open all valves and hold in safety position during hold, reset, or quit\n");
	openValve(verticalValve);  //open vertical valves
	paramBool("bLogTrue", "Set", false);  //don't log during interlude
	if(paramBool("bTurnFlag", "Get", true)) {   //if turnflag on
		openValve(turnValve_A);  //open sample A turn plate valves
		if(paramu16("numSAMPLE", "Get", 0) == MAX_SAMPLE) {
			openValve(turnValve_B); //if there are two samples, open both valves
		}
	}
	printf("out of configPauseReset\n");
}

void UpDownStepValveConfig(int16_t currentState, int16_t nextState) {
	if(currentState != nextState) { //need to begin transitioning to next state
		if(nextState == upStep) {
			openValve(verticalValve);
			paramu64("stepTime_ms", "Set", getTimestamp_ms());
			paramBool("bDownFlag", "Set", true);
		} else if (nextState == downStep) {
			closeValve(verticalValve);
			paramu64("stepTime_ms", "Set", getTimestamp_ms());
			paramBool("bUpFlag", "Set", true);
		} else if (nextState == hold || nextState == reset || nextState == quit) {
			configPauseReset();
		} else {  //if we encounter invalid state
			printf("Invalid state change\n");
		}
	}

}

/**
 * Main loop for the backend code
 */
void runTest(text_responses *text_obj) {
	printf("got into run test \n");
	while(paramu32("count", "Get", 0) <= paramu32("cycle", "Get", 0)) {
		signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
		getTimersPrintStates();
		updateVals();
		int16_t currentState = param16("currentState", "Get", 0);
		int16_t nextState = param16("nextState", "Get", 0);
		if (currentState == reset) {
			configPauseReset();
			printf("Test ABORTED so...");
			break;
		}
		//Based on states determine what to do
		//printf("I made it here before switch!\n");
		switch(currentState) {
			case init:
				printf("init \n");
				iolib_init(); 				//initiate GPIO library
				initValve();   				//initiate valve pins
				//initADC();    				//set up ADCs
				signal(SIGALRM, SIG_IGN);
				//enableLoadCellADC(); 	 	//enable load cell pins
				//enableFootADC(); 
				signal(SIGALRM, SIG_IGN);
				initTest();
				break;
				
			case downStep:	
				printf("upstep\n");
				UpDownStepValveConfig(currentState, nextState);
				break;
			case upStep:	
				printf("downstep\n");
				UpDownStepValveConfig(currentState, nextState);
				break;
			case hold:
				printf("hold\n");
				if (currentState != nextState) {
					if(nextState == downStep) {
						initTest();
						closeValve(verticalValve);
					} 
					else if(nextState == init) {
						printf("Restarting test\n");
						sleep(3);
					} 
					else if (nextState == reset || nextState == quit) {
						configPauseReset();
					} 
					else {
						printf("Invalid state change\n");
					}
				}
				break;
				
			case reset:
				printf("reset\n");
				break;
				
			case quit:
				printf("break\n");
				break;
				
			default:
				break;

		}
		signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
		//check sample substate, see if contact of sensor is made
		for (int16_t sample = sample_A; sample < paramu16("numSAMPLE", "Get", 0); sample++) {
			analyzeContact(sample);
			signal(SIGALRM, SIG_IGN);
			alarm(100000000);
		}
		//check the sample substate and see if contact of the sensor is made
		//check if full cycle completed
		if(paramBool("bUpFlag", "Get", true) && paramBool("bDownFlag", "Get", true)) {
			signal(SIGALRM, SIG_IGN);
			printf("Cycle complete\n");
			signal(SIGALRM, SIG_IGN);
			uint32_t val = paramu32("count", "Get", 0) + 1;
			paramu32("count", "Set", val);
			paramBool("bDownFlag", "Set", false);
			paramBool("bUpFlag", "Set", false);
		}
		signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
		
		//log data
		logData();
		if(currentState == quit) {
			break; //break for the while loop
		}
		//set current state to next state
		param16("currentState", "Set", param16("nextState", "Get", 0));
		signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	}
	configPauseReset();
	turnOffPressureReg(); //turn off pressure regulator at end of test
	freeEverything();

}

//test method for testing ADC0 and ADC1
void testADC() {
	printf("----------------------------------------\n");
	printf("ADC tests:\n");
	unsigned int sample1;
	unsigned int sample2;
	//int i ,j;
	
	
	system("echo cape-bone-iio > /sys/devices/bone_capemgr.9/slots");
		 for(int b = 0; b < 10; b++) 
		 {
			 printf("in loop\n");
			 
				printf("Now set valve 1 and 3 high and leave 2 and 4 low\n");
				pin_low(HEADER_P8,SOL_VALVE_1);
				pin_low(HEADER_P8,SOL_VALVE_2);
				pin_low(HEADER_P8,SOL_VALVE_3);
				pin_low(HEADER_P8,SOL_VALVE_4);
				pin_high(HEADER_P8,SOL_VALVE_1);
				pin_high(HEADER_P8,SOL_VALVE_3);
			 
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
				printf("Now set valve 1 and 3 high and leave 2 and 4 low\n");
				pin_low(HEADER_P8,SOL_VALVE_1);
				pin_low(HEADER_P8,SOL_VALVE_2);
				pin_low(HEADER_P8,SOL_VALVE_3);
				pin_low(HEADER_P8,SOL_VALVE_4);
				pin_high(HEADER_P8,SOL_VALVE_1);
				pin_high(HEADER_P8,SOL_VALVE_3);
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
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	/*
	
	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	unsigned int buffer_AIN_0[BUFFER_SIZE] ={0};
	unsigned int buffer_AIN_1[BUFFER_SIZE] ={0};
	iolib_init(); 				//initiate GPIO library
	BBBIO_ADCTSC_module_ctrl(BBBIO_ADC_WORK_MODE_TIMER_INT, clk_div);
	BBBIO_ADCTSC_channel_ctrl(BBBIO_ADC_AIN0, BBBIO_ADC_STEP_MODE_SW_CONTINUOUS, open_dly, sample_dly, \
		BBBIO_ADC_STEP_AVG_1, buffer_AIN_0, BUFFER_SIZE);

	BBBIO_ADCTSC_channel_ctrl(BBBIO_ADC_AIN1, BBBIO_ADC_STEP_MODE_SW_CONTINUOUS, open_dly, sample_dly, \
	BBBIO_ADC_STEP_AVG_1, buffer_AIN_1, BUFFER_SIZE);
	BBBIO_ADCTSC_channel_enable(BBBIO_ADC_AIN0);
	BBBIO_ADCTSC_channel_enable(BBBIO_ADC_AIN1);
	
	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	BBBIO_ADCTSC_work(SAMPLE_SIZE);
	sample1 = buffer_AIN_0[0];
	sample2 = buffer_AIN_1[0];
	printf("\t[sample : %d , %f v]\n", sample1, ((float)sample1 / 4095.0f) * 1.8f);
	printf("\t[sample : %d , %f v]\n", sample2, ((float)sample2 / 4095.0f) * 1.8f);
	sleep(1);
	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	printf("Now set valve 1 and 3 high and leave 2 and 4 low\n");
	pin_low(HEADER_P8,SOL_VALVE_1);
	pin_low(HEADER_P8,SOL_VALVE_2);
	pin_low(HEADER_P8,SOL_VALVE_3);
	pin_low(HEADER_P8,SOL_VALVE_4);
	pin_high(HEADER_P8,SOL_VALVE_1);
	pin_high(HEADER_P8,SOL_VALVE_3);
	sleep(2);
	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	BBBIO_ADCTSC_work(SAMPLE_SIZE);
		sample1 = buffer_AIN_0[0];
		sample2 = buffer_AIN_1[0];
		printf("\t[sample : %d , %f v]\n", sample1, ((float)sample1 / 4095.0f) * 1.8f);
		printf("\t[sample : %d , %f v]\n", sample2, ((float)sample2 / 4095.0f) * 1.8f);
	sleep(2);
	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	printf("Now set valve 1 and 3 low and set 2 and 4 high\n");
	pin_low(HEADER_P8,SOL_VALVE_1);
	pin_low(HEADER_P8,SOL_VALVE_3);
	pin_high(HEADER_P8,SOL_VALVE_2);
	pin_high(HEADER_P8,SOL_VALVE_4);
	sleep(2);
	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	BBBIO_ADCTSC_work(SAMPLE_SIZE);
	sample1 = buffer_AIN_0[0];
	sample2 = buffer_AIN_1[0];
	printf("\t[sample : %d , %f v]\n", sample1, ((float)sample1 / 4095.0f) * 1.8f);
	printf("\t[sample : %d , %f v]\n", sample2, ((float)sample2 / 4095.0f) * 1.8f);

	signal(SIGALRM, SIG_IGN); // need to ignore the stupid timer
	*/
	
	iolib_free();
}

//test method for testing vertical valve test
void testVerticalValves() {
	//VERTICAL VALVE TEST
	initValve();   				//initiate valve pins
	printf("----------------------------------------\n");
	printf("Starting vertical valve tests:\n");
	//make sure the up down valves in hold: 0 0
	printf("Set the first sample valve in hold.\n");
	pin_low(HEADER_P8,SOL_VALVE_1);
	pin_low(HEADER_P8,SOL_VALVE_2);
	printf("Set the second valve in hold. \n");
	pin_low(HEADER_P8,SOL_VALVE_3);
	pin_low(HEADER_P8,SOL_VALVE_4);
	sleep(10); //let delay for 10 seconds
	printf("Now set valve 1 and 3 high and leave 2 and 4 low\n");
	pin_high(HEADER_P8,SOL_VALVE_1);
	pin_high(HEADER_P8,SOL_VALVE_3);
	sleep(10);
	printf("Now set valve 1 and 3 low and set 2 and 4 high\n");
	pin_low(HEADER_P8,SOL_VALVE_1);
	pin_low(HEADER_P8,SOL_VALVE_3);
	pin_high(HEADER_P8,SOL_VALVE_2);
	pin_high(HEADER_P8,SOL_VALVE_4);
	sleep(10);
	printf("Set the first sample valve in hold.\n");
	pin_low(HEADER_P8,SOL_VALVE_2);
	printf("Set the second valve in hold. \n");
	pin_low(HEADER_P8,SOL_VALVE_4);
	sleep(10);
	iolib_free();
}

//test method for testing the rotary valves sol. 5 and 6
//no idea whether high/low turns it left or right
void testRotaryValves() {
	initValve();   				//initiate valve pins
	printf("----------------------------------------\n");
	printf("Starting rotary valve tests:\n");
	//try solenoid 5 first
	printf("Test solenoid 5 first!\n");
	printf("Drive it high.\n");
	pin_low(HEADER_P8,SOL_VALVE_5);
	sleep(10);
	printf("Drive it low.\n");
	pin_high(HEADER_P8,SOL_VALVE_5);
	sleep(10);
	printf("Test solenoid 6!\n");
	printf("Drive it high.\n");
	pin_low(HEADER_P8,SOL_VALVE_6);
	sleep(10);
	printf("Drive it low.\n");
	pin_high(HEADER_P8,SOL_VALVE_6);
	sleep(10);
	iolib_free();
}

//tests the PWM by giving duty cycle 10% all the way to 100%
void PWMTest() {
	printf("----------------------------------------\n");
	printf("PWM tests:\n");
	float percent = 10.0;
	for (int k = 10; k < 110; k+= 10) {
		float percent = k + 0.0;
		printf("Testing PWM at duty cycle: %f%%\n", percent);
		BBBIO_PWMSS_Setting(BBBIO_PWMSS0, PWM_HZ ,percent , percent);
		BBBIO_ehrPWM_Enable(BBBIO_PWMSS0);
		sleep(10);
	}

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
	printf("close\n");
	iolib_free();
}

/**
 * THESE ARE ALL THE FUNCTIONS FOR THE BACKEND CODE!!!!
 */


int main (int    argc,
      char **argv)
{
	//if we want to test the ADC, GPIO, and PWM in isolation, comment out rest of main
	/**
	testADC();
	testVerticalValves();
	testRotaryValves()
	PWMTest();
	*/
	iolib_init(); 				//initiate GPIO library
	initValve();   				//initiate valve pins
	//initADC();    				//set up ADCs
	//enableLoadCellADC(); 	 	//enable load cell pins
	//enableFootADC();  			//enable foot adc pins
	//run GUI
	testADC();
	
	
	GtkApplication *app;
	int status;
	app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return status;
}


