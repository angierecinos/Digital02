/*
 * I2C.c
 *
 * Created: 5/02/2026 19:28:45
 *  Author: Usuario
 */ 

#include <avr/io.h>
#include <stdint.h>
# define F_CPU 16000000
# include <util/delay.h>
#include "I2C.h"

// Datasheet indica bit 7 como 1
#define COMMAND_BIT    0x80

// Registros acrorde a datasheet
#define REG_ENABLE     0x00
#define REG_ATIME      0x01
#define REG_CDATAL     0x14
#define REG_RDATAL     0x16
#define REG_GDATAL     0x18
#define REG_BDATAL     0x1A

// Definición para lectura y escritura del sensor
#define TCS3472_READ (0x29 << 1) | 0x01 //Pongo el último bit en 1 para lectura
#define TCS3472_WRITE  (0x29 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

//Función para inicializar I2C Maestro
void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler){
	DDRC &= ~((1 << DDC4) | (1 << DDC5)); //Pines I2C como entradas y SDA y SCL
	//Se debe de seleccionar el valor de los bits para el prescaler del registro TWSR
	
	switch(Prescaler){
		case 1:
		TWSR &= ~((1 << TWPS1) | (1 << TWPS0));
		break;
		case 4:
		TWSR &= ~(1 << TWPS1);
		TWSR |= (1 << TWPS0);
		break;
		case 16:
		TWSR &= ~(1 << TWPS0);
		TWSR |= (1 << TWPS1);
		break;
		case 64:
		TWSR |= (1 << TWPS1) | (1 << TWPS0);
		break;
		default:
		TWSR &= ~((1 << TWPS1) | (1 << TWPS0));
		Prescaler = 1;
		break;
	}
	TWBR = ((F_CPU/SCL_Clock)-16)/(2*Prescaler); //Calcular la velocidad (ver presentación)
	TWCR |= (1 << TWEN);	// Activar la interfase (TWI - Two Wire Interfase) = I2C
}

//Función de inicio de la comunicación I2C
uint8_t I2C_Master_Start(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); //Master, Reiniciar bandera de Int, Condicion de Start
	while (!(TWCR & (1 << TWINT))); //Espera hasta que se encienda la bandera
	
	return ((TWSR & 0xF8) == 0x08); //Nos quedamos únicamente con los bits de estado
}

//Función de reinicio de la comunicación I2C
uint8_t I2C_Master_RepeatedStart(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); //Master, Reiniciar bandera de Int, COndicion de start
	while (!(TWCR & (1 << TWINT))); // Esperar a que se encienda la bandera
	return ((TWSR & 0xF8) == 0x10); //

}

//Función de parada de la comunicación I2C
void I2C_Master_Stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // Inicia el envío secuencia parada STOP
	while (TWCR & (1 << TWSTO)); // Esperar a que el bit se limpie
}

//Función de transmisión de datos del maestro al esclavo
//esta función devolverá un 0 si el esclavo a revibido un dato
uint8_t I2C_Master_Write(uint8_t dato)
{
	uint8_t estado;
	TWDR = dato; //Cargo el dato
	TWCR = (1 << TWINT) | (1 << TWEN); // Inicia la secuencia de envío
	//Habilitando la interfaz y limpiando la bandera de interrupción
	
	while(!(TWCR & (1 << TWINT))); //Nos quedamos unicamente con los bits de estado RW1 Status
	estado = TWSR & 0xF8;	// Nos quedamos unicamente con los bits de estado TWI Status
	//Verificar si se transmitio una SLA + W cons ACK, o como un dato
	if (estado == 0x18 || estado == 0x28){
		return 1;
		}else{
		return estado;
	}
}

//Función de recepción de datos enviados por el escalvo al maestro
//esta función es para leer los datos que están en el esclavo

