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

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000
#include <util/delay.h>
#include <stdint.h>
#include "SPI/SPI.h"
#include "ADC/ADC.h"
#include "UART/UART.h"

uint8_t pot0 = 0;
uint8_t pot1 = 0;
uint8_t centenas;
uint8_t decenas;
uint8_t unidades;
uint8_t centenas2;
uint8_t decenas2;
uint8_t unidades2;
uint8_t option = 0;
uint8_t menu_flag = 1;
uint8_t mostrar_pot = 0;
uint8_t contador = 0;
uint8_t cambio = 0;
uint8_t LEDS = 0;
char temporal;
char buffer[6];
uint8_t buffer_index;
uint8_t uart_flag;


void refreshPORT(uint8_t valor);

//***************************************************//
// Function prototypes
void setup();
void showMenu();
uint8_t processLEDS(char *i);
void showPot();

//***************************************************//
// Main function
int main(void)
{
	setup();
    while (1) 
    {
		
		if (menu_flag)
		{
			showMenu();
		}
		
		if (mostrar_pot)
		{
			showPot();
		}
		if (cambio)
		{
						
			LEDS = processLEDS(&buffer[0]);
			refreshPORT(LEDS);
			
			PORTC &= ~(1 << PORTC5);
			spiWrite('L');
			_delay_ms(50);
			spiWrite(LEDS);
			_delay_ms(50);
			PORTC |= (1 << PORTC5);
			cambio = 0;
			option = 0;
			menu_flag = 1;
		}
		
		_delay_ms(250);
    }
}


//***************************************************//
// NON-Interrupt subroutines
void setup()
{
	cli();
	
	// Puerto de salida para leds (8 bits)
	DDRC |= (1 << DDC5);
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
	DDRB |= (1 << DDB0)	| (1 << DDB1);
	
	// Selecciona el slave
	PORTC &= ~(1 << PORTC5); 
	
	// Puero de salida para leds
	PORTB &= ~((1 << PORTB0) | (1 << PORTB1));
	PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7));
	
	initSPI(SPI_MASTER_OSC_DIV16, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
	initUART();
	
	sei();
	
}

uint8_t spiTransfer(uint8_t data)
{
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));  // espera fin de transferencia
	return SPDR;                   // devuelve lo recibido
}

void showMenu()
{
	sendString("\r\n*** MENU ***\r\n");
	sendString("1: Leer Potenciómetros\r\n");
	sendString("2: Ingresar número\r\n");
	sendString("Seleccione opción: ");
	menu_flag = 0;
}

void showPot()
{
	if (option == 1)
	{
		
		// Selecciona el slave = 0 (se quiere hablar con el)
		PORTC &= ~(1 << PORTC5); 
		
		spiWrite('c');	
		_delay_ms(50);
		spiWrite(0x00);
		_delay_ms(50);
		pot0 = spiRead();
		
		spiWrite('c');
		_delay_ms(50);
		spiWrite(0x00);
		_delay_ms(50);
		pot1 = spiRead();
		
		PORTC |= (1 << PORTC5); // Slave select = 1
		
		// POT 1
		centenas = pot0/100;					// Unicamente se queda la parte entera
		decenas = (pot0 % 100) / 10;			// Se utiliza residuo para obtener decenas
		unidades = pot0 % 10;					// Se utiliza residuo para obtener unidades
		
		sendString("Potenciómetro 1: ");
		writeChar(centenas + '0');
		writeChar(decenas + '0');
		writeChar(unidades + '0');
		sendString(" \r\n");
		
		// POT 2
		centenas2 = (pot1 % 1000) / 100;		// Unicamente se queda la parte entera
		decenas2 = (pot1 % 100) / 10;			// Se utiliza residuo para obtener decenas
		unidades2 = pot1 % 10;					// Se utiliza residuo para obtener unidades
		
		sendString("Potenciómetro 2: ");
		writeChar(centenas2 + '0');
		writeChar(decenas2 + '0');
		writeChar(unidades2 + '0');
		sendString(" \r\n");
		menu_flag = 1;
		option = 0;
		uart_flag = 0;
	}
	
}

uint8_t processLEDS(char* i)
{
	int resultado = 0;	// Se declara e inicializa una variable para guardar el resultado de la conversión
	while (*i >= '0' && *i <= '9')	// Se mantiene en el while siempre que el dígito al que apunte sea un número (termina si detecta \n o \0 por ejemplo)
	{
		resultado = resultado * 10 + (*i - '0');	// Toma el ascii, lo convierte en dígito y lo ordena en sistema decimal
		i++;	// suma al caracter a convertir
	}
	return resultado;	// Se retorna el valor del resultado de la conversión
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
ISR(USART_RX_vect)
{
	temporal = UDR0;					// Se recibe el dato
	writeChar(temporal);					// Eco automático de dato ingresado
	sendString(" \r\n");
	
	if (temporal == '\n') // Si la cadena termina en el caracter de "enter" entra al if
	{
		buffer[buffer_index] = '\0';	// Termina el string
		uart_flag = 1;					// Enciende la bandera de UART
		//option = 0;
		buffer_index = 0;				// Reinicia el índice del buffer
	}
	else
	{
		if (buffer_index < sizeof(buffer) - 1) // Mientras que el índice del caracter recibido sea menor que el tamaño de la lista (Buffer) entra al if
		{
			buffer[buffer_index++] = temporal;	// Guarda en la lista el caracter recibido y suma uno al índice de la lista
		}
	}
	
	if(option == 0 && uart_flag == 1) {
		//uart_flag = 0;
		if(buffer[0] == '1') {
			option = 1;
			sendString("\r\nLeyendo potenciómetros... \r\n");
			mostrar_pot = 1;
			//showPot();
			} else if(buffer[0] == '2') {
			option = 2;
			sendString("\r\nIngrese número deseado ");
			} else {
			sendString("\r\nOpción no valida. Intente nuevamente.\r\n");
			menu_flag = 1;
		}
		} else if(option == 2) {
		//processLEDS(temporal);
		uart_flag = 0;
		cambio = 1;
		//option = 0;
	}
}

