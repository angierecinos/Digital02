/*
 * Proyecto01.c
 *
 * Created: 5/02/2026 19:26:24
 * Author : Angie
 */ 


//************************************************************************************
// Encabezado (librerías)
#include <avr/io.h>
#include <stdint.h>
# define F_CPU 16000000
#include <util/delay.h>
#include "I2C/I2C.h"
#include "LCD/LCD.h"

#define slave1 0x30
#define slave2 0x40

#define slave1R (0x30 << 1) | 0x01 //Pongo el último bit en 1 para lectura
#define slave1W (0x30 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

#define slave2R (0x40 << 1) | 0x01 //Pongo el último bit en 1 para lectura
#define slave2W (0x40 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

#define TCS3472_READ (0x29 << 1) | 0x01 //Pongo el último bit en 1 para lectura
#define TCS3472_WRITE  (0x29 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir


uint8_t direccion;
uint8_t temp;
uint8_t bufferI2C = 0;
uint32_t lectura_adc; 
uint32_t centenas;
uint32_t decenas;
uint32_t unidades;
uint8_t refreshLCD = 0;
uint16_t r, g, b, c;
float R_norm;
float G_norm;
float B_norm;
uint8_t bufferI2C_1 = 0;
uint8_t bufferI2C_2 = 0;
uint8_t bufferI2C_3 = 0;
uint32_t lectura_S1;
uint32_t lectura_S2;
uint32_t lectura_S3;
uint8_t slave_select = 1;
char color;

//************************************************************************************
// Function prototypes
void setup();
void refreshPORT(uint8_t valor);
void show_LCD();

void setup()
{
	//DDRB |= (1 << DDB5);
	initLCD8bitsBD();
	
}

//************************************************************************************
// Main Function
int main(void)
{
	setup();
	DDRB |= (1 << DDB5); //Inicializo led del arduino, esto solo me va a servir para ver si si se está comunicando
	PORTB &= ~(1 << PORTB5);
	
	DDRC |= (1 << DDC2);
	
	//Puerto de salida para leds (en este caso sería el LCD)
	
	I2C_Master_Init(100000, 1); //Inicializar como MAster F_SCL 100kHz, prescaler 1
	init_TCS3472();
	
	while (1)
	{
		PORTB |= (1 << PORTB5); //Solo sirve para ver si está enviando datos
		
		switch(slave_select){
			case 1:
			
				if(!I2C_Master_Start()) return; //Si no recibe dato solo se regresa
				
				if(!I2C_Master_Write(slave1W)){ //Mandar la dirección de escritura, si no recibo el ACK manda el stop
					I2C_Master_Stop();
					return;
				}
				I2C_Master_Write('R'); //Comando para leer "te voy a leer"
				
				//Ahora configuro para leer
				if (!I2C_Master_RepeatedStart()){
					I2C_Master_Stop();// SI no recibo nada termino la comunicación con return
					return;
				}
				if(!I2C_Master_Write(slave1R)){ //Si si se da la comunicación entonces leer
					I2C_Master_Stop();
					return;
				}
				//Esto se ejecuta en el caso que si se da la comunicación
				I2C_Master_Read(&bufferI2C_1, 0); //Esto es solo para recibir un solo byte de info ya que no se manda ACK
				
				I2C_Master_Stop(); //Finalizamos
				
				PORTB &= ~(1 << PORTB5); //Apago el led para indicar que ya no hay comunicación
				
				lectura_S1 = bufferI2C_1;
				
				if (lectura_S1 <= 10){
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave1W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('L');  // Comando: por ejemplo 'L' = LED ON

					I2C_Master_Stop();
				}
				else if (lectura_S1 > 10){
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave1W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('S');  // Comando: por ejemplo 'L' = LED ON

					I2C_Master_Stop();
				}
				slave_select = 2;
			break;
			
			case 2:
				if(!I2C_Master_Start()) return; //Si no recibe dato solo se regresa
				
				if(!I2C_Master_Write(slave2W)){ //Mandar la dirección de escritura, si no recibo el ACK manda el stop
					I2C_Master_Stop();
					return;
				}
				I2C_Master_Write('R'); //Comando para leer "te voy a leer"
				
				//Ahora configuro para leer
				if (!I2C_Master_RepeatedStart()){
					I2C_Master_Stop();// SI no recibo nada termino la comunicación con return
					return;
				}
				if(!I2C_Master_Write(slave2R)){ //Si si se da la comunicación entonces leer
					I2C_Master_Stop();
					return;
				}
				//Esto se ejecuta en el caso que si se da la comunicación
				I2C_Master_Read(&bufferI2C_2, 0); //Esto es solo para recibir un solo byte de info ya que no se manda ACK
				
				I2C_Master_Stop(); //Finalizamos
				
				PORTB &= ~(1 << PORTB5); //Apago el led para indicar que ya no hay comunicación
				
				lectura_S2 = bufferI2C_2;
				slave_select = 3;
			break;
			
			case 3:
				c = TCS3472_ReadClear();
				r = TCS3472_ReadRed();
				g = TCS3472_ReadGreen();
				b = TCS3472_ReadBlue();
		
				// Se "normaliza" el color	- Luz es Clear - elimino ese valor
				/*R_norm = r / c = (L × 0.60) / L = 0.60
				G_norm = g / c = (L × 0.30) / L = 0.30
				B_norm = b / c = (L × 0.10) / L = 0.10*/
		
				// Si cambia la luz, cambian todos	
				if(c > 0)
				{
					R_norm = (float)r / (float)c;
					G_norm = (float)g / (float)c;
					B_norm = (float)b / (float)c;
				}
				else
				{
					R_norm = 0;
					G_norm = 0;
					B_norm = 0;
				}

				// Rojo
				if(R_norm > 0.45 && G_norm < 0.30 && B_norm < 0.30)
				{
					PORTC |= (1 << PORTC2);
					color = 1;
				}
				
				// Verde
				if(G_norm > 0.45 && R_norm < 0.35 && B_norm < 0.35)
				{
					PORTC |= (1 << PORTC2);
					color = 2;
				}
				
				// Azul
				if(B_norm > 0.45 && R_norm < 0.30 && G_norm < 0.35)
				{
					PORTC &= ~(1 << PORTC2);
					color = 3;
				}
				
				lectura_S3 = bufferI2C_3;
				slave_select = 1;
		}
		
		
		
		show_LCD();
		_delay_ms(50);
	}
}

