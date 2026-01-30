/*
 * UART.h
 *
 * Created: 25/01/2026 17:55:20
 *  Author: Usuario
 */ 


#ifndef UART_H_
#define UART_H_
#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void initUART();
void writeChar(char caracter);
void sendString(char* texto);

#endif /* UART_H_ */