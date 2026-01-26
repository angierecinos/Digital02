/*
 * LCD.c
 *
 * Created: 22/01/2026 
 *  Author: Angie
 */ 

#include "LCD.h"

// Funcion para inicializar LCD en 4 bits
void initLCD_4bits(void)
{
	DDRC |= (1 << DDC0) | (1 << DDC1) | (1 << DDC2) | (1 << DDC3) | (1 << DDC4) | (1 << DDC5);
	PORTC = 0;
	
	LCD_Port4(0x00);
	_delay_ms(20);		// Necesito al menos 15 ms para que se reinicie la LCD
	LCD_Comando(0X03);	 
	_delay_ms(5);		// Espero al menos 5 ms
	LCD_Comando(0x03);
	_delay_ms(11);
	LCD_Comando(0x03);
	
	LCD_Comando(0x02);
	
	// Function set
	LCD_Comando(0x02);
	LCD_Comando(0x08);
	
	// Display On/Off
	LCD_Comando(0x00);
	LCD_Comando(0x0C);
	
	// Entry mode
	LCD_Comando(0x00);
	LCD_Comando(0x06);
	
	// Clear display
	LCD_Comando(0x00);
	LCD_Comando(0x01);
	
}

// Funcion para inicializar LCD en 8 bits
void initLCD_8bits(void)
{
	DDRC |= (1<<PORTC0) | (1<<PORTC1);		// E y RS
	DDRD |= 0xFF;							// En PORTD D0-D7
	PORTD = 0x00;							// Se envían 8 bits juntos
	
	_delay_ms(20);							// Necesito al menos 15 ms para que se reinicie la LCD
	
	// Function set
	LCD_Comando8(0x00);
	LCD_Comando8(0x38);
	
	// Display On/Off
	LCD_Comando8(0x00);
	LCD_Comando8(0x0C);
	
	// Entry mode
	LCD_Comando8(0x00);
	LCD_Comando8(0x06);
	
	// Clear display
	LCD_Comando8(0x00);
	LCD_Comando8(0x01);
	
}


// Funcion para enviar un comando
void LCD_Comando4(char a)
{
	// RS = 0 => LCD lo interpreta como comando
	PORTC &= ~(1 << PORTC1);
	LCD_Port4(a);
	
	// EN = 1
	PORTC |= (1 << PORTC0);
	_delay_ms(4);
	
	// EN = 0
	PORTC &= ~(1 << PORTC0);
}
// Funcion para enviar un comando en 8 bits
void LCD_Comando8(char a)
{
	// RS = 0 => LCD lo interpreta como comando
	PORTC &= ~(1 << PORTC1);
	PORTD = a;
	
	// EN = 1
	PORTC |= (1 << PORTC0);
	_delay_ms(4);
	
	// EN = 0
	PORTC &= ~(1 << PORTC0);
}


// Funcion para escribir un valor de 4 bits
void LCD_Port4(char valor)
{
	if (valor & 1)
	// D4 = 1
	PORTC |= (1 << PORTC2);
	else
	// D4 = 0
	PORTC &= ~(1 << PORTC2);
	
	if (valor & 2)
	// D5 = 1
	PORTC |= (1 << PORTC3);
	else
	// D5 = 0
	PORTC &= ~(1 << PORTC3);
	
	if (valor & 4)
	// D6 = 1
	PORTC |= (1 << PORTC4);
	else
	// D6 = 0
	PORTC &= ~(1 << PORTC4);
	
	if (valor & 8)
	// D7 = 1
	PORTC |= (1 << PORTC5);
	else
	// D7 = 0
	PORTC &= ~(1 << PORTC5);
	
}


// Funcion para enviar un caracter
void LCD_Char4(char caracter)
{
	
	char Cbajo, Calto;
	Cbajo = caracter & 0x0F;
	Calto = (caracter & 0xF0) >> 4; 
	
	// RS = 1 => Dato interpretado como dato
	PORTC |= (1 << PORTC1);
	LCD_Port4(Calto);
	
	// EN = 1
	PORTC |= (1 << PORTC0);
	_delay_ms(4);
	
	// EN = 0
	PORTC &= ~(1 << PORTC0); // Lee la LCD
	LCD_Port4(Cbajo);
	
	// EN = 1
	PORTC |= (1 << PORTC0);
	_delay_ms(4);
	
	// EN = 0
	PORTC &= ~(1 << PORTC0); // Lee la LCD
	
}
// Funcion para enviar un caracter
void LCD_Char8(char caracter)
{
	
	// RS = 1 => Dato interpretado como dato
	PORTC |= (1 << PORTC1);
	PORTD = caracter;
	
	// EN = 1
	PORTC |= (1 << PORTC0);
	_delay_ms(4);
	
	// EN = 0
	PORTC &= ~(1 << PORTC0); // Lee la LCD
	
}


// Funcion para enviar una cadena 4 bits
void LCD_String4(char *a)
{
	int i; 
	for (i = 0; a[i] != '\0'; i++)
	LCD_Char4(a[i]);
}
// Funcion para enviar una cadena 8 bits
void LCD_String8(char *a)
{
	int i;
	for (i = 0; a[i] != '\0'; i++)
	LCD_Char8(a[i]);
}


// Funcion para correr a la izquierda 4 bits
void LCD_Shift_Right4(void)
{
	LCD_Comando(0x01);
	LCD_Comando(0x0C);
}
// Funcion para correr a la izquierda 8 bits
void LCD_Shift_Right8(void)
{
	LCD_Comando8(0x01);
	LCD_Comando8(0x0C);
}


