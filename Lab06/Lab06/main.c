/*
 * Lab06.c
 *
 * Created: 12/03/2026 19:13:51
 * Author : Angie Recinos 23294
 */ 

//
// Encabezado (Libraries)
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "UART/UART.h"

// Variables a usar
uint8_t btn_flag = 0;
volatile char boton;

// Function prototypes
void setup();


int main(void)
{
    setup();
	/* Replace with your application code */
    while (1) 
    {
		if (btn_flag == 1)
		{
			btn_flag = 0;
			switch (boton)
			{
			case 'D': 
			sendString("Control _2: Derecha\r\n");
			break;
			
			case 'I':
			sendString("Control _2: Izquierda\r\n");
			break;
			
			case 'Z':
			sendString("Control _2: Abajo\r\n");
			break;
			
			case 'U':
			sendString("Control_2: Arriba\r\n");
			break;
			
			case 'A':
			sendString("Control _2: A\r\n");
			break;
			
			case 'B':
			sendString("Control _2: B\r\n");
			break;
			}
		}
    }
}


//
// NON-Interrupt subroutines
void setup()
{
	cli();
	
	// Configurar prescaler de sistemas
	//CLKPR	= (1 << CLKPCE);					// Habilita cambios en prescaler
	//CLKPR	= (1 << CLKPS2);					// Setea presc a 16 para 1MHz
	
	DDRB  &= ~((1 << PORTB0) | (1 << PORTB1));									// Se setean PC5 y PC0 como entradas
	PORTB |= (1 << PORTB0) | (1 << PORTB1);
	
	DDRC  &= ~((1 << PORTC0) | (1 << PORTC1)| (1 << PORTC2) | (1 << PORTC3));	// Se setean PC5 y PC0 como entradas
	PORTC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3);		// Se habilitan los pull ups internos
	
	//UCSR0B	= 0x00;															// Apaga serial
	
	// -------- PORTC (PC0–PC3) --------
	PCICR	|= (1 << PCIE1) | (1 << PCIE0);										// Se habilitan interrupciones pin-change
	// -------- PORTB (PB0–PB1) --------
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT1);
	PCMSK1	|= (1 << PCINT8) | (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11);	// Se habilitan solo para los PC5 y PC6
	
	
	
	initUART();
	
	sei();
}

//
// Interrupt routines

ISR(PCINT0_vect){
	if (!(PINB & (1 << PORTB0)))						// Se revisa si el botón está presionado
	{
		btn_flag = 1;
		boton = 'B';
	}
	else if (!(PINB & (1 << PORTB1)))					// Se revisa si el botón está presionado
	{
		btn_flag = 1;
		boton = 'A';
	}
}

ISR(PCINT1_vect){
	if (!(PINC & (1 << PORTC0)))						// Se revisa si el botón de modo está presionado
	{
		btn_flag = 1; 
		boton = 'D';
	}
	else if (!(PINC & (1 << PORTC1)))					// Se revisa si el botón de EEPROM está presionado
	{
		btn_flag = 1;
		boton = 'I';
	}
	else if (!(PINC & (1 << PORTC2)))					// Se revisa si el botón de EEPROM está presionado
	{
		btn_flag = 1;
		boton = 'Z';
	}
	else if (!(PINC & (1 << PORTC3)))					// Se revisa si el botón de EEPROM está presionado
	{
		btn_flag = 1;
		boton = 'U';
	}
}

