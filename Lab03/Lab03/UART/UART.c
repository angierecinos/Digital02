/*
 * UART.c
 *
 * Created: 25/01/2026 17:55:05
 *  Author: Usuario
 */ 

#include "UART.h"

void initUART()
{
	// Configurar PD0 y PD1
	DDRD |=  (1 << DDD1);
	DDRD &= ~(1 << DDD0);
	
	// Se apaga (no utilizo doble velocidad)
	UCSR0A = 0;
	// Habilitar interrupts recibir, recepcion y transmision
	UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
	// Modo asíncrono y con paridad deshabilitada | Quiero 8 bits
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	// Valor UBRR = 103 -> 9600 @ 16MHz
	UBRR0 = 103;
}

void writeChar(char caracter)
{
	// Si no se hubiera esperado a que termine de trasladar, puede dar un error
	while ((UCSR0A & (1 << UDRE0)) == 0);
	UDR0 = caracter;
	
}

void sendString(char* texto)
{
	// Se hace siempre que indice sea diferente de un valor "nulo"
	// Aumenta el valor de indice
	for (uint8_t indice = 0; *(texto + indice) != '\0'; indice++)
	{
		writeChar(texto[indice]);
	}
}
