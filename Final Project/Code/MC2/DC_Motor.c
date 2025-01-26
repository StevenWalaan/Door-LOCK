/*
 * DC_Motor.c
 *
 *  Created on: 13 Oct 2023
 *      Author: Steven
 */

#include"DC_Motor.h"
#include "gpio.h"
#include "std_types.h"

void DcMotor_Init(void)
{
	/* choosing the motor output pins*/
	GPIO_setupPinDirection(DC_MOTOR_PORT_ID, DC_MOTOR_PIN1_ID ,PIN_OUTPUT);
	GPIO_setupPinDirection(DC_MOTOR_PORT_ID, DC_MOTOR_PIN2_ID ,PIN_OUTPUT);

	/* STOPING the motor at first*/
	GPIO_writePin(DC_MOTOR_PORT_ID, DC_MOTOR_PIN1_ID ,LOGIC_LOW);
	GPIO_writePin(DC_MOTOR_PORT_ID, DC_MOTOR_PIN1_ID ,LOGIC_LOW);

}

void Dc_MotorRotate(DcMotor_State state,uint8 speed)
{
	switch(state)
	{
	case stop:
		GPIO_writePin(DC_MOTOR_PORT_ID, DC_MOTOR_PIN1_ID ,LOGIC_LOW);
		GPIO_writePin(DC_MOTOR_PORT_ID, DC_MOTOR_PIN2_ID ,LOGIC_LOW);

		break;

	case clock_wise:
		GPIO_writePin(DC_MOTOR_PORT_ID, DC_MOTOR_PIN1_ID ,LOGIC_LOW);
		GPIO_writePin(DC_MOTOR_PORT_ID, DC_MOTOR_PIN2_ID ,LOGIC_HIGH);

		break;

	case anti_clock_wise:
		GPIO_writePin(DC_MOTOR_PORT_ID, DC_MOTOR_PIN1_ID ,LOGIC_HIGH);
		GPIO_writePin(DC_MOTOR_PORT_ID, DC_MOTOR_PIN2_ID ,LOGIC_LOW);
		break;
	}
	/*call the PWM func to choose speed of motor */
	PWM_Timer0_Start(speed);

}
