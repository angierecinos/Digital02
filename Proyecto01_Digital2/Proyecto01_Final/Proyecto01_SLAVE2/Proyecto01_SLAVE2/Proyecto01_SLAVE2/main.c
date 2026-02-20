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
# include "HX711/HX711.h"
# include "UART/UART.h"
# include "PWM/PWM.h"

//Se define la dirección del esclavo, en este caso como es mi programa yo decido que dirección tiene
// caso contrario cuando se trabaja con un sensor, se debe de colocar la dirección descrita por el datasheet del sensor
# define SlaveAddress 0x40

uint8_t buffer = 0;
uint8_t valor_ADC;
volatile uint8_t adc_flag = 0;
uint8_t estado;
uint16_t distance = 0;
uint16_t tiempo = 0;
int32_t offset;
volatile int32_t hx_raw = 0;
volatile uint16_t peso_g = 0;
int32_t scale  = 394;      // cuentas por gramo
uint8_t stepper_count = 1;
uint8_t steps = 0;
uint8_t stepper = 0;

# define angulo1_servo 170
# define angulo2_servo 110


//************************************************************************************
// Function prototypes
void distancia();
void setup();

//************************************************************************************
// Main Function
int main(void)
{
	setup();
	initHX711();
	_delay_ms(200);               // estabilizar HX711
	
	initPWM1A(non_invert, 8);

	// 3) Calcular offset (tara) al encender (sin carga)
	offset = HX711_Tare(16);

	// 4) Init I2C Slave
	I2C_Slave_Init(SlaveAddress);

	// 5) Habilitar interrupciones globales
	sei();

	// 6) Loop principal: mantener peso actualizado aunque el master no lo pida
	while (1)
	{
		if (HX711_IsReady())              // solo cuando hay dato nuevo
		{
			int32_t raw = HX711_ReadRaw();  // lectura cruda (signed)

			int32_t diff = raw - offset;    // quitar tara
			if (diff < 0) diff = -diff;     // valor absoluto (porque la señal está invertida)

			// convertir a gramos (entero)
			uint16_t g = (uint16_t)(diff / scale);

			// guardar global para que ISR lo envíe
			peso_g = g;
		}

		 _delay_ms(2);
		
		// Dependiendo del comando que reciba se carga al servo la posición necesaria
		if (buffer == 'B'){
			//OCR1A = 1500;
			servo_position1A(angulo1_servo);
			buffer = 0;
		}
		else if (buffer == 'Y'){
			//OCR1A = 5000;
			servo_position1A(angulo2_servo);
			buffer = 0;
		}
		
		// Si se recibe T se comienza la secuencia del stepepr
		if (buffer == 'T'){
			stepper = buffer;	
		}
		// Se realiza el movimiento del stepper 
		while (stepper == 'T')
			{
				// Se suman los pasos del stepper hasta llegar a 100 pasos
				steps++;
				if (steps == 100)
				{
					steps = 0;
					stepper = 0; // Se reinician las variables para salir del ciclo
				}
				
				switch (stepper_count){
					// Se enciende la primera bobina
					case 1:
					PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5));
					PORTD |= (1 << PORTD2);
					stepper_count = 2;
					_delay_ms(3);
					break;
			
					// Se enciende la segunda bobina
					case 2:
					PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5));
					PORTD |= (1 << PORTD3);
					stepper_count = 3;
					_delay_ms(3);
					break;
			
					// Se enciende la tercera bobina
					case 3:
					PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5));
					PORTD |= (1 << PORTD4);
					stepper_count = 4;
					_delay_ms(3);
					break;
			
					// Se enciende la última bobina
					case 4:
					PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5));
					PORTD |= (1 << PORTD5);
					stepper_count = 1;
					_delay_ms(3);
					break;
				}
			}
			
			// Si se recibe 'A' entonces se realiza la secuencia inversa
			if (buffer == 'A'){
				stepper = buffer;
			}
			
			while (stepper == 'A')
			{
				// Se cuentan 100 pasos de duración del ciclo
				steps++;
				if (steps == 100)
				{
					steps = 0;	// Se reinician las variables para salir del ciclo
					stepper = 0;
				}
				
				switch (stepper_count){
					// Se enciende la cuarta bobina
					case 1:
					PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5));
					PORTD |= (1 << PORTD5);
					stepper_count = 2;
					_delay_ms(3);
					break;
					
					// Se enciende la tercera bobina
					case 2:
					PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5));
					PORTD |= (1 << PORTD4);
					stepper_count = 3;
					_delay_ms(3);
					break;
					
					// Se enciende la segunda bobina
					case 3:
					PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5));
					PORTD |= (1 << PORTD3);
					stepper_count = 4;
					_delay_ms(3);
					break;
					
					// Se enciende la última bobina
					case 4:
					PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5));
					PORTD |= (1 << PORTD2);
					stepper_count = 1;
					_delay_ms(3);
					break;
				}
			}
	}
}

//************************************************************************************
// NON-INterrupt subroutines
void setup()
{
	// Inicializo los pines del puerto D a utilizar para el Stepper
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5);
	PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5));
}

//************************************************************************************
// Interrupt subroutines
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
		case 0xA8: // SLA+R recibido -> primer byte
		TWDR = (uint8_t)(peso_g >> 8); // MSB
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;

		case 0xB8: // Byte transmitido, ACK recibido -> segundo byte
		TWDR = (uint8_t)(peso_g & 0xFF); // LSB
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


