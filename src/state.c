#include "state.h"

/**
 * @brief takes in struct and determines next state based on data
 *
 *
 * @param test_param_t struct, pointer
 *
 *
 * @return int state
 */
int16_t stateMachine(test_param_t *pParam_t)
{
	int16_t newState;
	if(pParam_t->bCommandFlag)
	{
		if(pParam_t->command == hold)
		{
			newState = hold;
		}
		else if(pParam_t->command == quit)
		{
			newState = quit;
		}
		else if(pParam_t->command == reset)
		{
			newState = reset;
		}
		else
		{
			printf("Get here holddd!\n");
			newState = hold;
			printf("no new condition ,stay in hold\n");
		}
	}
	else
	{
		switch (pParam_t->currentState)
		{
		case init: //unsure of other things to do in this init loop
			newState = downStep;
			break;

		case downStep:
			if(pParam_t->elapsedTime_ms >= pParam_t->downStepTime_ms)
			{
				newState = upStep;
			}
			else
			{
				newState = downStep;
			}
			break;

		case upStep:
			if(pParam_t->elapsedTime_ms >= pParam_t->upStepTime_ms)
			{
				newState = downStep;
			}
			else
			{
				newState = upStep;
			}
			break;

		case quit:
			newState = quit;
			break;

		case hold:
//KAS- unreachable code because this is in the "else" for pParam_t->bCommandFlag already
//will comment out and checkfunc, was most likely concerned about
	/*		if(pParam_t->bCommandFlag)
			{
				if(pParam_t->command == hold)
				{
					newState = hold;
				}
				else if(pParam_t->command == quit)
				{
					newState = quit;
				}
				else if(pParam_t->command == reset)
				{
					newState = reset;
				}
				else
				{
					newState = hold;
					printf("no new condition ,stay in hold\n");
				}
			}
			else
			{
			*/
				newState = hold;
			//}
			break;
		case reset:
//KAS- the else is unreachable.  also, it's the same thing.  this will get stuck without a keyboard command 
			//removed redudant case -RS
				newState = reset;

			break;

		default:
			printf("stuck in default\n");
			break;
		}
	}
	return newState;

}
/**
 * @brief takes in struct and determines next state based on data
 *
 *
 * @param data_t struct, pointer
 *
 *
 * @return int substate
 */
//KAS- i would just call this a bool and not a statemachine and just have getFootContact since that's the only thing this is used for
//RS - This is just changing the name of the function right?   I will change bSubStateMachine to bSensorContact
bool bSensorContact(data_t *pSample_t)
{
	bool newSubState;

	if(pSample_t->baseForce + 10 < pSample_t->measuredForce)
	{
		newSubState = sensorContact;
	}
	else
	{
		newSubState = noSensorContact;
	}
	return newSubState;
}
