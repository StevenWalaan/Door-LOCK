/*
 * Timer1.c
 *
 *  Created on: 29 Oct 2023
 *      Author: Steven
 */


#include"Timer1.h"
#include <avr/io.h>
#include <avr/interrupt.h> /* For ICU ISR */

/* Global variables to hold the address of the call back function in the application */
static volatile void (*g_callBackPtr)(void) = NULL_PTR;


ISR(TIMER1_COMPA_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}

ISR(TIMER1_OVF_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}

void Timer1_init(const Timer1_ConfigType *Config_Ptr)
{
	/*PUT THIS BITS AS 1 BEC. TIMER NOT WORKS WITH PWM MODE*/
	TCCR1A = (1<<FOC1A)|(1<<FOC1B);
	/*PUT THE PRESCALER IN FIRST 3 BITS IN TCCR1B*/
	/*AND PUT THE OTHER BITS =0*/
	TCCR1B=(Config_Ptr->prescaler);
	/*PUT THE INITIAL VALUE*/
	TCNT1=(Config_Ptr->initial_value);

	if(Config_Ptr->mode==NORMAL_MODE)
	{
		TCCR1B &= ~(1<<WGM12) &~(1<<WGM13) &~(1<<WGM11) &~(1<<WGM10);

		/*TO ONLY ENABLE INTERRUPT OF NORMAL MODE*/
		TIMSK|=(1<<TOIE1);
		TIMSK &=~(1<<OCIE1A) &~(1<<OCIE1B) &~(1<<TICIE1);

	}
	else if(Config_Ptr->mode==COMPARE_MODE)
	{

		TCCR1B|=(1<<WGM12);
		/*TO ONLY ENABLE INTERRUPT OF COMPARE MODE*/
		TIMSK|=(1<<OCIE1A);
		TIMSK &=~(1<<TOIE1) &~(1<<OCIE1B) &~(1<<TICIE1);

		OCR1A=(Config_Ptr->compare_value);
	}

}

void Timer1_deInit(void)
{
	TCCR1A=0;
	TCCR1B=0;
	TIMSK=0;
	OCR1A=0;
	TCNT1=0;
	g_callBackPtr=  NULL_PTR ;
}

void Timer1_setCallBack(void(*a_ptr)(void))
{
	/* Save the address of the Call back function in a global variable */
	g_callBackPtr = a_ptr;
}

