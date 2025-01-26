/*
 * DC_Motor.h
 *
 *  Created on: 13 Oct 2023
 *      Author: Steven
 */

#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_


#include "common_macros.h"

#include "std_types.h"


#define DC_MOTOR_PORT_ID  PORTB_ID

#define DC_MOTOR_PIN1_ID  PIN4_ID
#define DC_MOTOR_PIN2_ID  PIN5_ID
#define DC_MOTOR_EN_ID    PIN3_ID


typedef enum
{
	stop,clock_wise,anti_clock_wise
}DcMotor_State;

void Dc_MotorRotate(DcMotor_State state,uint8 speed);
void DcMotor_Init(void);

#endif /* DC_MOTOR_H_ */
