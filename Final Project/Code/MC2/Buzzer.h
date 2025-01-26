/*
 * Buzzer.h
 *
 *  Created on: 30 Oct 2023
 *      Author: Steven
 */

#ifndef HAL_BUZZER_H_
#define HAL_BUZZER_H_

#include "std_types.h"

#define BUZZER_PORT_ID  PORTA_ID

#define BUZZER_PIN_ID  PIN2_ID

void Buzzer_init(void);
void Buzzer_on(void);
void Buzzer_off(void);
#endif /* HAL_BUZZER_H_ */
