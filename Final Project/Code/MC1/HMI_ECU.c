/*
 * HMI_ECU.c
 *
 *  Created on: 31 Oct 2023
 *      Author: Steven
 */
#include <avr/io.h> /* To use the IO Ports Registers */
#include "keypad.h"
#include "lcd.h"
#include <util/delay.h>
#include "gpio.h"
#include "std_types.h"
#include "uart.h"
#include "Timer1.h"

/*DEFINITIONS TO USED BY UART TO MAKE SENDING AND RECIVING MORE ACCURATE*/
#define  MC1_ASK_MC2 0X01
#define  MC2_ASK_MC1 0X02
#define  MC1_ANS_MC2 0X03
#define  MC2_ANS_MC1 0X04


typedef enum
{
	create_pass,options,open_door,change_pass_req,warning,open_door_req
}status;
status state;

uint8 wait_flag=0;

void openCallBackFunction(void)
{
	/*STATIC BEC IT USED ONLY IN THIS SCOPE*/
	static uint8 open_tick=0;
	open_tick++;
	if(open_tick==1){
		LCD_clearScreen();
		LCD_displayString("Door unlocking");

		Timer1_ConfigType Timer1_Configurations1= {0,23438,F_CPU_1024,COMPARE_MODE};
		Timer1_init(&Timer1_Configurations1);
	}
	else if(open_tick==6){
		LCD_clearScreen();
		LCD_displayString("Door is open");
	}
	else if(open_tick==7){
		LCD_clearScreen();
		LCD_displayString("Door locking");
	}
	else if(open_tick==12){
		open_tick=0;
		wait_flag=0;
		Timer1_deInit(); /*STOPS THE TIMER*/
	}
}

void warnCallBackFunction(void)
{
	/*STATIC BEC IT USED ONLY IN THIS SCOPE*/
	static uint8 warn_tick=0;
	warn_tick++;
	if(warn_tick==1)
	{
		LCD_clearScreen();
		LCD_displayString("Error");
		Timer1_ConfigType Timer1_Configurations2= {0,39063,F_CPU_1024,COMPARE_MODE};
		Timer1_init(&Timer1_Configurations2);
	}
	if(warn_tick==12)
	{
		warn_tick=0;
		wait_flag=0;
		Timer1_deInit();
	}


}

