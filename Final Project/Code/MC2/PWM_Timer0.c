/*
 * PWM_Timer0.c
 *
 *  Created on: 13 Oct 2023
 *      Author: Steven
 */


#include <avr/io.h>
#include "gpio.h"
#include "PWM_Timer0.h"

void PWM_Timer0_Start(uint8 duty_cycle)
{
	uint8 duty;
	duty=((duty_cycle*255)/100);

	/*
	 * WGM01 AND WGM00 for FAST PWM
	 * COM01 for non inverting
	 * CS01 For pre-scaler=8
	 */
	TCCR0=(1<<WGM01) |(1<<WGM00)|(1<<COM01)|(CS01);
	TCNT0=6; //INITIAL VALUE
	OCR0 =duty; //compare value
	GPIO_setupPinDirection(PORTB_ID , PIN3_ID,PIN_OUTPUT );


}
