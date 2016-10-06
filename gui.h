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
#ifndef GUI_H_
#define GUI_H_

#include <gtk/gtk.h>
#include <stdlib.h> //so the compiler knows what malloc 
#include <string.h> //need this for comparing strings
#include "./BBBIOlib/BBBio_lib/BBBiolib.h"
#include<pthread.h>
#include<stdio.h>
#include<unistd.h>
#include <signal.h>

 //DEFINES
 #define MIN_LBS 15.0
 #define MAX_LBS 580.0
 #define MIN_STEP 100
 #define MAX_STEP 10000


typedef struct text_responses{
	GtkWidget *entry1, *entry2, *entry3, *entry4, *entry5, *entry6, *entry7, *parent;
}text_responses;

void createWarningWindow(gchar *message, GtkWindow *window);
void callGUI();
void runTest();
void configPauseReset();
void UpDownStepValveConfig(int16_t currentState, int16_t nextState);
void on_pause (GtkWidget *widget, gpointer data, text_responses *text_obj);
void on_resume (GtkWidget *widget, gpointer data, text_responses *text_obj);
void on_abort (GtkWidget *widget, gpointer   data, text_responses *text_obj);
void *doSomeThing(void *arg);
void testADC();
void testVerticalValves();
void testRotaryValves();
void PWMTest();
#endif
