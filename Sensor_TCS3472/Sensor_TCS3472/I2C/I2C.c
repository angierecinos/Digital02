/*
 * I2C.c
 *
 * Created: 5/02/2026 19:41:42
 *  Author: Usuario
 */ 

#include <avr/io.h>
#include <stdint.h>
#include "I2C.h"



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
	//while (!(TWCR & (1 << TWSTO))); // Esperar a que el bit se limpie
	while (TWCR & (1 << TWSTO)); 
}

//Función de transmisión de datos del maestro al esclavo
//esta función devolverá un 0 si el esclavo a recibido un dato
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
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN); //Habilitar INterfase I2C con envío de ACK
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



