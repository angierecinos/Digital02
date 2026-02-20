/*
 * PWM.h
 *
 * Created: 2/17/2026 7:25:12 PM
 *  Author: edvin
 */ 


#ifndef PWM_H_
#define PWM_H_

#include <avr/io.h>

#define invert 1
#define non_invert 0

void initPWM2A(uint8_t invertido, uint16_t prescaler);
void initPWM2B(uint8_t invertido, uint16_t prescaler);

uint16_t mapeoADCtoPulse(uint16_t adc_val);
uint16_t mapeoADCtoPulse1(uint16_t adc_val);

void servo_positionA(uint16_t pulse);
void servo_positionB(uint16_t pulse);
void servo_position1A(uint16_t pulse);
void servo_position1B(uint16_t pulse);

void processCoord(char* input);

#endif /* PWM_H_ */