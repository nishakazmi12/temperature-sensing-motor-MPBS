#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>
#include "avr/iom162.h"

void adc(void);
void read_adc(void);

int main(void)
{
	DDRC = 0x00;					//input of micro-controller from ADC
	DDRE = 0xFF;					//output to ADC select interrupt
	DDRB = 0x77;					//PB0 = OC0 PB1 = OC2 PB2 =  (unconnected) PB3 = EOC PD4 = OE PB5 = ALE   PB6 = START  PB1 = OC2 
	DDRD = 0xFF;					// output to LED
	DDRA = 0x00;

	TCCR0|= (1<<WGM01)|(1<<COM00)|(1<<CS00);		  // WGM = 10  CTC
												  // COM= 01 toggle on compare clear
												  //CS = 001 no pre-scaler. 

	TCCR2|= (1<<CS21)|(1<<WGM20)|(1<<COM21);		  //CS = 010 8 pre-scaler  WGM = 10 phase correct PWM  
												  //|(1<<COM20)|(1<<CS22); //|(1<<WGM21)
												  // COM= 10 clear OCR2 on compare match (inverted)
	OCR0|= 2;					// 2 cycles 1 pulse
	PORTB|= 0x08;				//OE set to 1     
	TIMSK = 0x00;				//Timer Interrupt mask


	while (1)
	{
		adc();
	}

}



void adc(void)
{
	PORTE = 0x00;				//IN0 set
	read_adc();
}



void read_adc()
{
	char SW;
	unsigned char num;
	PORTB = PORTB| 0x60;			// ALE and START set to 1
	_delay_ms(100);
	PORTB = PORTB & 0x2F;		// ALE and START reset to 0
	while((PINB&0x08)==0x08);  
	while((PINB&0x08)==0x00);
	PORTB = PORTB | (0x10);		// OE set to 1
	_delay_ms(200);
	num = PINC;   
	PORTD = num;					//converted bits send to PORTD
	PORTB = PORTB&(0xEF);		// OE reset to 0
	_delay_ms(100);
	SW = PINA & 0x03;

	switch(SW)
	{
		case 0:							  //fan off
		{
			OCR2 = 0;
			break;
		}

		case 1:							 //fan on
		{
			OCR2 = 128;					 //50% duty cycle
			break;
		}

		case 2:							 //Adjusted Values
		{
			if (num==0x00)
			{
				OCR2 = 0;				//0% duty cycle
			}
			else if (num>=0x02 && num<=0x4F)
			{
				OCR2 = 62;				//24% duty cycle
			}
			else if (num>=0x52 && num<=0x9F)
			{
				OCR2 = 102;				//40% duty cycle
			}
			else if (num>=0xA1 && num<=0xEF)
			{
				OCR2 = 192;				// 75% duty cycle
			}
			else if (num>=0xF0 && num>=0xFF)
			{
				OCR2 = 230;				//90% duty cycle
				
			}
			break;
		}

		default:
		{
			OCR2 = 0;
			break;
		}
	}

}
