/*
 * SPI.c
 *
 * Created: 29/01/2026 10:35:44
 *  Author: Usuario
 */ 

#include "SPI.h"
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

void initSPI(SPI_Type sType, SPI_Data_Order sDataOrder, SPI_Clock_Polarity sClockPolarity, SPI_Clock_Phase sClockPhase)
{
	// PB2 -> SS | PB3 -> MOSI | PB4 -> MISO | PB5 -> SCK
	
	if (sType & (1 << MSTR))
	{
		//		MOSI - Output, SCK - clk , NEGADO_SS
		DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2);
		//		MISO - Input
		DDRB &= ~(1 << DDB4); 
		//		Master en SPCR
		SPCR |= (1 << MSTR);
		
		uint8_t temp = sType & 0b00000111;
		switch (temp){
			
			// DIV2
			case 0:
				SPCR &= ~((1 << SPR1) | (1 << SPR0));
				SPSR |= (1 << SPI2X);
			break;
			
			// DIV4
			case 1:
				SPCR &= ~((1 << SPR1) | (1 << SPR0));
				SPSR &= ~(1 << SPI2X);
			break;
			
			// DIV8
			case 2:
				SPCR |= (1 << SPR0);
				SPCR &= ~(1 << SPR1);
				SPSR |= (1 << SPI2X);
			break;
			
			// DIV16
			case 3:
				SPCR |= (1 << SPR0);
				SPCR &= ~(1 << SPR1);
				SPSR &= ~(1 << SPI2X);
			break;
			
			// DIV32
			case 4:
				SPCR &= ~(1 << SPR0);
				SPCR |= (1 << SPR1);
				SPSR |= (1 << SPI2X);
			break;
			
			// DIV64
			case 5:
				SPCR &= ~(1 << SPR0);
				SPCR |= (1 << SPR1);
				SPSR &= ~(1 << SPI2X);
			break;
			
			// DIV128
			case 6:
				SPCR |= (1 << SPR0) | (1 << SPR1); 
				SPSR &= ~(1 << SPI2X);
			break;
		}
	}
	else
		{
			// Slave mode
			//		MISO - Output
			DDRB |= (1 << DDB4); 
			//			MOSI - Input, SCK - clk , NEGADO_SS
			DDRB &= ~((1 << DDB3) | (1 << DDB5) | (1 << DDB2));
			// Slave
			SPCR &= ~(1 << MSTR);
		}
		// Enable other bits
		SPCR |= (1 << SPE) | sDataOrder | sClockPolarity | sClockPhase;
	 
}

/*static void spiReceiveWait()
{
	// Wait for data 2 fully receive
	while(!(SPSR & (1 << SPIF)));
}*/

void spiWrite(uint8_t dato)
{
	SPDR = dato;
}

unsigned spiDataReady()
{
	if(SPSR & (1 << SPIF))
	return 1;
	else
	return 0;
}

uint8_t spiRead(void)
{
	while (!(SPSR & (1 << SPIF)));	// Wait receive complete
	return(SPDR);					// Read from buffer
}