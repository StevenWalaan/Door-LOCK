/*
 * Control_ECU.c
 *
 *  Created on: 31 Oct 2023
 *      Author: Steven
 */

#include <avr/io.h> /* To use the IO Ports Registers */
#include"Buzzer.h"
#include"DC_Motor.h"
#include "external_eeprom.h"
#include <util/delay.h>
#include "uart.h"
#include "Timer1.h"
#include "twi.h"

/*DEFINITIONS TO USED BY UART TO MAKE SENDING AND RECIVING MORE ACCURATE*/
#define  MC1_ASK_MC2 0X01
#define  MC2_ASK_MC1 0X02
#define  MC1_ANS_MC2 0X03
#define  MC2_ANS_MC1 0X04


typedef enum
{
	create_pass,options,open_door,change_pass_req,warning,open_door_req
}status;
status state=create_pass;

/*FLAGS*/
uint8 pass_flag=0;
uint8 pass_match_flag=0;
uint8 pass_counter=0;
uint8 wait_flag=0;

void openCallBackFunction(void)
{
	/*STATIC BEC IT USED ONLY IN THIS SCOPE*/
	static uint8 open_tick=0;
	open_tick++;
	if(open_tick==1){

		Dc_MotorRotate(clock_wise,100);

		Timer1_ConfigType Timer1_Configurations1= {0,23438,F_CPU_1024,COMPARE_MODE};
		Timer1_init(&Timer1_Configurations1);
	}
	else if(open_tick==6){
		Dc_MotorRotate(stop,0);
	}
	else if(open_tick==7){
		Dc_MotorRotate(anti_clock_wise,100);
	}
	else if(open_tick==12){/*TO STOP THE MOTOR AFTER LAST 15 SEC*/
		Dc_MotorRotate(stop,0);
		open_tick=0;
		wait_flag=0;
		Timer1_deInit();
	}
}

void warnCallBackFunction(void)
{

	/*STATIC BEC IT USED ONLY IN THIS SCOPE*/
	static uint8 warn_tick=0;
	warn_tick++;
	if(warn_tick==1)
	{

		Buzzer_on();/* TURN ON THE BUZZER FOR 1 MIN.*/
		Timer1_ConfigType Timer1_Configurations2= {0,39063,F_CPU_1024,COMPARE_MODE};
		Timer1_init(&Timer1_Configurations2);
	}
	if(warn_tick==12)
	{
		warn_tick=0;
		wait_flag=0;
		Buzzer_off();/* TURN IT OFF*/
		Timer1_deInit(); /*STOPS THE TIMER*/
	}


}


