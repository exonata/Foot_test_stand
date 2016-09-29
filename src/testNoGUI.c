
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
 *  @file testNoGUI.c
 *  @brief until monitor arrives feeds values into the actuateBBB
 *
 *
 *  $Revision: $1.0
 *  $Date: $06/30/2016
 *
 */

#include "actuateBBB.c"
#include "gui.h"
int main(int argc, char *argv[])
{
	gchar * defaultVal = "N";
	int sampleSize = 1;
	float desiredForce = 40;
	int upStep = 500;
	int downStep = 500;
	int numCycles = 500; 
	gchar *rotatingPlates = "Y";
	actuateBeaglebone(defaultVal, sampleSize, desiredForce, upStep, downStep, numCycles, rotatingPlates);
}