//************************************************************************************
// NON-INterrupt subroutines
void refreshPORT(uint8_t valor)
{
	if (valor & 0b10000000)
	{
		PORTB |= (1 << PORTB1);
		}else{
		PORTB &= ~(1 << PORTB1);
	}
	if (valor &	0b01000000){
		PORTB |= (1 << PORTB0);
	}
	else{
		PORTB &= ~(1 << PORTB0);
	}
	if (valor &	0b00100000){
		PORTD |= (1 << PORTD7);
	}
	else{
		PORTD &= ~(1 << PORTD7);
	}
	if (valor &	0b00010000){
		PORTD |= (1 << PORTD6);
	}
	else{
		PORTD &= ~(1 << PORTD6);
	}
	if (valor &	0b00001000){
		PORTD |= (1 << PORTD5);
	}
	else{
		PORTD &= ~(1 << PORTD5);
	}
	if (valor &	0b00000100){
		PORTD |= (1 << PORTD4);
	}
	else{
		PORTD &= ~(1 << PORTD4);
	}
	if (valor &	0b00000010){
		PORTD |= (1 << PORTD3);
	}
	else{
		PORTD &= ~(1 << PORTD3);
	}
	if (valor &	0b00000001){
		PORTD |= (1 << PORTD2);
	}
	else{
		PORTD &= ~(1 << PORTD2);
	}
}

void show_LCD()
{
	
	LCD_Set_Cursor8bitBD(1,1);
	LCD_Write_String8bitBD("S1:   S2:  S3:");
	LCD_Set_Cursor8bitBD(1,2);
	
	// Convertir a ASCII y enviar dígito por dígito
	centenas = lectura_S1/100;					// Unicamente se queda la parte entera
	decenas = (lectura_S1 % 100) / 10;			// Se utiliza residuo para obtener decenas
	unidades = lectura_S1 % 10;				// Se utiliza residuo para obtener unidades
	
	LCD_Write_Char8bitBD(centenas + '0');
	LCD_Write_Char8bitBD(decenas + '0');
	LCD_Write_Char8bitBD(unidades + '0');
	
	LCD_Write_String8bitBD("cm ");
	
	LCD_Set_Cursor8bitBD(7,2);
	
	// Convertir a ASCII y enviar dígito por dígito
	centenas = lectura_S2/100;					// Unicamente se queda la parte entera
	decenas = (lectura_S2 % 100) / 10;			// Se utiliza residuo para obtener decenas
	unidades = lectura_S2 % 10;				// Se utiliza residuo para obtener unidades
	
	LCD_Write_Char8bitBD(centenas + '0');
	LCD_Write_Char8bitBD(decenas + '0');
	LCD_Write_Char8bitBD(unidades + '0');
	
	LCD_Set_Cursor8bitBD(12,2);
	
	// Convertir a ASCII y enviar dígito por dígito
	if (color == 1)
	{
		LCD_Write_String8bitBD("red");
		
	}
	else if (color == 2)
	{
		LCD_Write_String8bitBD("gre");
		
	}
	else if (color == 3)
	{
		LCD_Write_String8bitBD("blu");
		
	}
}
//************************************************************************************
// Interrupt subroutines