int main(void)
{
	uint8 pass[5],repass[5]; /*FOR PASSWORD*/
	uint8 option; /* FOR THE OPTIONS (OPEN DOOR OR CHANGE PASS)*/

	/*enable interrupt*/
	SREG|=(1<<7);



	/* Create configuration structure for UART driver */
	UART_ConfigType UART_Configurations = {BIT_8,NO_PARITY,BIT_1,9600};


	UART_init(&UART_Configurations);

	LCD_init();
	while(1)
	{
		/*MC1 WAITING TILL MC2 COMMUNICATE WITH IT TO RECIVE THE STATE*/

		while(UART_recieveByte()!= MC2_ASK_MC1);
		UART_sendByte(MC1_ANS_MC2);
		state=UART_recieveByte();

		switch(state)
		{

		case create_pass:

			LCD_clearScreen();
			LCD_displayString("plz enter pass:");
			LCD_moveCursor(1,0);/* Move the cursor to the second row */
			for( uint8 i=0;i<5;i++)
			{
				/* Get the pressed key number, if any switch pressed for more than 500 ms it will considered more than one press */
				pass[i]=KEYPAD_getPressedKey();

				//LCD_intgerToString(pass[i]);

				/*Display '*' after pressing instead of displaying the no.*/
				LCD_displayCharacter('*');
				_delay_ms(500);
			}
			/*waiting till press "=" (enter)*/
			while(KEYPAD_getPressedKey()!='=');
			_delay_ms(500);

			LCD_clearScreen();
			LCD_displayString("plz re-enter");
			LCD_displayStringRowColumn(1,0,"same pass:");
			/*for loop to enter pass again*/
			for( uint8 i=0;i<5;i++)
			{
				repass[i]=KEYPAD_getPressedKey();

				LCD_displayCharacter('*');
				//LCD_intgerToString(repass[i]);
				_delay_ms(500);
			}

			while(KEYPAD_getPressedKey()!='=');
			_delay_ms(500);
			for( uint8 i=0;i<5;i++)
			{
				/*MC1 sending the pass to MC2 */

				UART_sendByte(MC1_ASK_MC2);
				while( UART_recieveByte()!= MC2_ANS_MC1);
				UART_sendByte(pass[i]);
				UART_sendByte(MC1_ASK_MC2);
				while( UART_recieveByte()!= MC2_ANS_MC1);
				UART_sendByte(repass[i]);

			}
			break;


		case options:

			LCD_clearScreen();
			LCD_displayString("+ : open door");
			LCD_displayStringRowColumn(1,0,"- : change pass");


			/*waits in a while loop till the user enters the right options */
			while(1){
				option=KEYPAD_getPressedKey();
				_delay_ms(500);
				if(option=='+' ||option=='-')
					break;
			}
			/*MC1 sending the required option  to MC2 */
			UART_sendByte(MC1_ASK_MC2);
			while( UART_recieveByte()!= MC2_ANS_MC1);
			UART_sendByte(option);

			break;

			/*put the two cases together bec. they have same function  */
		case change_pass_req:
		case open_door_req:

			LCD_clearScreen();
			LCD_displayString("plz entr pass:");
			LCD_moveCursor(1,0);

			for( uint8 i=0;i<5;i++)
			{
				/* Get the pressed key number, if any switch pressed for more than 500 ms it will considered more than one press */
				pass[i]=KEYPAD_getPressedKey();

				//LCD_intgerToString(pass[i]);
				/*Display '*' after pressing instead of displaying the no.*/
				LCD_displayCharacter('*');

				_delay_ms(500);
			}

			/*waiting till press "=" (enter)*/
			while(KEYPAD_getPressedKey()!='=');
			_delay_ms(500);

			/* send the pass to MC2*/
			for( uint8 i=0;i<5;i++)
			{
				UART_sendByte(MC1_ASK_MC2);
				while( UART_recieveByte()!= MC2_ANS_MC1);
				UART_sendByte(pass[i]);

			}
			break;

		case open_door:

			/*enable flag */
			wait_flag=1;
			/* Create configuration structure for TIMER1 driver */
			/*
			 * FIRST MAKE THE TIMER INIT BY 0.25 SEC
			 * TO GO TO THE CALL BACK FUNCTION FAST WITHOUT DELAYS
			 * THEN MAKE TIMER INIT TO 3 SEC IN THE CALL BACK FUNCTION
			 */
			Timer1_ConfigType Timer1_Configurations1= {0,1953,F_CPU_1024,COMPARE_MODE};
			Timer1_setCallBack(openCallBackFunction);

			Timer1_init(&Timer1_Configurations1);
			/* waits till the flag returns to zero (to go to option state)*/
			while(wait_flag!=0);
			state=options;
			break;

		case warning:

			/*enable flag*/
			wait_flag=1;
			/* Create configuration structure for TIMER1 driver */

			/* Create configuration structure for TIMER1 driver */
			/*
			 * FIRST MAKE THE TIMER INIT BY 0.25 SEC
			 * TO GO TO THE CALL BACK FUNCTION FAST WITHOUT DELAYS
			 * THEN MAKE TIMER INIT TO 5 SEC IN THE CALL BACK FUNCTION
			 */
			Timer1_ConfigType Timer1_Configurations2= {0,1953,F_CPU_1024,COMPARE_MODE};
			Timer1_setCallBack(warnCallBackFunction);

			Timer1_init(&Timer1_Configurations2);
			/* waits till the flag returns to zero (to go to option state)*/
			while(wait_flag!=0);
			state=options;
			break;


		}


	}

}

