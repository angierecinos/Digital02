/*
 * HX711.h
 *
 * Created: 2/13/2026 4:45:29 PM
 *  Author: edvin
 */ 


#ifndef HX711_H_
#define HX711_H_

#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <stdint.h>

void initHX711(void);

void HX711_WaitReady(void);

int32_t HX711_Read(void);

uint8_t HX711_IsReady(void);

int32_t HX711_Tare(uint8_t muestras);

int32_t HX711_ReadRaw(void);

#endif /* HX711_H_ */