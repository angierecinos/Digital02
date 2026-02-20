/*
 * PWM.c
 *
 * Created: 2/17/2026 7:25:24 PM
 *  Author: edvin
 */ 

#include "PWM.h"
#include <avr/interrupt.h>

void initPWM0A(uint8_t invertido, uint16_t prescaler)
{
	DDRD	|= (1 << DDD6); // Como salida PD6
	TCCR0A	&= ~((1 << COM0A1) | (1<<COM0A0));
	
	if (invertido == invert)
	{
		TCCR0A	|= (1 << COM0A1) | (1 << COM0A0); //Invertido
		} else {
		TCCR0A	|= (1 << COM0A1);
	}
	
	TCCR0A	|= (1 << WGM01) | (1 << WGM00); // Modo 3 -> Fast PWM y top 0xFF;
	
	TCCR0B	&= ~((1 << CS02) | (1<<CS01) | (1<<CS00));
	switch(prescaler){
		case 1:
		TCCR0B	|= (1 << CS00); // Presc de 256 segun excel
		break;
		case 8:
		TCCR0B	|= (1 << CS01); // Presc de 256 segun excel
		break;
		case 64:
		TCCR0B	|= (1 << CS01) | (1 << CS00); // Presc de 256 segun excel
		break;
		case 256:
		TCCR0B	|= (1 << CS02); // Presc de 256 segun excel
		break;
		case 1024:
		TCCR0B	|= (1 << CS02) | (1<< CS00); // Presc de 256 segun excel
		break;
	}
	
	//OCR0A = 127;             // Interrupción cada 125kHz / (124 + 1) = 1 kHz (1 ms)

	//TIMSK0 |= (1 << TOIE0); // Habilita la interrupción del timer
}

void initPWM0B(uint8_t invertido, uint16_t prescaler)
{
	DDRD	|= (1 << DDD5); // Como salida PD6
	TCCR0A	&= ~((1 << COM0B1) | (1<<COM0B0));
	
	if (invertido == invert)
	{
		TCCR0A	|= (1 << COM0B1) | (1 << COM0B0); //Invertido
		} else {
		TCCR0A	|= (1 << COM0B1);
	}
	
	TCCR0A	|= (1 << WGM01) | (1 << WGM00); // Modo 3 -> Fast PWM y top 0xFF;
	
	TCCR0B	&= ~((1 << CS02) | (1<<CS01) | (1<<CS00));
	switch(prescaler){
		case 1:
		TCCR0B	|= (1 << CS00); // Presc de 256 segun excel
		break;
		case 8:
		TCCR0B	|= (1 << CS01); // Presc de 256 segun excel
		break;
		case 64:
		TCCR0B	|= (1 << CS01) | (1 << CS00); // Presc de 256 segun excel
		break;
		case 256:
		TCCR0B	|= (1 << CS02); // Presc de 256 segun excel
		break;
		case 1024:
		TCCR0B	|= (1 << CS02) | (1<< CS00); // Presc de 256 segun excel
		break;
	}
	
	//OCR0A = 127;             // Interrupción cada 125kHz / (124 + 1) = 1 kHz (1 ms)

	//TIMSK0 |= (1 << TOIE0); // Habilita la interrupción del timer
}

/*uint8_t pwm_counter = 0;*/
//uint8_t pwm = 0;

void initPWM1A(uint8_t invertido, uint16_t prescaler)
{
	ICR1	 = 39999;									// Será mi valor TOP
	DDRB	|= (1 << DDB1);							// Como salida PB1
	
	TCCR1A	&= ~((1 << COM1A1) | (1<<COM1A0));			// Unicamente quiero modificar estos 2
	
	if (invertido == invert)
	{
		TCCR1A	|= (1 << COM1A1) | (1 << COM1A0);		// Invertido
		} else {
		TCCR1A	|= (1 << COM1A1);						// No invertido
	}
	
	TCCR1A	|= (1 << WGM11);							// Modo 14 -> Fast PWM y top ICR1
	TCCR1B	|= (1 << WGM13) | (1 << WGM12);				// Modo 14 -> Fast PWM y top ICR1
	TCCR1B	&= ~((1 << CS12) | (1<<CS11) | (1<<CS10));	// Apago únicamente los necesarios
	
	switch(prescaler){
		case 1:
		TCCR1B	|= (1 << CS10);
		break;
		case 8:
		TCCR1B	|= (1 << CS11);
		break;
		case 64:
		TCCR1B	|= (1 << CS11) | (1 << CS10);
		break;
		case 256:
		TCCR1B	|= (1 << CS12);							// Presc de 256 segun excel
		break;
		case 1024:
		TCCR1B	|= (1 << CS12) | (1<< CS10);			// Presc de 256 segun excel
		break;
		default:
		break;
	}
}

