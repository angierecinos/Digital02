/*
 * Lab01.c
 ;
 ;
 ; Universidad del Valle de Guatemala
 ; Departamento de Ingenieria Mecatronica, Electronica y Biomedica
 ; IE2023 Semestre 1 2025
 ;
 ; Created: 15/01/2026 
 ; Author : Angie Recinos
 ; Carnet : 23294
 ; Description: Juego de carreras
 */ 

// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>
#include "DISPLAY7SEG/DISPLAY7SEG.h"
#define F_CPU 16000000

uint8_t contadorJ1 = 0;		// Contador para jugador 1	
uint8_t contadorJ2 = 0;		// Contador para jugador 2
uint8_t digito = 0;			// Conteo inicia en 5... 4... 3...
uint8_t strt_flag = 0;		// Se presiona el botón de inicio
uint8_t ganador = 0;		// Bandera para saber ganador
uint8_t corran = 0;			// Bandera para indicar cada carrera

// Function prototypes
void setup();
void initTMR0();
void winner();

// Main Function
int main(void)
{
	setup();
	initTMR0();
	display_mostrar(0);
	while (1)
	{

	}
}

// NON - Interrupt subroutines
void setup()
{
	cli();
	
	// Configurar prescaler de sistemas
	CLKPR	=	(1 << CLKPCE);								// Habilita cambios en prescaler
	CLKPR	=	(1 << CLKPS2) | (1 << CLKPS1);				// Setea prescaler a 64 para 250kHz
	
	init_dis();												// Inicializa puerto D del display
	DDRB	&= ~(1 << PORTB5);								// PB5, PC0 y PC1 como entradas
	DDRC	&= ~((1 << PORTC0) | (1<< PORTC1));	
	
	DDRC	|= (1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5);	// Leds
	DDRB	|= (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4);	// Leds
	
	// Configurar interrupciones de cambio de pin
	PCICR	|= (1 << PCIE0);								// Habilitar interrupciones para PCINT0 (PB5)
	PCICR	|= (1 << PCIE1);								// Habilitar interrupciones para PCTIN1 (PC)
	PCMSK0	|= (1 << PCINT5);								// Habilitar interrupciones para PB5 (PCINT5)
	PCMSK1	|= (1 << PCINT8) | (1 << PCINT9);				// Habilitar interrupciones para PC0 y PC1
	
	UCSR0B	=	0x00;										// Apaga serial
	
	sei();													// Apaga las interrupciones
	
}

void initTMR0()
{
	TCCR0A	=	0;	TCCR0B |=	(1 << CS02) | (1 << CS00);		// Setear prescaler a 1024	TCNT0	=	134;							// Cargar valor para delay de 0.5s	TIMSK0	=	(1 << TOIE0);
}

void winner()
{
	contadorJ1 = 0;
	contadorJ2 = 0;
	
	if (ganador == 1)
	{
		ganador = 0;
		corran = 0;
		PORTC |= (1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5);
		PORTB &= ~((1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4));
		display_mostrar(1);
	}
	else if (ganador == 2)
	{
		ganador = 0;
		corran = 0;
		PORTB |= (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4);
		PORTC &= ~((1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5));
		display_mostrar(2);
	}
	
}

// Interrupt routines
ISR(TIMER0_OVF_vect)
{
	TCNT0 = 134; 
		
	if (strt_flag)
		{
			digito++;
			
			switch(digito){
				case 1:
				display_mostrar(5);
				break;
				
				case 2:
				display_mostrar(4);
				break;
				
				case 3:
				display_mostrar(3);
				break;
				
				case 4:
				display_mostrar(2);
				break;
				
				case 5:
				display_mostrar(1);
				break;
				
				case 6:
				display_mostrar(0);
				strt_flag = 0; 
				corran = 1;
				break;
			}
		}
		else {
			digito = 0;
		}
		
	}
	
ISR(PCINT0_vect)
{
	if (!(PINB & ( 1 << PORTB5)))
	{
		strt_flag = 1;
		PORTB &= ~((1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4));
		PORTC &= ~((1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5));
	}
}



ISR(PCINT1_vect)
{
	// Botón jugador 1
	if (!(PINC & ( 1 << PORTC0)))
	{
		if (corran)
		{
			contadorJ1++;
		}
		
		if (contadorJ1 < 4 && corran == 1)
		{
			//contadorJ1++;
			// Empieza en PORTC2 porque en PC0 y PC1 hay botones
			PORTC |= (1 << (PORTC2 + contadorJ1 - 1));
		} 
		else if (contadorJ1 == 4 && corran == 1)
		{
			PORTC |= (1 << (PORTC2 + contadorJ1 - 1));
			ganador = 1;
			winner();
		}
		
	}

	// Botón jugador 2
	if (!(PINC & (1 << PORTC1)))
	{
		
		if (corran)
		{
			contadorJ2++;
		}
	
		if (contadorJ2 < 4 && corran == 1)
		{
			PORTB |= (1 << (PORTB1 + contadorJ2 - 1));
		}
		else if (contadorJ2 == 4 && corran == 1 )
		{
			PORTB |= (1 << (PORTB1 + contadorJ2 - 1));
			ganador = 2;
			winner();
		}
	}
}

