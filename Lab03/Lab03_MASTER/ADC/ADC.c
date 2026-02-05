/*
 * ADC.c
 *
 * Created: 22/01/2026 01:23:40
 *  Author: Angie
 */ 

#include "ADC.h"

void initADC()
{
	ADMUX	= 0;
	ADMUX	|= (1 << REFS0);					//ADMUX &= ~(1<< REFS1); // Se ponen los 5V como ref
	
	ADMUX	|= (1 << ADLAR);					// Justificación a la izquierda
	ADMUX	|= (1 << MUX1); //| (1<< MUX0);		// Seleccionar el ADC2
	ADCSRA	= 0;
	ADCSRA	|= (1 << ADPS1) | (1 << ADPS0);		// Frecuencia de muestreo de 125kHz
	ADCSRA	|= (1 << ADIE);						// Hab interrupción
	ADCSRA	|= (1 << ADEN);
}