void initPWM1B(uint8_t invertido, uint16_t prescaler)
{
	
	ICR1	 = 39999;
	DDRB	|= (1 << DDB2);								// Como salida PB2
	TCCR1A	&= ~((1 << COM1B1) | (1<<COM1B0));			// Unicamente quiero modificar estos 2
	
	if (invertido == invert)
	{
		TCCR1A	|= (1 << COM1B1) | (1 << COM1B0);		// Invertido
		} else {
		TCCR1A	|= (1 << COM1B1);						// No invertido
	}
	
	TCCR1A	|= (1 << WGM11);							// Modo 14 -> Fast PWM y top ICR1
	TCCR1B	|= (1 << WGM13) | (1 << WGM12);				// Modo 14 -> Fast PWM y top ICR1
	TCCR1B	&= ~((1 << CS12) | (1<<CS11) | (1<<CS10));  // Apago para encender los necesarios
	
	switch(prescaler){
		case 1:
		TCCR1B	|= (1 << CS10);
		break;
		case 8:
		TCCR1B	|= (1 << CS11);
		break;
		case 64:
		TCCR1B	|= (1 << CS11) | (1 << CS10);
		break;
		case 256:
		TCCR1B	|= (1 << CS12);
		break;
		case 1024:
		TCCR1B	|= (1 << CS12) | (1<< CS10);
		break;
		default:
		break;
	}
	
}

void initPWM2A(uint8_t invertido, uint16_t prescaler)
{
	DDRB	|= (1 << DDB3); // Como salida PB3 (OC2A)
	TCCR2A	&= ~((1 << COM2A1) | (1<<COM2A0));
	
	if (invertido == invert)
	{
		TCCR2A	|= (1 << COM2A1) | (1 << COM2A0); //Invertido
		} else {
		TCCR2A	|= (1 << COM2A1);
	}
	
	TCCR2A	&= ~(1 << WGM21);	// Modo 1 -> Phase Correct PWM, TOP=0xFF
	TCCR2A	|=  (1 << WGM20);
	TCCR2B	&= ~(1 << WGM22);
	
	TCCR2B	&= ~((1 << CS22) | (1<<CS21) | (1<<CS20));
	switch(prescaler){
		case 1:
		TCCR2B	|= (1 << CS20); // Prescaler 1
		break;
		case 8:
		TCCR2B	|= (1 << CS21); // Prescaler 8
		break;
		case 32:
		TCCR2B	|= (1 << CS21) | (1 << CS20); // Prescaler 32 (solo Timer2)
		break;
		case 64:
		TCCR2B	|= (1 << CS22); // Prescaler 64
		break;
		case 128:
		TCCR2B	|= (1 << CS22) | (1 << CS20); // Prescaler 128 (solo Timer2)
		break;
		case 256:
		TCCR2B	|= (1 << CS22) | (1 << CS21); // Prescaler 256
		break;
		case 1024:
		TCCR2B	|= (1 << CS22) | (1 << CS21) | (1 << CS20); // Prescaler 1024
		break;
	}
	
	//OCR2A = 127;             // Duty inicial (0-255)
	//TIMSK2 |= (1 << TOIE2);  // Habilita la interrupción del timer (si se usa)
}

void initPWM2B(uint8_t invertido, uint16_t prescaler)
{
	DDRD	|= (1 << DDD3); // Como salida PD3 (OC2B)
	TCCR2A	&= ~((1 << COM2B1) | (1<<COM2B0));
	
	if (invertido == invert)
	{
		TCCR2A	|= (1 << COM2B1) | (1 << COM2B0); //Invertido
		} else {
		TCCR2A	|= (1 << COM2B1);
	}
	
	TCCR2A	&= ~(1 << WGM21);	// Modo 1 -> Phase Correct PWM, TOP=0xFF
	TCCR2A	|=  (1 << WGM20);
	TCCR2B	&= ~(1 << WGM22);
	
	TCCR2B	&= ~((1 << CS22) | (1<<CS21) | (1<<CS20));
	switch(prescaler){
		case 1:
		TCCR2B	|= (1 << CS20); // Prescaler 1
		break;
		case 8:
		TCCR2B	|= (1 << CS21); // Prescaler 8
		break;
		case 32:
		TCCR2B	|= (1 << CS21) | (1 << CS20); // Prescaler 32 (solo Timer2)
		break;
		case 64:
		TCCR2B	|= (1 << CS22); // Prescaler 64
		break;
		case 128:
		TCCR2B	|= (1 << CS22) | (1 << CS20); // Prescaler 128 (solo Timer2)
		break;
		case 256:
		TCCR2B	|= (1 << CS22) | (1 << CS21); // Prescaler 256
		break;
		case 1024:
		TCCR2B	|= (1 << CS22) | (1 << CS21) | (1 << CS20); // Prescaler 1024
		break;
	}
	
	//OCR2B = 127;             // Duty inicial (0-255)
	//TIMSK2 |= (1 << TOIE2);  // Habilita la interrupción del timer (si se usa)
}

void servo_positionA(uint16_t angulo)
{

	OCR2A = 5 + (angulo * (37-5)/180);
	//OCR0A = 5 + (angulo * (37-5)/180);


}

void servo_positionB(uint16_t angulo)
{
	OCR2B =  16 + (angulo*(31-16)/180);
}

void servo_position1A(uint16_t angulo)
{
	OCR1A = 1500 + (angulo * (3500UL) / 180);
	//OCR1A = 2000 + (angulo * (4000-2000) / 180);
	//OCR1A = 125 + (angulo * (250-125) / 180);
}

void servo_position1B(uint16_t angulo)
{
	OCR1B =  125 + (angulo*(230-125)/180);
}

uint16_t mapeoADCtoPulse(uint16_t adc_val)
{
	return ((adc_val * 180) / 255);		// Escalar 0-255 a 125-250
}

uint16_t mapeoADCtoPulse1(uint16_t adc_val)
{
	return ((adc_val * 180) / 255);		// Escalar 0-255 a 125-250
}