// Funcion para correr a la derecha 4 bits
void LCD_Shift_Left4(void)
{
	LCD_Comando(0x01);
	LCD_Comando(0x08);
}
// Funcion para correr a la derecha 8 bits
void LCD_Shift_Left8(void)
{
	LCD_Comando8(0x01);
	LCD_Comando8(0x08);
}

// Funcion para el cursor 4 bits
void LCD_Cursor4(char c, char f)
{
	char temp, talto, tbajo;
	if (f == 1)
	{
		temp = 0x80 + c - 1;
		talto = (temp & 0xF0) >> 4;
		tbajo = temp & 0x0F; 
		LCD_Comando(talto);
		LCD_Comando(tbajo);		
	}
	else if (f == 2)
	{
		temp = 0xC0 + c - 1;
		talto = (temp & 0xF0) >> 4;
		tbajo = temp & 0x0F;
		LCD_Comando(talto);
		LCD_Comando(tbajo);
	}
	
}
// Funcion para el cursor 8 bits
void LCD_Cursor8(char c, char f)
{
	if (f == 1)
	{
		LCD_Comando8(0x80 + c - 1);
	}
	else if (f == 2)
	{
		LCD_Comando8(0xC0 + c - 1);
	}
	
}

////////////////////////////////////// 8 bits (PORTD y PORTB) //////////////////////////////////////
// Funcion para enviar un comando
void LCD_Comando8BD(char a)
{
	// RS = 0 => LCD lo interpreta como comando
	PORTC &= ~(1 << PORTC1);
	LCD_Port8_Split(a);
	
	// EN = 1
	PORTC |= (1 << PORTC0);
	_delay_ms(4);
	
	// EN = 0
	PORTC &= ~(1 << PORTC0);
	//_delay_ms(4);
}

// Funcion para inicializar LCD en 8 bits
void initLCD_8bitsBD(void)
{
	// E y RS
	DDRC |= (1<<PORTC0) | (1<<PORTC1);		
	// En PORTD D2-D7
	DDRD |=(1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD5) | (1<<DDD6) | (1<<DDD7);
	// En PORTB B0-B1
	DDRB |=(1<<DDB0) | (1<<DDB1);
	
	PORTB &= ~((1<<DDB0) | (1<<DDB1));
	PORTD &= ~((1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD5) | (1<<DDD6) | (1<<DDD7));							
	
	_delay_ms(20);							// Necesito al menos 15 ms para que se reinicie la LCD
	
	LCD_Comando8BD(0x30);
	_delay_ms(5);
	LCD_Comando8BD(0x30);
	_delay_ms(1);
	LCD_Comando8BD(0x30);
	_delay_ms(1);
	
	// Function set
	LCD_Comando8BD(0x38);
	_delay_us(100);
	
	// Display On/Off
	LCD_Comando8BD(0x00);
	_delay_us(100);
	LCD_Comando8BD(0x0C);
	_delay_us(100);
	
	// Entry mode
	LCD_Comando8BD(0x06);
	_delay_us(100);
	
	// Clear display
	LCD_Comando8BD(0x01);
	_delay_ms(2);
	
	
}

// Funcion para escribir en 2 puertos
void LCD_Port8_Split(char a)
{
	if(a & 1)
	PORTD |= (1<<PORTD2);
	else
	PORTD &= ~(1<<PORTD2);
	
	if(a & 2)
	PORTD |= (1<<PORTD3);
	else
	PORTD &= ~(1<<PORTD3);
	
	if(a & 4)
	PORTD |= (1<<PORTD4);
	else
	PORTD &= ~(1<<PORTD4);
	
	if(a & 8)
	PORTD |= (1<<PORTD5);
	else
	PORTD &= ~(1<<PORTD5);
	
	if(a & 16)
	PORTD |= (1<<PORTD6);
	else
	PORTD &= ~(1<<PORTD6);
	
	if(a & 32)
	PORTD |= (1<<PORTD7);
	else
	PORTD &= ~(1<<PORTD7);
	
	if(a & 64)
	PORTB |= (1<<PORTB0);
	else
	PORTB &= ~(1<<PORTB0);
	
	if(a & 128)
	PORTB |= (1<<PORTB1);
	else
	PORTB &= ~(1<<PORTB1);
}

// Funcion para enviar un caracter
void LCD_Char8BD(char caracter)
{
	
	// RS = 1 => Dato interpretado como dato
	PORTC |= (1 << PORTC1);
	LCD_Port8_Split(caracter);
	
	// EN = 1
	PORTC |= (1 << PORTC0);
	_delay_ms(2);
	
	// EN = 0
	PORTC &= ~(1 << PORTC0); // Lee la LCD
	_delay_ms(2);
	
}

// Funcion para enviar una cadena 8 bits
void LCD_String8BD(char *a)
{
	int i;
	for (i = 0; a[i] != '\0'; i++)
	LCD_Char8BD(a[i]);
}

// Funcion para correr a la izquierda 8 bits
void LCD_Shift_Right8BD(void)
{
	LCD_Comando8BD(0x01);
	LCD_Comando8BD(0x0C);
}

// Funcion para correr a la derecha 8 bits
void LCD_Shift_Left8BD(void)
{
	LCD_Comando8BD(0x01);
	LCD_Comando8BD(0x08);
}

// Funcion para el cursor 8 bits
void LCD_Cursor8BD(char c, char f)
{
	if (f == 1)
	{
		LCD_Comando8BD(0x80 + c - 1);
	}
	else if (f == 2)
	{
		LCD_Comando8BD(0xC0 + c - 1);
	}
	
}
