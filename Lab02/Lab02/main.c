/*
 * Lab02.c
 ;
 ;
 ; Universidad del Valle de Guatemala
 ; Departamento de Ingenieria Mecatronica, Electronica y Biomedica
 ; IE2023 Semestre 1 2025
 ;
 ; Created: 22/01/2026 
 ; Author : Angie Recinos
 ; Carnet : 23294
 ; Description: LCD
 */ 


// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000
#include <util/delay.h>
#include "ADC/ADC.h"
#include "LCD/LCD.h"
#include "UART/UART.h"

uint32_t eleccion_adc = 0;
uint32_t lectura_adc; 
uint32_t voltaje2;
uint32_t voltaje3;
uint32_t milestimas;
uint32_t centenas;
uint32_t decenas;
uint32_t unidades;
uint32_t centenas3;
uint32_t decenas3;
uint32_t unidades3;
uint8_t cambio = 0;
uint8_t refreshLCD = 0;
uint8_t option = 0;
uint8_t menu_flag = 1; 
uint8_t contador = 0;
char temporal;

// Function prototypes
void setup();
void showMenu();
void processCounter(char counter);
void counter_ascii(uint8_t num);
void showPot();

// Main function
int main(void)
{
    setup();
	
	LCD_Cursor8BD(2,1);
	LCD_String8BD("S1:");
	LCD_Cursor8BD(7,1);
	LCD_String8BD("S2:");
	LCD_Cursor8BD(12,1);
	LCD_String8BD("S3:");
    while (1) 
    {
		//if (refreshLCD && cambio)
		if (cambio)
		{
			//LCD_Cursor8(1,2);
			//LCD_String8("     ");   // limpia
			LCD_Cursor8BD(1,2);
			
			// Regla de tres
			//voltaje = (lectura_adc * 500)/255;
			
			// Convertir a ASCII y enviar dígito por dígito
			centenas = voltaje2/100;					// Unicamente se queda la parte entera
			decenas = (voltaje2 % 100) / 10;			// Se utiliza residuo para obtener decenas
			unidades = voltaje2 % 10;				// Se utiliza residuo para obtener unidades
			
			LCD_Char8BD(centenas + '0');
			LCD_Char8BD('.');
			LCD_Char8BD(decenas + '0');
			LCD_Char8BD(unidades + '0');
			LCD_Char8BD('V');
			
			LCD_Cursor8BD(7,2);
			
			// Regla de tres
			//voltaje = (lectura_adc * 500)/255;
			
			// Convertir a ASCII y enviar dígito por dígito
			milestimas = voltaje3/1000;
			centenas3 = (voltaje3 % 1000) / 100;					// Unicamente se queda la parte entera
			decenas3 = (voltaje3 % 100) / 10;			// Se utiliza residuo para obtener decenas
			unidades3 = voltaje3 % 10;					// Se utiliza residuo para obtener unidades
			
			LCD_Char8BD(milestimas + '0');
			LCD_Char8BD(centenas3 + '0');
			LCD_Char8BD(decenas3 + '0');
			LCD_Char8BD(unidades3 + '0');

			cambio = 0;
			refreshLCD = 0;
		}
		
		if (menu_flag)
		{
			showMenu();
			LCD_Cursor8BD(12,2);
			LCD_Char8BD((contador / 10) + '0');
			LCD_Char8BD((contador % 10) + '0');
			
		}
		
    }
}


// NON-Interrupt subroutines
void setup()
{
	cli();
	
	initLCD_8bitsBD();
	initUART();
	initADC();
	ADCSRA |= (1 << ADSC);
	
	sei();
}


void showMenu()
{
	sendString("\r\n*** MENU ***\r\n");
	sendString("1: Leer Potenciómetros\r\n");
	sendString("2: Incrementar contador\r\n");
	sendString("Seleccione opción: ");
	menu_flag = 0;
}

void showPot()
{
	if (option == 1)
	{
		// Convertir a ASCII y enviar dígito por dígito
		// POT 1
		centenas = voltaje2/100;					// Unicamente se queda la parte entera
		decenas = (voltaje2 % 100) / 10;			// Se utiliza residuo para obtener decenas
		unidades = voltaje2 % 10;					// Se utiliza residuo para obtener unidades
		
		sendString("Potenciómetro 1: ");
		writeChar(centenas + '0');
		writeChar('.');
		writeChar(decenas + '0');
		writeChar(unidades + '0');
		writeChar('V');
		sendString(" \r\n");
		
		// POT2
		milestimas = voltaje3/1000;
		centenas3 = (voltaje3 % 1000) / 100;					// Unicamente se queda la parte entera
		decenas3 = (voltaje3 % 100) / 10;			// Se utiliza residuo para obtener decenas
		unidades3 = voltaje3 % 10;					// Se utiliza residuo para obtener unidades
		
		sendString("Potenciómetro 2: ");
		writeChar(milestimas + '0');
		writeChar(centenas3 + '0');
		writeChar(decenas3 + '0');
		writeChar(unidades3 + '0');
		sendString(" \r\n");
		menu_flag = 1;
		option = 0;

	}
}

void processCounter(char counter)
{
	
	if (counter == '+')
	{
		if (contador < 50)
			contador++;
		else 
			contador = 0;
	}
	
	else if (counter == '-')
	{
		if (contador > 0)
			contador--;
		else
			contador = 50;
	}
	counter_ascii(contador);
	menu_flag = 1;
}

void counter_ascii(uint8_t num)
{
	sendString("Contador: ");
	if (num >= 10)
	{
		writeChar((num / 10) + '0');
	}
		writeChar((num % 10) + '0');
		sendString("\r\n");
		
}

//*******************************************
// Interrupt routines

ISR(USART_RX_vect)
{
	
	temporal = UDR0;					// Se recibe el dato
	writeChar(temporal);					// Eco automático de dato ingresado
	sendString(" \r\n");
	
	if(option == 0) {
		
		if(temporal == '1') {
			option = 1;
			sendString("\r\nLeyendo potenciómetros... \r\n");
			showPot();
			} else if(temporal == '2') {
			option = 2;
			sendString("\r\nIncremente (+) o decremente (-) contador ");
			} else {
			sendString("\r\nOpción no valida. Intente nuevamente.\r\n");
			menu_flag = 1;
		}
		} else if(option == 2) {
		processCounter(temporal);
		option = 0;
		}
}


ISR(ADC_vect)
{
	eleccion_adc = ADMUX & 0x03;
	
	lectura_adc	= ADCH;
	
	switch(eleccion_adc)
	{
		case 2:
			ADMUX	&= 0xF0;
			voltaje2 = (lectura_adc * 500)/255;			
			ADMUX  |= (1 << MUX1) | (1<< MUX0);
			break;
			
		case 3:
			ADMUX	&= 0xF0;
			voltaje3 = (lectura_adc * 1023)/255;
			ADMUX  |= (1 << MUX1);
			break;
			
		default:
			break;
			
	}
	
	//option = 0;
	cambio = 1;
	
	ADCSRA |= (1 << ADSC);
}

