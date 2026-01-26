/*
 * LCD.h
 *
 * Created: 22/01/2026 
 *  Author: Angie
 */ 


#ifndef LCD_H_
#define LCD_H_
#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

#define E (1 << PORTC0)

// Funcion para inicializar LCD en 4 bits
void initLCD_4bits(void);
void initLCD_8bits(void);
void initLCD_8bitsBD(void);

// Funcion para poner en el puerto un valor
// En 8 bits solo PORTD = a
void LCD_Port4(char valor);
void LCD_Port8_Split(char a);

// Funcion para enviar un comando
void LCD_Comando(char a);
void LCD_Comando8(char a);
void LCD_Comando8BD(char a);

// Funcion para enviar un caracter
void LCD_Char4(char caracter);
void LCD_Char8(char caracter);
void LCD_Char8BD(char caracter);

// Funcion para enviar un string
void LCD_String4(char *a);
void LCD_String8(char *a);
void LCD_String8BD(char *a);

// Desplazamiento a la derecha
void LCD_Shift_Right4(void);
void LCD_Shift_Right8(void);
void LCD_Shift_Right8BD(void);

// Desplazamiento a la derecha
void LCD_Shift_Left4(void);
void LCD_Shift_Left8(void);
void LCD_Shift_Left8BD(void);

// Establecer el cursor
void LCD_Cursor8(char c, char f);
void LCD_Cursor4(char c, char f);
void LCD_Cursor8BD(char c, char f);

#endif /* LCD_H_ */