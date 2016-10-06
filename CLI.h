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
 *  @file CLI.h
 *  @brief Command line module, is called every loop by main control loop and scans for
 *  		user input/defined flags *
 *  $Revision: $1.0
 *  $Date: $04/11/2016
 *
 */

#ifndef CLI_H_
#define CLI_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "newmain.h"


void usage(void);
bool scan_args(void);
int16_t returnCommand(void);

#endif
