/*
 * Proyecto01_SLAVE2.c
 *
 * Created: 5/02/2026 19:40:26
 * Author : Angie
 */ 


//************************************************************************************
//================================== ESCLAVO =====================================

// Encabezado (librerías)
# include <avr/io.h>
# include <stdint.h>
# define F_CPU 16000000
# include <util/delay.h>
# include <avr/interrupt.h>
# include "I2C/I2C.h" 
# include "ADC/ADC.h"

//Se define la dirección del esclavo, en este caso como es mi programa yo decido que dirección tiene
// caso contrario cuando se trabaja con un sensor, se debe de colocar la dirección descrita por el datasheet del sensor
# define SlaveAddress 0x40



uint8_t buffer = 0;
uint8_t valor_ADC;
volatile uint8_t adc_flag = 0;
uint8_t estado;
uint16_t distance = 0;
uint16_t tiempo = 0;

//************************************************************************************
// Function prototypes

void distancia();

//************************************************************************************
// Main Function
int main(void)
{
	DDRB |= (1 << DDB5);
	PORTB &= ~(1 << PORTB5); //Led para encender y apgar indicando una comunicación exitosa
	
	//inicializar ADC
	initADC();
	
	I2C_Slave_Init(SlaveAddress); //Se define la dirección del esclavo
	
	DDRD |= (1 << PORTD2) | (1 << PORTD4);   // TRIG salida | PORTD4 Motor
	DDRD &= ~(1 << PORTD3);  // ECHO entrada
	
	sei(); //Habilitar interrupciones
	
	//ADCSRA	|= (1 << ADSC);				// Se realiza la lectura de ADC
	
	while (1)
	{
		if(buffer == 'R'){ //Reviso si el caractér de lectura esta recibiendose
			//PINB |= (1 << PINB5); //Se hace un toggle para indicar que si hay datos 
			buffer = 0;
		}
		if(PIND & (1 << PORTD3))
		PORTB |= (1 << PORTB5);   // LED ON
		else
		PORTB &= ~(1 << PORTB5);  // LED OFF
		//distancia();
		if (distance <= 2)
		{
			// Apagar motor
			PORTD &= ~(1 << PORTD4);
		}
		else
		{
			PORTD |= (1 << PORTD4);
		}
		//Iniciar la secuencia de ADC
		/*if (adc_flag == 1)
		{
			ADCSRA	|= (1 << ADSC);				// Se realiza la lectura de ADC
			adc_flag = 0;
		}*/
	}
}

//************************************************************************************
// NON-INterrupt subroutines

void distancia()
{
	
	tiempo = 0;
	
	// TRIG se pone en 1 por 10us
	// Se envían 8 pulsos ultrasónicos y si hay rebote ECHO = 1 (detecta)
	// El tiempo que dura será = distancia
	
	// Para lectura correcta se pone en 0
	PORTD &= ~(1 << PORTD2);
	_delay_us(2);
	
	// Pulso de 10us acorde a datasheet
	PORTD |= (1 << PORTD2);
	_delay_us(10);
	
	// Se apaga el trig
	PORTD &= ~(1 << PORTD2);
	
	// Echo aquí aun es 0, esperar al cambio
	while (!(PIND & (1 << PORTD3)))
	// Echo aquí ya es 1, se cuenta el tiempo
	while (PIND & (1 << PORTD3))
	{
		// Medición de tiempo en us
		_delay_us(1);
		tiempo++;
	}
	
	// Vel sonido = 0.0343 cm / us
	// C/us equivale a 0.01715 cm reales -> 1 cm se tarda 58.3us
	distance  = tiempo/58;

	if (distance > 15)
	{
		distance = 15;
	}
}

//************************************************************************************
// Interrupt subroutines

ISR(ADC_vect)
{
	valor_ADC = ADCH;			// Se guarda ADCH
	adc_flag = 1;
}

ISR(TWI_vect){
	estado = TWSR & 0xFC; //Nos quedamos unicamente con los bits de estado TWI Status
	switch(estado){
		//**************************
		// Slave debe recibir dato
		//**************************
		case 0x60: //SLA+W recibido
		case 0x70: //General call
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE); //Indica "si te escuche"
			break;
		
		case 0x80: //Dato recibido, ACK enviado
		case 0x90: //Dato recibido General call, ACK enviado
			buffer = TWDR; //Ya puedo utilizar los datos
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
			break;
		//*****************************
		// Slave debe transmitir dato
		//*****************************
		//en cada case hay un comando que ya está predeterminado (ver presentacion)
		case 0xA8: //SLA+R recibido
		case 0xB8: //Dato transmitido, ACK recibido
			//TWDR = valor_ADC; //Dato a enviar
			TWDR = distance;
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
			break;
		//IMPORTANTE: que pasa si quiero enviar más de un dato?
		//Se puede hacer un arreglo por cada vez que envío un dato (ver grabación clase 1:08:11)
		
		case 0xC0: //Dato transmitido, NACK recibido
		case 0xC8: //Último dato transmitido
			TWCR = 0; //Limpio la interfaz para rebibir nuevo dato
			//TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN); //Reiniciarse
			TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE); // codigo correcto?
			break;
			
		case 0xA0: // STOP o repeated START recibido como slave
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
			break;
		//**********************
		// Cualquier error
		//**********************
		default:
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
			break;
		
	}
	
}


