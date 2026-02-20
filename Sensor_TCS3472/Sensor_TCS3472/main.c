 /*
 * Proyecto01_SLAVE.c
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

//Se define la dirección del esclavo: se debe de colocar la dirección descrita por el datasheet del sensor
# define TCS3472_ADDRESS 0x29

volatile uint8_t buffer = 0;
uint8_t valor_ADC;
volatile uint8_t adc_flag = 0;
uint8_t estado;

//************************************************************************************
// Function prototypes

void Init_TCS3472(void);

//************************************************************************************
// Main Function
int main(void)
{
	
	DDRD |= (1 << DDD4);
	PORTD &= ~((1 << PORTD2) | (1 << PORTD4));
	PORTD &= ~(1 << PORTD3);
	
	// Led para encender y apgar indicando una comunicación exitosa
	DDRB |= (1 << DDB5);
	PORTB &= ~(1 << PORTB5); 
	
	//inicializar ADC
	//initADC();
	
	I2C_Slave_Init(TCS3472_ADDRESS); //Se define la dirección del sensor
	
	
	sei(); //Habilitar interrupciones
	
	//ADCSRA	|= (1 << ADSC);				// Se realiza la lectura de ADC
	
	while (1)
	{
		PORTB |= (1 << PORTB5);   // LED ON
		PORTB &= ~(1 << PORTB5);  // LED OFF
		

		
		if(buffer == 'R'){ //Reviso si el caractér de lectura esta recibiendose
			PINB |= (1 << PINB5); //Se hace un toggle para indicar que si hay datos
			buffer = 0;
		}
		else if (buffer == 'L'){
			PORTD |= (1 << PORTD4);  // LED ON
			buffer = 0;
		}
		else if (buffer == 'S'){
			PORTD &= ~(1 << PORTD4); // LED OFF
			buffer = 0;
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
			TWDR = last_distance;
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


