/*
 * LCD.c
 *
 * Created: 1/22/2026 11:55:45 AM
 *  Author: edvin
 */ 

#include "LCD.h"

// --------------------------- Funciones para modo 4bits --------------------------------------------
// Función para inicializar LCD en modo 4 bits
void initLCD4bits(void)
{
	DDRC |= (1<<DDC0)|(1<<DDC1)|(1<<DDC2)|(1<<DDC3)|(1<<DDC4)|(1<<DDC5);
	PORTC = 0;
	
	LCD_Port4bit(0x00);
	_delay_ms(20);
	LCD_CMD4bit(0x03);
	_delay_ms(5);
	LCD_CMD4bit(0x03);
	_delay_ms(11);
	LCD_CMD4bit(0x03);
	
	LCD_CMD4bit(0x02);
	
	//////////////////////////////////////////////////////////////////////
	// Function Set
	LCD_CMD4bit(0x02);
	LCD_CMD4bit(0x08);
	
	// Display ON/OFF
	LCD_CMD4bit(0x00);
	LCD_CMD4bit(0x0C);
	
	// Entry Mode
	LCD_CMD4bit(0x00);
	LCD_CMD4bit(0x06);
	
	// Clear Display
	LCD_CMD4bit(0x00);
	LCD_CMD4bit(0x01);
}

// Función para enviar un comando
void LCD_CMD4bit(char a)
{
	// RS = 0; // => RS = 0 // Dato en el puerto lo va a interpretar como 
	PORTC &= ~(1<<PORTC0);
	LCD_Port4bit(a);
	// EN = 1; // => E = 1
	PORTC |= (1<<PORTC1);
	_delay_ms(4);
	// EN = 0; // => E = 0
	PORTC &= ~(1<<PORTC1);
}

// Función para colocar en el puerto un valor
void LCD_Port4bit(char a)
{
	if (a & 1)
	// D4 = 1;
	PORTC |= (1<<PORTC2);
	else
	// D4 = 0;
	PORTC &= ~(1<<PORTC2);
	
	if (a & 2)
	// D5 = 1;
	PORTC |= (1<<PORTC3);
	else
	// D5 = 0;
	PORTC &= ~(1<<PORTC3);
	
	if (a & 4)
	// D6 = 1;
	PORTC |= (1<<PORTC4);
	else
	// D6 = 0;
	PORTC &= ~(1<<PORTC4);
	
	if (a & 8)
	// D7 = 1;
	PORTC |= (1<<PORTC5);
	else
	// D7 = 0;
	PORTC &= ~(1<<PORTC5);
}

// Función para enviar un caracter
void LCD_Write_Char4bit(char c)
{
	char Cbajo, Calto;
	Cbajo = c & 0x0F;
	Calto = (c & 0xF0)>>4; // 11110000 => >>4 => 00001111
	
	// RS = 1; // => RS = 1 // Dato en el puerto la va a interpretar como 
	PORTC |= (1<<PORTC0);
	LCD_Port4bit(Calto);
	// EN = 1; // => E = 1
	PORTC |= (1<<PORTC1);
	_delay_ms(4);
	// EN = 0; // E => 0
	PORTC &= ~(1<<PORTC1);
	LCD_Port4bit(Cbajo);
	// EN = 1; // => E = 1
	PORTC |= (1<<PORTC1); 
	_delay_ms(4);
	// EN = 0; // E => 0
	PORTC &= ~(1<<PORTC1);
}

// Función para enviar una cadena
void LCD_Write_String4bit(char *a)
{
	int i;
	for (i = 0; a[i] != '\0'; i++)
	LCD_Write_Char4bit(a[i]);
}

// Desplazamiento hacia la derecha
void LCD_Shift_Right4bit(void)
{
	LCD_CMD4bit(0x01);
	LCD_CMD4bit(0x0C);
}

// Desplazamiento hacia la izquierda
void LCD_Shift_Left4bit(void)
{
	LCD_CMD4bit(0x01);
	LCD_CMD4bit(0x08);
}

// Establecer el cursor
void LCD_Set_Cursor4bit(char c, char f)
{
	char temp, talto, tbajo;
	if (f == 1)
	{
		temp = 0x08 + c - 1;
		talto = (temp & 0xF0) >> 4;
		tbajo = (temp & 0x0F);
		LCD_CMD4bit(talto);
		LCD_CMD4bit(tbajo);
	}
	else if (f == 2)
	{
		temp = 0xC0 + c - 1;
		talto = (temp & 0xF0) >> 4;
		tbajo = temp & 0xF0;
		LCD_CMD4bit(talto);
		LCD_CMD4bit(tbajo);
	}
}

// --------------------------- Funciones para modo 8bits --------------------------------------------
// Función para inicializar LCD en modo 8 bits
void initLCD8bits(void)
{
	DDRD = 0xFF;
	PORTD = 0x00;

	DDRB |= (1<<DDB0)|(1<<DDB1)|(1<<DDB2);
	PORTB &= ~((1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2));

	_delay_ms(20);

	LCD_CMD8bit(0x30);
	_delay_ms(5);
	LCD_CMD8bit(0x30);
	_delay_ms(1);
	LCD_CMD8bit(0x30);

	//////////////////////////////////////////////////////////////////////
	// Function Set
	LCD_CMD8bit(0x38);

	// Display ON/OFF
	LCD_CMD8bit(0x08);
	LCD_CMD8bit(0x0C);

	// Entry Mode
	LCD_CMD8bit(0x06);

	// Clear Display
	LCD_CMD8bit(0x01);
	_delay_ms(2);
}