int main(void)
{
	uint8 pass[5],repass[5];
	uint8 option;

	SREG|=(1<<7);

	Buzzer_init();
	DcMotor_Init();

	/*MY address=1 and by solving equation baud rate we put 0x02 in TWBR*/
	TWI_ConfigType TWI_Configurations = {0x01,0x02};
	TWI_init(&TWI_Configurations);
	/* Create configuration structure for UART driver */
	/* 8_bit mode without parity bit and 1 stop bit with baud_rate=9600*/
	UART_ConfigType UART_Configurations = {BIT_8,NO_PARITY,BIT_1,9600};
	UART_init(&UART_Configurations);


	while(1)
	{
		/*MC2 COMMUINCATES WITH MC1 AND WAITS TO IT'S ANSWER TO SEND STATE TO IT*/

		UART_sendByte(MC2_ASK_MC1);
		while( UART_recieveByte()!= MC1_ANS_MC2);
		UART_sendByte(state);
		switch(state)
		{
		case create_pass:

			/*
			 * RECIEVING THE PASS ENTERD FROM USER
			 */
			for(uint8 i=0;i<5;i++)
			{
				while( UART_recieveByte()!= MC1_ASK_MC2);
				UART_sendByte(MC2_ANS_MC1);
				pass[i]=UART_recieveByte();
				while( UART_recieveByte()!= MC1_ASK_MC2);
				UART_sendByte(MC2_ANS_MC1);
				repass[i]=UART_recieveByte();

			}
			/* PUT IT IN ARRAYS TO COMARE BET. THE TWO PASSWORDS*/
			for(uint8 i=0;i<5;i++)
			{
				if(pass[i]==repass[i]){
					pass_flag=0;
				}
				else{
					pass_flag=1;
					/*CHECKING A DIFFERENT NO. IN PASS*/

					break;/*END OF FOR LOOP TO TAKE NEW PASS*/
				}

			}

			/* IF TWO PASS MATCHES SEND THEM TO EEPROM
			 * AND GO TO OPTIONS STATE*/
			if(pass_flag==0){
				for(uint8 i=0;i<5;i++){
					EEPROM_writeByte(0x0001+i, pass[i]);
					_delay_ms(15);
				}
				state=options;
			}
			else{
				/*PUT IT TO NORMAL VALUE FOR THE NEXT TIME*/
				pass_flag=0;
			}
			break;

		case options:

			/*WAITING TILL THE MC1 SEND THE REQUIRD OPTION*/
			while(UART_recieveByte()!= MC1_ASK_MC2);
			UART_sendByte(MC2_ANS_MC1);
			option=UART_recieveByte();



			if(option == '+')
			{
				state=open_door_req;

			}
			else{
				state=change_pass_req;

			}

			break;

		case change_pass_req:


			for( uint8 i=0;i<5;i++)
			{
				/*RECIVES THE OLD PASS FIRST FROM USER*/
				while(UART_recieveByte()!= MC1_ASK_MC2);
				UART_sendByte(MC2_ANS_MC1);
				pass[i]=UART_recieveByte();/*PUT IT IN ARRAY*/

				/*TAKES OLD PASS FROM EEPROM*/
				EEPROM_readByte(0x0001+i,&repass[i]);
				_delay_ms(15);

				/*IF THEY ARE NOT MATCH PUT FLAG =1*/
				if(pass[i]!=repass[i])
				{
					pass_match_flag=1;

				}

			}

			if(pass_match_flag==1)
			{
				pass_counter++;/*COUNTER INCREAMENT*/
				pass_match_flag=0;/*PUT IT ZERO AGAIN FOR NEXT CHECK*/
				state=change_pass_req;
			}
			else{
				/*IF PASS IS RIGHT GO TO CHANGE IT*/
				state=create_pass;
				pass_counter=0; /*PUT IT ZERO AGAIN FOR NEXT CHECK*/
			}

			if(pass_counter==3){
				state=warning;
				pass_counter=0;/*PUT IT ZERO AGAIN FOR NEXT TIME*/

			}

			break;

		case open_door_req:
			for( uint8 i=0;i<5;i++)
			{
				while(UART_recieveByte()!= MC1_ASK_MC2);
				UART_sendByte(MC2_ANS_MC1);
				pass[i]=UART_recieveByte();
				EEPROM_readByte(0x0001+i, &repass[i]);
				_delay_ms(15);
				/*IF THEY ARE NOT MATCH PUT FLAG =1*/
				if(pass[i]!=repass[i])
				{
					pass_match_flag=1;

				}

			}
			if(pass_match_flag==1)
			{
				pass_counter++;
				pass_match_flag=0;
				state=change_pass_req;

			}
			else{
				/*IF PASS IS RIGHT GO TO OPEN DOOR IT*/
				state=open_door;
				pass_counter=0;
			}

			if(pass_counter==3){
				state=warning;
				pass_counter=0;

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

			/* Create configuration structure for TIMER1 driver */
			Timer1_ConfigType Timer1_Configurations1= {0,1953,F_CPU_1024,COMPARE_MODE};

			Timer1_setCallBack(openCallBackFunction);

			Timer1_init(&Timer1_Configurations1);

			/* waits till the flag returns to zero (to go to option state)*/
			while(wait_flag!=0);
			state=options;
			break;

		case warning:

			wait_flag=1;
			/* Create configuration structure for TIMER1 driver */


			/*
			 * FIRST MAKE THE TIMER INIT BY 0.25 SEC
			 * TO GO TO THE CALL BACK FUNCTION FAST WITHOUT DELAYS
			 * THEN MAKE TIMER INIT TO 5 SEC IN THE CALL BACK FUNCTION
			 */

			Timer1_ConfigType Timer1_Configurations2= {0,1953,F_CPU_1024,COMPARE_MODE};
			Timer1_setCallBack(warnCallBackFunction);

			Timer1_init(&Timer1_Configurations2);
			while(wait_flag!=0);
			state=options;
			break;
		}
	}
}
