/*
 * Display7seg.c
 *
 * Created: 18/01/2026 15:14:10
 *  Author: Usuario
 */ 


#include "DISPLAY7SEG.h"

int tabla_7seg[16] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0X5F, 0x70, 0x7F, 0X7B, 0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47};

void init_dis(void)
{
	DDRD	=	0xFF;										// PORTD como salida para los segmentos del display
	PORTD	=	0x00;										// Apagar salidas
}

void display_mostrar(uint8_t numero)
{
	PORTD = tabla_7seg[numero];
					
}