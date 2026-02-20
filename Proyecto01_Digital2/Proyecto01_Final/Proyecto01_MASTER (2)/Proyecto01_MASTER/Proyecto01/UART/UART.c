/*
 * UART.c
 *
 * Created: 1/28/2026 1:53:49 AM
 *  Author: edvin
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

void initUART()
{
	// Paso 1: Configurar pines PD0 (rx) y PD1 (tx)
	DDRD	|= (1 << DDD1);
	DDRD	&= ~(1 << DDD0);
	
	// Paso 2: Configurar UCSR0A
	UCSR0A = 0;
	
	// Paso 3: Configurar UCSR0B, Habilitando interrupts al recibir; Habilitando recepción; Habilitando transmisión
	UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
	
	// Paso 4: UCSR0C
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
	
	// Paso 5: UBRR0: UBRR0 = 103 -> 9600 @ 16MHz
	UBRR0 = 103;
}

void writeChar(char caracter)
{
	while ((UCSR0A & (1 << UDRE0)) == 0);  // Esperar a que el registro de datos esté vacío
	UDR0 = caracter;  // Enviar el caracter al registro de UART
}

void writeString (char* cadena)
{
	for (uint8_t puntero = 0; *(cadena+puntero) != '\0'; puntero++)  // Itera sobre cada carácter de la cadena
	{
		writeChar(*(cadena+puntero));  // Enviar cada carácter a través de UART
	}
}

void writeLong(int32_t valor)
{
	char buffer[12];          // suficiente para int32_t
	ltoa(valor, buffer, 10);  // convierte a base 10
	writeString(buffer);
}

uint8_t ascii_to_int(char* i)	// Función para convertir de ascii a int
{
	int resultado = 0;	// Se declara e inicializa una variable para guardar el resultado de la conversión
	while (*i >= '0' && *i <= '9')	// Se mantiene en el while siempre que el dígito al que apunte sea un número (termina si detecta \n o \0 por ejemplo)
	{
		resultado = resultado * 10 + (*i - '0');	// Toma el ascii, lo convierte en dígito y lo ordena en sistema decimal
		i++;	// suma al caracter a convertir
	}
	return resultado;	// Se retorna el valor del resultado de la conversión
}

void enviar_valor_uart(uint16_t valor, char *prefijo)	// Función para enciar UART (Feedback)
{
	uint8_t update[3];				// Se declara una lista para guardar el ascii convertido
	update[0] = valor % 10;			// Se separan las unidades
	update[1] = (valor % 100) / 10;	// Se separan las decenas
	update[2] = valor / 100;		// Se separan las centenas

	writeString(prefijo);		// Se manda el prefijo para interpretación usando escribir_cadena
	writeChar(update[2] + '0');		// Se envían las centenas traducidas a ascii
	writeChar(update[1] + '0');		// Se envían las decenas traducidas a ascii
	writeChar(update[0] + '0');		// Se envían las unidades traducidas a ascii
}

void enviar_valor_uart16b(uint16_t valor, char *prefijo)	// Función para enciar UART (Feedback)
{
	uint8_t update[4];				// Se declara una lista para guardar el ascii convertido
	update[0] = valor % 10;			// Se separan las unidades
	update[1] = (valor % 100) / 10;	// Se separan las decenas
	update[2] = (valor % 1000) / 100;		// Se separan las centenas
	update[3] = valor / 1000;
	

	writeString(prefijo);		// Se manda el prefijo para interpretación usando escribir_cadena
	writeChar(update[3] + '0');		// Se envían los millares traducidos a ascii
	writeChar(update[2] + '0');		// Se envían las centenas traducidas a ascii
	writeChar(update[1] + '0');		// Se envían las decenas traducidas a ascii
	writeChar(update[0] + '0');		// Se envían las unidades traducidas a ascii
}