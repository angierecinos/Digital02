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
#define F_CPU 16000000

uint8_t contadorJ1 = 0;		// Contador para jugador 1	
uint8_t contadorJ2 = 0;		// Contador para jugador 2
uint8_t digito = 0;			// Conteo inicia en 5... 4... 3...
uint8_t strt_flag = 0;		// Se presiona el botón de inicio

int tabla_7seg[16] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0X5F, 0x70, 0x7F, 0X7B, 0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47};

// Function prototypes
void setup();
void initTMR0();

// Main Function
int main(void)
{
	setup();
	initTMR0();
	PORTD = tabla_7seg[0];
	/* Replace with your application code */
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
	
	
	DDRB	&= ~(1 << PORTB5);								// PB5, PC0 y PC1 como entradas
	DDRC	&= ~((1 << PORTC0) | (1<< PORTC1));	
	
	DDRD	=	0xFF;										// PORTD como salida para los segmentos del display
	PORTD	=	0x00;										// Apagar salidas
	
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

// Interrupt routines
ISR(TIMER0_OVF_vect)
{
	TCNT0 = 134; 
		
	if (strt_flag)
		{
			digito++;
			
			switch(digito){
				case 1:
				PORTD = tabla_7seg[5];
				break;
				
				case 2:
				PORTD = tabla_7seg[4];
				break;
				
				case 3:
				PORTD = tabla_7seg[3];
				break;
				
				case 4:
				PORTD = tabla_7seg[2];
				break;
				
				case 5:
				PORTD = tabla_7seg[1];
				break;
				
				case 6:
				PORTD = tabla_7seg[0];
				strt_flag = 0; 
				break;
			}
		}
		else {
			digito = 0;
			PORTD = tabla_7seg[0];
		}
		
	}
	
ISR(PCINT0_vect)
{
	if (!(PINB & ( 1 << PORTB5)))
	{
		strt_flag = 1;
	}
}



ISR(PCINT1_vect)
{
	if (!(PINC & ( 1 << PORTC0)))
	{
		if (contadorJ1 > 16)
		{
			contadorJ1 = 0;
			PORTC = (PORTC & 0x03) | (contadorJ1 << 2);
		}
		else 
		{
			contadorJ1++;
			PORTB = (PORTB & 0x03) | (contadorJ1 << 2);
		}
	}
	if (!(PINC & (1 << PORTC1)))
	{
		contadorJ2++;
	}
}

