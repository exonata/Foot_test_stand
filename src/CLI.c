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
 *  @file CLI.c
 *  @brief Command line module, is called every loop by main control loop and scans for
 *  		user input/defined flags
 *
 *
 *  $Revision: $1.0
 *  $Date: $04/11/2016
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "CLI.h"
#include <getopt.h>

int16_t *command;

/**@brief when user runs with the -h command line arg, display help text
 *
 *
 * @return
 */

void usage()
{
  fprintf(stderr,
      " usage:\n"
      "     [p] [h] [q] [r] [n]\n"
      "       p 			pause current test\n"
      "       h             print out this help message\n"
      "       q             cancel test\n"
      "       r 			restart system\n"
      "       n		   		restart from pause\n"
      "\n");
}


/**@brief parse command line arguments, and sets the command pointer with command
 * line input
 *
 *@return bool true if input detected
 */
bool scan_args(void)
{
	int16_t input;
	input = -1;
	scanf(" %c", &input);
	command = &input;
    if(command[0] == -1)
    {
    	return false;
    }

    if(command[0] == 'h' || command[0] == 'p' || command[0] == 'q' || command[0] == 'r' || command[0] == 'n')
    {
		return true;
    }
    else
    {
    	printf("Invalid input \n");
    	usage();
    	return false;
    }
}
/**@brief parse command flag and return command
 *
 * @return command
 */
int16_t returnCommand()
{
	switch(*command)
		{
		  case 'h':
			  usage();
			  break;
		  case 'p':
			  printf("Case changed to pause\n");
			  return hold;
			  break;  //set state to pause
		  case 'q':
			  return quit;
			  break;  //set state to close
		  case 'r':
//KAS- "reset" seems a little more like "end" maybe?
			  return reset;
			  break;  //shutdown everything
		  case 'n':
			  return init;
			  break;  //eventually choose where to save files when connected to the network

		  case ':':
			  fprintf(stderr, " -%c missing arg\n", optopt);
					usage(); exit(1); break;
		  case '?':
			  fprintf(stderr, " unknown arg %c\n", optopt);
					usage(); exit(1); break;
		} // switch

}
