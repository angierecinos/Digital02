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
#include <avr/interrupt.h>
#include "I2C/I2C.h"
#include "LCD/LCD.h"
#include "UART/UART.h"

// Se determina la dirección de los slaves
#define slave1 0x30
#define slave2 0x40

#define slave1R (0x30 << 1) | 0x01 //Pongo el último bit en 1 para lectura
#define slave1W (0x30 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

#define slave2R (0x40 << 1) | 0x01 //Pongo el último bit en 1 para lectura
#define slave2W (0x40 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

#define TCS3472_READ (0x29 << 1) | 0x01 //Pongo el último bit en 1 para lectura
#define TCS3472_WRITE  (0x29 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

// Declaración de variables globales
uint8_t est_motor = 0;
uint8_t data = 0;
uint8_t estado_actual;
uint8_t estado_anterior;
uint8_t servo_angle = 0;
uint8_t direccion_stepper = 0;
uint8_t pb_flag;
uint8_t uart_map1;
uint8_t uart_map2;
uint8_t uart_map3;
uint8_t modo = 0;
uint8_t direccion;
uint8_t temp;
uint8_t bufferI2C = 0;
uint32_t lectura_adc; 
uint32_t millares;
uint32_t centenas;
uint32_t decenas;
uint32_t unidades;
uint8_t refreshLCD = 0;
uint16_t r, g, b, c;
float R_norm;
float G_norm;
float B_norm;
uint8_t bufferI2C_1 = 0;
uint16_t bufferI2C_21 = 0;
uint16_t bufferI2C_22 = 0;
uint8_t bufferI2C_3 = 0;
uint32_t lectura_S1;
uint32_t lectura_S2;
uint32_t lectura_S3;
uint8_t slave_select = 1;
uint8_t recibido;
uint8_t buffer_index = 0;
uint8_t uart_flag = 0;
char color;
char uartbuffer[20];

//************************************************************************************
// Function prototypes
void setup();
void refreshPORT(uint8_t valor);
void show_LCD();

// Función Setup
void setup()
{
	// Inicialización de LCD
	initLCD8bitsBD();
	
	// Inicialización de UART
	initUART();
	
	// Inicializacón de puertos
	DDRB |= (1 << DDB5); //Inicializo led del arduino, esto solo me va a servir para ver si si se está comunicando
	PORTB &= ~(1 << PORTB5); // Inicialmente apagado
	
	DDRC &= ~(1 << DDC2);	// Se inicia PORTC2 como input
	PORTC |= (1 << PORTC2); // Pull-up para botón
	DDRC |= (1 << DDC1);	// Se inicia PORTC1 como output
	PORTB &= ~(1 << PORTB5); // Inicialmente apagado
	
	//Puerto de salida para leds (en este caso sería el LCD)
	
	I2C_Master_Init(100000, 1); //Inicializar como MAster F_SCL 100kHz, prescaler 1
	init_TCS3472();
	
	PCICR |= (1 << PCIE1);             // Habilita interrupciones pin-change para PINC
	PCMSK1 |= (1 << PCINT10); // Habilita interrupciones para PC2
	
}