//sirve para recopilar los datos del esclavo en dado caso sean 3 bytes de información
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack)
{
	uint8_t estado;
	if(ack){
		//ACK se envía para decirle al esclavo "quiero más datos"
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); //Habilitar INterfase I2C con envío de ACK
		} else {
		//NACK: se indica que es el último byte
		TWCR = (1 << TWINT) | (1 << TWEN); //Habilitar Interfase I2C sin envío de ACD (NACK)
	}
	
	while(!(TWCR & (1 << TWINT))); //Esperar la bandera de interrupción TWINT
	
	estado = TWSR & 0xF8;
	//Verificar si se recibio Dato con ACK o sin ACK
	if (ack && estado !=0x50) return 0; // Data recibida, ACK
	if (!ack && estado !=0x58) return 0; //Data recibida, pero sin ACK
	
	*buffer = TWDR; //Obtenemos el resulktado en el registro de datos
	return 1;
}

//Función para inicializar I2C Esclavo
void I2C_Slave_Init(uint8_t addres)
{
	DDRC &= ~((1 << DDC4) | (1 << DDC5)); //Pines de I2C como entradas
	
	TWAR = addres << 1; // Se asigna la dirección que tendrá
	//TWAR = (address << 1 | 0x01); //Se asigna la dirección que tendra y habilita llamada general
	
	//Se habilita la interfaz, ACK automatico, se habilita la ISR
	TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
}


// ---------------------------- Funciones para sensor ---------------------------------


// Función para escribir un registro (El sensor tiene varios registros propios)
void TCS3472_WriteReg(uint8_t reg, uint8_t value)
{
	I2C_Master_Start();
	I2C_Master_Write(TCS3472_WRITE);		// Establezco comunicación con sensor
	// Por ej.		0x80 | ATIME(0x01) = 0x81 -> Comando enviado a la integración de tiempo
	I2C_Master_Write(COMMAND_BIT | reg);	// Indico a que registro le quiero escribir siempre siendo comando
	I2C_Master_Write(value);				// Valor que le estoy enviando, por ejemplo a 0x00 le envío 0x01 -> Enable ON
	I2C_Master_Stop();
}

// Leer 16 bits (El sensor da las lecturas en 16 bits)
// Le indicaré de qué registro quiero leer (clear, read, green o blue)
uint16_t TCS3472_Read16(uint8_t reg)
{
	
	// Indico byte alto y byte bajo
	uint8_t low, high;

	I2C_Master_Start();
	
	// Comunicación con el sensor
	I2C_Master_Write(TCS3472_WRITE);
	// Decido desde donde quiero leer 
	I2C_Master_Write(COMMAND_BIT | reg);
	// Cambio a modo lectura
	I2C_Master_RepeatedStart();
	// Se indica que se desea lectura del sensor
	I2C_Master_Write(TCS3472_READ);
	
	// Indicar en que posicion de memoria quiero guardar dato leído
	I2C_Master_Read(&low, 1);   // ACK -> aun quiero más datos
	I2C_Master_Read(&high, 0);  // NACK -> ya no quiero más datos

	I2C_Master_Stop();

	return (high << 8) | low;	// Uno la lectura del Byte high con la del Byte low
}

// Inicialización del sensor
void init_TCS3472(void)
{
	// Enciende el sensor
	TCS3472_WriteReg(REG_ENABLE, 0x01);
	_delay_ms(3);

	// Activar ADC (0011 -> ADC y ON)
	TCS3472_WriteReg(REG_ENABLE, 0x03);

	// Tiempo de integración (ejemplo ~100ms)
	TCS3472_WriteReg(REG_ATIME, 0xD6);

	_delay_ms(100);
}

// Lecturas para los registros Clear, Red, Blue y Green
uint16_t TCS3472_ReadClear(void)
{
	return TCS3472_Read16(REG_CDATAL);
}

uint16_t TCS3472_ReadRed(void)
{
	return TCS3472_Read16(REG_RDATAL);
}

uint16_t TCS3472_ReadGreen(void)
{
	return TCS3472_Read16(REG_GDATAL);
}

uint16_t TCS3472_ReadBlue(void)
{
	return TCS3472_Read16(REG_BDATAL);
}
