#ifndef MOTOR_H
#define MOTOR_H

#include "../FSM_types.h"
/********************  Macro definitions  ********************/
#define SERVO_RATIO ((double)(200.0/360))
#define GEAR_RATIO ((uint64)(1))
/********************  External functions  ********************/
/*
*   Description:
*       Initializes the motor by establishing USB connection
*   In: 
*       void
*   Out:
*       E_OK: connection succeed
*       E_NOK: connection failed
*/
Return_t motor_init(void);

/*
*   Description:
*       Moves the motor by a certain angle
*   In: 
*       angle: The angle to be rotated
*   Out:
*       E_OK: motor movement succeed
*       E_NOK: motor movement failed
*/
Return_t motor_move_by_angle(uint16 angle);
#endif