//************************************************************************************
// Main Function
int main(void)
{
	//  Se llama el setup
	setup();
	
	sei(); // Se activan las interrupciones globales

	// Se ingresa al ciclo principal	
	while (1)
	{
		
		PORTB |= (1 << PORTB5); //Solo sirve para ver si está enviando datos
		
		// Selección entre control de Adafruit y Automático
		switch (modo)
		{
			// ----------------------------------- Caso Automático --------------------------------------------
			case 0:
			
			// Si detecta el pb de cambio de modo hace la secuencia para cambio de modo
			if (pb_flag == 1)
			{
				modo = 1; // Cambia a modo manual
				PORTC &= ~(1 << PORTC1); // Apaga el led
				pb_flag = 0; // Apaga la bandera de lectura
			}
			
			// Hacemos un switch case para cambiar el slave al que le hablamos
			switch(slave_select){
			
			// ------------------------------ Slave 1 ------------------------------------
			case 1:
			
				// Hacemos la rutina de comunicación del slave
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
				
				lectura_S1 = bufferI2C_1;	// Se guarda el dato recibido en una varaible
				
				// Se compara la variable para que realice diferentes operaciones
				if (lectura_S1 >= 5  && lectura_S1 <= 43){
					est_motor = 1; // Para control de Adafruit
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave1W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('L');  // Manda el comando de inicio para el motor DC

					I2C_Master_Stop();
				}
				// Proceso de apagado
				else if (lectura_S1 >= 44){
					est_motor = 0; // Para control de Adafruit
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave1W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('S');  // Manda el comando para detener el motor DC

					I2C_Master_Stop();
				}
				// Esto es para envío de datos a Adafruit
				else if (lectura_S1 <= 5 && lectura_S1 >= 3)
				{
					enviar_valor_uart(lectura_S1, "DI:");   // RX_Distancia
					writeChar('\n');
					enviar_valor_uart(est_motor, "MD:");	// RX_MotDC
					writeChar('\n');
				}
				// Esto es para envío de datos a Adafruit
				else if (lectura_S1 <= 21 && lectura_S1 >= 19)
				{
					enviar_valor_uart(lectura_S1, "DI:");   // RX_Distancia
					writeChar('\n');
				}
				// Esto es para envío de datos a Adafruit
				else if (lectura_S1 <= 43 && lectura_S1 >= 40)
				{
					enviar_valor_uart(lectura_S1, "DI:");   // RX_Distancia
					writeChar('\n');
					enviar_valor_uart(est_motor, "MD:");	// RX_MotDC
					writeChar('\n');
				}
				// Cambia la selección al Slave 2
				slave_select = 2;
			break;
			
			// ------------------------------ Slave 2 ------------------------------------
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
				I2C_Master_Read(&bufferI2C_21, 1); //Esto es solo para recibir un solo byte de info ya que no se manda ACK
				I2C_Master_Read(&bufferI2C_22, 0);
				
				I2C_Master_Stop(); //Finalizamos
				
				PORTB &= ~(1 << PORTB5); //Apago el led para indicar que ya no hay comunicación
				
				// Guarda el dato recibido en una variable de 16 bits
				lectura_S2 = ((uint16_t)bufferI2C_21 << 8) | bufferI2C_22;
				
				// Dependiendo del valor hace una función específica
				if (lectura_S2 >= 30){
					direccion_stepper = 3;	// Para control de Adafruit
					// Envía comando al Slave para activar el stepper en una dirección
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('T');  // Comando: por ejemplo 'L' = LED ON

					I2C_Master_Stop();
				}// Dependiendo del valor hace una función específica
				
				else if (lectura_S2 >= 5 && lectura_S2 <= 29){
					direccion_stepper = 1; // Para control de Adafruit
					// Envía comando al Slave para activar el stepper en una dirección
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('A');  // Comando: por ejemplo 'L' = LED ON

					I2C_Master_Stop();
				}
				// Si mi peso está fuera del rango determinado apaga el stepper
				else if (lectura_S2 <= 5 || lectura_S2 >= 31)
				{
					direccion_stepper = 2; // Para control de adafuit
				}
				// Para control de Adafruit, se envía si tiene cierto peso
				else if (lectura_S2 == 38)
				{
					enviar_valor_uart16b(lectura_S2, "PS:");   // RX_Peso
					writeChar('\n');
					enviar_valor_uart(direccion_stepper, "ST:"); // RX_Stepper
					writeChar('\n');
				}
				// Para control de Adafruit, se envía si tiene cierto peso
				else if (lectura_S2 == 28)
				{
					enviar_valor_uart16b(lectura_S2, "PS:");   // RX_Peso
					writeChar('\n');
					enviar_valor_uart(direccion_stepper, "ST:");	// RX_Stepper
					writeChar('\n');
				}
				// Para control de Adafruit, se envía si tiene cierto peso
				else if (lectura_S2 >= 1 && lectura_S2 <= 4)
				{
					enviar_valor_uart16b(lectura_S2, "PS:");   // RX_Peso
					writeChar('\n');
					enviar_valor_uart(direccion_stepper, "ST:");	// RX_Stepper
					writeChar('\n');
				}
				
				// Se selecciona el Slave 3 (sensor I2C)
				slave_select = 3;
			break;
			
			// ------------------------------ Slave 3 ------------------------------------
			case 3:
				c = TCS3472_ReadClear();
				r = TCS3472_ReadRed();
				g = TCS3472_ReadGreen();
				b = TCS3472_ReadBlue();
		
				// Se "normaliza" el color	- Luz es Clear - elimino ese valor
// 				R_norm = r / c = (L × 0.60) / L = 0.60
// 				G_norm = g / c = (L × 0.30) / L = 0.30
// 				B_norm = b / c = (L × 0.10) / L = 0.10
		
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
					color = 1; // Para control de LCD y Adafruit
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}
					
					I2C_Master_Write('Y');  // Comando para un ángulo del servo

					I2C_Master_Stop();
					
					// Monitoreo de Adafruit, manda color y posición del servo
					enviar_valor_uart(color, "CL:");
					writeChar('\n');
					
					enviar_valor_uart(110, "SV:");
					writeChar('\n');
				}
				
				// Verde
				if(G_norm > 0.45 && R_norm < 0.35 && B_norm < 0.35)
				{
					color = 2;
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('B');  // Comando para un ángulo del servo

					I2C_Master_Stop();
					
					// Monitoreo de Adafruit, manda color 
					enviar_valor_uart(color, "CL:");   // RX_Color
					writeChar('\n');
				}
				
				// Azul
				if(B_norm > 0.35 && R_norm < 0.30 && G_norm < 0.45)
				{
					color = 3; // Para monitoreo de LCD y Adafruit
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('B');  // Comando para ángulo del servo

					I2C_Master_Stop();
					
					// Monitoreo de Adafruit, manda color y posición del servo
					enviar_valor_uart(color, "CL:");
					writeChar('\n');
					
					enviar_valor_uart(170, "SV:");
					writeChar('\n');
				}
				
				// Se guarda en una variable la lectura del color
				lectura_S3 = bufferI2C_3;
				// Selecciona el Slave 1
				slave_select = 1;
		}
		
		show_LCD(); // Función para mostrar en el LCD la información
		_delay_ms(50); // Delay funcional para que no colapse el código
			
			break;
			
		
		// ----------------------------------- Caso Manual --------------------------------------------
		case 1:
		
		// Si detecta el pb de cambio de modo hace la secuencia para cambio de modo
		if (pb_flag == 1)
		{
			modo = 0; // Cambia a modo automático
			PORTC |= (1 << PORTC1); // Enciende el led de modo
			pb_flag = 0; // Apaga la bandera del botón
		}
		
		else if (uart_flag == 1)	// Si la bandera de UART está encendida entra al if
		{
			uart_flag = 0;	// Apaga la bandera del UART

			if (uartbuffer[0] == 'M' && uartbuffer[1] == 'D' && uartbuffer[2] == ':')	// Si se recibió "MD:" entra al if
			{
				uart_map1 = ascii_to_int(&uartbuffer[3]);			// Se llama la función que transforma de ascii a int
				
				if (uart_map1 == 1)
				{
					// Hace rutina de comunicación I2C para mandar comando al slave
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave1W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('L');  // Comando de encendido de motor DC

					I2C_Master_Stop();
				}
				else if (uart_map1 == 0)
				{
					// Hace rutina de comunicación I2C para mandar comando al slave
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave1W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('S');  // Comando de apagado de motor DC

					I2C_Master_Stop();
				}
				
				enviar_valor_uart(uart_map1, "MD:");			// Se envía feedback a Adafruit por UART
				writeChar('\n');
			}
			else if (uartbuffer[0] == 'S' && uartbuffer[1] == 'V' && uartbuffer[2] == ':')	// Si se recibió "SV:" entra al if
			{
				uart_map2 = ascii_to_int(&uartbuffer[3]);			// Se llama la función que transforma de ascii a int
				
				if (uart_map2 == 170)
				{
					// Hace rutina de comunicación I2C para mandar comando al slave
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('B');  // Comando para control de servo

					I2C_Master_Stop();
				}
				else if (uart_map2 == 120)
				{
					// Hace rutina de comunicación I2C para mandar comando al slave
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('Y');  // Comando para control de servo

					I2C_Master_Stop();
				}
				
				enviar_valor_uart(uart_map2, "SV:");			// Se envía feedback a Adafruit por UART
				writeChar('\n');
			}
			else if (uartbuffer[0] == 'S' && uartbuffer[1] == 'T' && uartbuffer[2] == ':')	// Si se recibió "ST:" entra al if
			{
				uart_map3 = ascii_to_int(&uartbuffer[3]);			// Se llama la función que transforma de ascii a int
				
				if (uart_map3 == 1)
				{
					// Hace rutina de comunicación I2C para mandar comando al slave
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('A');  // Comando para control de stepper

					I2C_Master_Stop();
				}
				else if (uart_map3 == 2)
				{
					// Hace rutina de comunicación I2C para mandar comando al slave
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('R');  // Comando para control de stepper


					I2C_Master_Stop();
				}
				else if (uart_map3 == 3)
				{
					// Hace rutina de comunicación I2C para mandar comando al slave
					if(!I2C_Master_Start()) return;

					if(!I2C_Master_Write(slave2W)){   // SLA + W
						I2C_Master_Stop();
						return;
					}

					I2C_Master_Write('T');  // Comando para control de stepper

					I2C_Master_Stop();
				}
				
				enviar_valor_uart(uart_map3, "ST:");			// Se envía feedback a Adafruit por UART
				writeChar('\n');
			}
		break;
		
		}
		}
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
	LCD_Write_String8bitBD("S1:   S2:   S3:");
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
	millares = lectura_S2 / 1000;
	centenas = (lectura_S2 % 1000) / 100; 				// Unicamente se queda la parte entera
	decenas = (lectura_S2 % 100) / 10;			// Se utiliza residuo para obtener decenas
	unidades = lectura_S2 % 10;				// Se utiliza residuo para obtener unidades
	
	LCD_Write_Char8bitBD(millares + '0');
	LCD_Write_Char8bitBD(centenas + '0');
	LCD_Write_Char8bitBD(decenas + '0');
	LCD_Write_Char8bitBD(unidades + '0');
	LCD_Write_Char8bitBD('g');
	
	LCD_Set_Cursor8bitBD(13,2);
	
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
ISR(USART_RX_vect)
{
	recibido = UDR0;	// Leer el carácter recibido desde el registro de UART

	if (recibido == '\n') // Si la cadena termina en el caracter de "enter" entra al if
	{
		uartbuffer[buffer_index] = '\0';	// Termina el string
		uart_flag = 1;					// Enciende la bandera de UART
		buffer_index = 0;				// Reinicia el índice del buffer
	}
	else
	{
		if (buffer_index < sizeof(uartbuffer) - 1) // Mientras que el índice del caracter recibido sea menor que el tamaño de la lista (Buffer) entra al if
		{
			uartbuffer[buffer_index++] = recibido;	// Guarda en la lista el caracter recibido y suma uno al índice de la lista
		}
	}
}

//-----------------Interrupción de Pin-Change-----------------
ISR(PCINT1_vect)
{
	estado_actual = PINC & (1 << PORTC2);  // Leer el estado actual de los botones
	
	if (((estado_anterior & (1 << PORTC2)) != 0) && ((estado_actual & (1 << PORTC2)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 1;																	// Si PC4 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
	}
	estado_anterior = estado_actual;  // Guardar el estado actual a estado anterior
}