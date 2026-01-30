/*
 * Lab03.c
 *
 ;
 ; Universidad del Valle de Guatemala
 ; Departamento de Ingenieria Mecatronica, Electronica y Biomedica
 ; IE2023 Semestre 1 2025
 ;
 ; Created: 29/01/2026 10:21:22
 ; Author : Angie Recinos
 ; Carnet : 23294
 ; Description : SPI MASTER
 */ 

//***************************************************//
// Encabezado (Libraries)
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "SPI/SPI.h"

uint8_t valorSPI = 0;
void refreshPORT(uint8_t valor);

//***************************************************//
// Function prototypes
void setup();


//***************************************************//
// Main function
int main(void)
{
	setup();
    while (1) 
    {
		// Selecciona el slave = 0 (se quiere hablar con el)
		PORTC &= ~(1 << PORTC5); 
		spiWrite('c');
		
		// Dato basura
		spiWrite(0x00); 
		
		valorSPI = spiRead();
		refreshPORT(valorSPI);
		
		PORTC |= (1 << PORTC5); // Slave select = 1
		
		_delay_ms(250);
    }
}


//***************************************************//
// NON-Interrupt subroutines
void setup()
{
	// Puerto de salida para leds (8 bits)
	DDRC |= (1 << DDC5);
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
	DDRB |= (1 << DDB0)	| (1 << DDB1);
	
	// Selecciona el slave
	PORTC &= ~(1 << PORTC5); 
	
	// Puero de salida para leds
	PORTB &= ~((1 << PORTB0) | (1 << PORTB1));
	PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7));
	
	initSPI(SPI_MASTER_OSC_DIV2, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
}

void refreshPORT(uint8_t valor)
{
	if (valor & 0b10000000)
	{
		PORTB |= (1 << PORTB1);
	}else{
		PORTB &= ~(1 << PORTB1);
	}
	if (valor &	0b01000000){
		PORTB |= (1 << PORTB0);
	}
	else{
		PORTB &= ~(1 << PORTB0);
	}
	if (valor &	0b00100000){
		PORTD |= (1 << PORTD7);
	}
	else{
		PORTD &= ~(1 << PORTD7);
	}
	if (valor &	0b00010000){
		PORTD |= (1 << PORTD6);
	}
	else{
		PORTD &= ~(1 << PORTD6);
	}
	if (valor &	0b00001000){
		PORTD |= (1 << PORTD5);
	}
	else{
		PORTD &= ~(1 << PORTD5);
	}
	if (valor &	0b00000100){
		PORTD |= (1 << PORTD4);
	}
	else{
		PORTD &= ~(1 << PORTD4);
	}
	if (valor &	0b00000010){
		PORTD |= (1 << PORTD3);
	}
	else{
		PORTD &= ~(1 << PORTD3);
	}
	if (valor &	0b00000001){
		PORTD |= (1 << PORTD2);
	}
	else{
		PORTD &= ~(1 << PORTD2);
	}
}

//***************************************************//
// Interrupt routines


/////////////////////////////// SLAVEEEEEEEEEEEEEEE ///////////////////////////////
// Revisar SS

// Encabezado (Libraries)
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "SPI/SPI.h"

volatile uint8_t adc_value = 0; 
uint8_t contador = 0; 

void initADC();
void refreshPORT(uint8_t valor);


int main(void)
{
	initSPI(SPI_SLAVE_SS, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
	initADC();
	SPCR |= (1 << SPIE);
	sei();
	while(1)
	{
		ADCSRA |= (1 << ADSC);
		_delay_ms(100);
	}
	
}

void initADC(void)
{
	ADMUX = 0;
	
	// Vref = AVCC = 5V
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);
	
	ADMUX |= (1 << ADLAR);
	
}