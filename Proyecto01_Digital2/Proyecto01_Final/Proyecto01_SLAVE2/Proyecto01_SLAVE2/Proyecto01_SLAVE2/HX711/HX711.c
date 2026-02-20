/*
 * HX711.c
 *
 * Created: 2/13/2026 4:45:18 PM
 *  Author: edvin
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include "HX711.h"

// Función para inicializar los pines del sensor
void initHX711()
{
	DDRD |= (1<<PORTD7);     // SCK salida
	DDRD &= ~(1<<PORTD6);    // DT entrada
	PORTD &= ~(1<<PORTD7);   // SCK LOW
}

uint8_t HX711_IsReady(void)
{
	return !(PIND & (1<<PORTD6));   // DT en LOW = listo
}

int32_t HX711_ReadRaw(void)
{
	int32_t data = 0;

	while(!HX711_IsReady());   // espera dato listo

	for(uint8_t i=0; i<24; i++)
	{
		PORTD |= (1<<PORTD7);      // SCK HIGH
		_delay_us(1);

		data = data << 1;
		if (PIND & (1<<PORTD6))    // <-- leer aquí (SCK HIGH)
		data++;

		PORTD &= ~(1<<PORTD7);     // SCK LOW
		_delay_us(1);
	}


	// Pulso extra para ganancia 128
	PORTD |= (1<<PORTD7);
	_delay_us(1);
	PORTD &= ~(1<<PORTD7);

	// Extender signo
	if(data & 0x800000)
	data |= 0xFF000000;

	return data;
}

int32_t HX711_Tare(uint8_t muestras)
{
	int64_t suma = 0;

	for(uint8_t i=0; i<muestras; i++)
	{
		suma += HX711_ReadRaw();
	}

	return (int32_t)(suma / muestras);
}

/*
uint16_t HX711_ToGrams(int32_t raw, int32_t offset)
{
	int32_t valor = raw - offset;

	if(valor < 0)
	valor = 0;

	return (uint16_t)(valor / scale);
}*/