// Función para enviar un comando
void LCD_CMD8bit(char a)
{
	// RS = 0; // => RS = 0
	PORTB &= ~(1<<PORTB0);
	PORTB &= ~(1<<PORTB1);
	LCD_Port8bit(a);
	// EN = 1; // => E = 1
	PORTB |= (1<<PORTB2);
	_delay_us(1);
	// EN = 0; // => E = 0
	PORTB &= ~(1<<PORTB2);
	_delay_us(50);
}

// Función para colocar en el puerto un valor
void LCD_Port8bit(char a)
{
	PORTD = a;   // D0-D7 = a
}

// Función para enviar un caracter
void LCD_Write_Char8bit(char c)
{
	// RS = 1; // => RS = 1
	PORTB |= (1<<PORTB0);
	// RW = 0; // => RW = 0
	PORTB &= ~(1<<PORTB1);
	LCD_Port8bit(c);
	// EN = 1; // => E = 1
	PORTB |= (1<<PORTB2);
	_delay_us(1);
	// EN = 0; // => E = 0
	PORTB &= ~(1<<PORTB2);
	_delay_us(50);
}

// Función para enviar una cadena
void LCD_Write_String8bit(char *a)
{
	int i;
	for (i = 0; a[i] != '\0'; i++)
	LCD_Write_Char8bit(a[i]);
}

// Desplazamiento hacia la derecha
void LCD_Shift_Right8bit(void)
{
	LCD_CMD8bit(0x01);
	LCD_CMD8bit(0x0C);
}

// Desplazamiento hacia la izquierda
void LCD_Shift_Left8bit(void)
{
	LCD_CMD8bit(0x01);
	LCD_CMD8bit(0x08);
}

// Establecer el cursor
void LCD_Set_Cursor8bit(char c, char f)
{
	char temp;
	if (f == 1)
	{
		temp = 0x80 + c - 1;
		LCD_CMD8bit(temp);
	}
	else if (f == 2)
	{
		temp = 0xC0 + c - 1;
		LCD_CMD8bit(temp);
	}
}

// --------------------------- Funciones para modo 8bits Combinando PORTB y PORTD --------------------------------------------
// Función para inicializar LCD en modo 8 bits
void initLCD8bitsBD(void)
{
	DDRD |= 0xFC;		// PD2-PD7 como salida (D0-D5)
	PORTD &= ~0xFC;

	DDRB |= (1<<DDB0)|(1<<DDB1)|(1<<DDB2)|(1<<DDB3)|(1<<DDB4);	// PB0-PB4 como salida (D6,D7,RS,RW,E)
	PORTB &= ~((1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2)|(1<<PORTB3)|(1<<PORTB4));

	_delay_ms(20);

	LCD_CMD8bitBD(0x30);
	_delay_ms(5);
	LCD_CMD8bitBD(0x30);
	_delay_us(200);
	LCD_CMD8bitBD(0x30);
	_delay_us(200);

	//////////////////////////////////////////////////////////////////////
	// Function Set
	LCD_CMD8bitBD(0x38);

	// Display ON/OFF
	LCD_CMD8bitBD(0x08);
	LCD_CMD8bitBD(0x0C);

	// Entry Mode
	LCD_CMD8bitBD(0x06);

	// Clear Display
	LCD_CMD8bitBD(0x01);
	_delay_ms(2);
}

// Función para enviar un comando
void LCD_CMD8bitBD(char a)
{
	// RS = 0; // => RS = 0
	PORTB &= ~(1<<PORTB2);
	PORTB &= ~(1<<PORTB3);
	LCD_Port8bitBD(a);
	_delay_us(5);                 // <-- setup time (ANTES DE E)
	// EN = 1; // => E = 1
	PORTB |= (1<<PORTB4);
	_delay_us(5);                 // <-- E high width
	// EN = 0; // => E = 0
	PORTB &= ~(1<<PORTB4);
	_delay_us(2);                 // <-- hold time
	_delay_us(50);
}

// Función para colocar en el puerto un valor
void LCD_Port8bitBD(char a)
{
	PORTD &= 0x03;
	PORTD |= ((a & 0x3F) << 2);

	PORTB &= ~((1<<PORTB0)|(1<<PORTB1));
	PORTB |= ((a >> 6) & 0x03);
}

// Función para enviar un caracter
void LCD_Write_Char8bitBD(char c)
{
	// RS = 1; // => RS = 1
	PORTB |= (1<<PORTB2);
	// RW = 0; // => RW = 0
	PORTB &= ~(1<<PORTB3);
	LCD_Port8bitBD(c);
	_delay_us(5);                 // <-- setup time
	// EN = 1; // => E = 1
	PORTB |= (1<<PORTB4);
	_delay_us(5);                 // <-- E high width
	// EN = 0; // => E = 0
	PORTB &= ~(1<<PORTB4);
	_delay_us(2);                 // <-- hold time
	_delay_us(50);
}

// Función para enviar una cadena
void LCD_Write_String8bitBD(char *a)
{
	int i;
	for (i = 0; a[i] != '\0'; i++)
	LCD_Write_Char8bitBD(a[i]);
}

// Desplazamiento hacia la derecha
void LCD_Shift_Right8bitBD(void)
{
	LCD_CMD8bitBD(0x01);
	LCD_CMD8bitBD(0x0C);
}

// Desplazamiento hacia la izquierda
void LCD_Shift_Left8bitBD(void)
{
	LCD_CMD8bitBD(0x01);
	LCD_CMD8bitBD(0x08);
}

// Establecer el cursor
void LCD_Set_Cursor8bitBD(char c, char f)
{
	char temp;
	if (f == 1)
	{
		temp = 0x80 + c - 1;
		LCD_CMD8bitBD(temp);
	}
	else if (f == 2)
	{
		temp = 0xC0 + c - 1;
		LCD_CMD8bitBD(temp);
	}
}