/*
 * I2C.h
 *
 * Created: 5/02/2026 19:29:01
 *  Author: Usuario
 */ 


#ifndef I2C_H_
#define I2C_H_

#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <stdint.h>

//Función para inicializar el maestro
void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler);

//Función de inicio de la comunicación I2C
uint8_t I2C_Master_Start(void);
uint8_t I2C_Master_RepeatedStart(void);

//Función para parar la comunicación I2C
void I2C_Master_Stop(void);

//Función de transmisión de datos maestro=>escalvo
//Devuelve 0 si el esclavo recibió el dato
uint8_t I2C_Master_Write(uint8_t dato);

//Función de recepción de datos
//Lee los datos que están en el esclavo
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack);

//Función para inicializar I2C Esclavo
void I2C_Slave_Init(uint8_t addres);

// --- Funciones del TCS3472 ---

void init_TCS3472(void);

void TCS3472_WriteReg(uint8_t reg, uint8_t value);

uint16_t TCS3472_Read16(uint8_t reg);

uint16_t TCS3472_ReadClear(void);

uint16_t TCS3472_ReadRed(void);

uint16_t TCS3472_ReadGreen(void);

uint16_t TCS3472_ReadBlue(void);



#endif /* I2C_H_ */