/*
 * LCD.h
 *
 * Created: 1/22/2026 11:55:56 AM
 *  Author: edvin
 */ 

#ifndef LCD_H_
#define LCD_H_
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

#define E (1<<PORTB3)
// --------------------------- Funciones para modo 4bits --------------------------------------------
// Función para inicializar LCD en modo 4 bits
void initLCD4bits(void);

// Función para colocar en el puerto un valor
void LCD_Port4bit(char a);

// Función para enviar un comando
void LCD_CMD4bit(char a);

// Función para enviar un caracter
void LCD_Write_Char4bit(char c);

// Función para enviar una cadena
void LCD_Write_String4bit(char *a);

// Desplazamiento hacia la derecha
void LCD_Shift_Right4bit(void);

// Desplazamiento hacia la izquierda
void LCD_Shift_Left4bit(void);

// Establecer el cursor
void LCD_Set_Cursor4bit(char c, char f);

// --------------------------- Funciones para modo 8bits --------------------------------------------
// Función para inicializar LCD en modo 8 bits
void initLCD8bits(void);

// Función para colocar en el puerto un valor
void LCD_Port8bit(char a);

// Función para enviar un comando
void LCD_CMD8bit(char a);

// Función para enviar un caracter
void LCD_Write_Char8bit(char c);

// Función para enviar una cadena
void LCD_Write_String8bit(char *a);

// Desplazamiento hacia la derecha
void LCD_Shift_Right8bit(void);

// Desplazamiento hacia la izquierda
void LCD_Shift_Left8bit(void);

// Establecer el cursor
void LCD_Set_Cursor8bit(char c, char f);


// --------------------------- Funciones para modo 8bits Combinando PORTB y PORTD --------------------------------------------
// Función para inicializar LCD en modo 8 bits
void initLCD8bitsBD(void);

// Función para colocar en el puerto un valor
void LCD_Port8bitBD(char a);

// Función para enviar un comando
void LCD_CMD8bitBD(char a);

// Función para enviar un caracter
void LCD_Write_Char8bitBD(char c);

// Función para enviar una cadena
void LCD_Write_String8bitBD(char *a);

// Desplazamiento hacia la derecha
void LCD_Shift_Right8bitBD(void);

// Desplazamiento hacia la izquierda
void LCD_Shift_Left8bitBD(void);

// Establecer el cursor
void LCD_Set_Cursor8bitBD(char c, char f);

#endif /* LCD_H_ */
