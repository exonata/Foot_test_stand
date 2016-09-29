#ifndef STATE_H_
#define STATE_H_


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <newmain.h>
#include "./BBBIOlib/BBBio_lib/BBBiolib.h"

struct data_t;
struct test_param_t;

int16_t stateMachine(test_param_t *pParam_t);
bool bSubStateMachine(data_t *pSample_t);

#endif
