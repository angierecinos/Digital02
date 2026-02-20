/*
 * UART.h
 *
 * Created: 1/28/2026 1:54:00 AM
 *  Author: edvin
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

#ifndef UART_H_
#define UART_H_

void initUART();

void writeChar(char caracter);

void writeString (char* cadena);

void writeLong(int32_t valor);

uint8_t ascii_to_int(char* i);

void enviar_valor_uart(uint16_t valor, char *prefijo);

void enviar_valor_uart16b(uint16_t valor, char *prefijo);

#endif /* UART_H_ */