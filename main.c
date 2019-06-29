/* Minimal V-USB joystick example. Runs on USBasp hardware.

Copyright (C) 2014 Shay Green
Licensed under GPL v2 or later. See License.txt. */
//#define USB_CFG_LONG_TRANSFERS	1

#define USE_FORCEFEEDBACK 1
//#define USE_YPOS 1
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "usbdrv/usbdrv.h"
#include "rotary.h"

// Report format: Y, X, buttons (up to 8)
static uint8_t report [3]; // current
static uint8_t report_out [3]; // last sent over USB
static long Xpos = 0;
static long Ypos = 0;
static int divider= 2;
static int multiplier= 1;
#define YPIN1 ( 1<<PC3)
#define YPIN2 ( 1<<PC0)
#define BUTTON0 (_BV(0))
#define BUTTON1 (_BV(1))
#define BUTTON2 (_BV(2))
#define BUTTON3 (_BV(3))
#define BUTTON4 (_BV(4))
#define BUTTON5 (_BV(5))
#define BUTTON6 (_BV(6))
#define BUTTON7 (_BV(7))


//motor pin l298n
#define MOTORPWMPIN ( _BV(1))
#define MOTORPIN1 ( _BV(5))
#define MOTORPIN2 ( _BV(6))
//motor controll l298n
#define PWM(x) OCR1A=x 
#define MOTOROFF PORTB &= ~MOTORPWMPIN 
#define MOTORON  PORTB |=  MOTORPWMPIN 
#define MOTORCCW PORTD &= ~MOTORPIN1;PORTD |= MOTORPIN2 
#define MOTORCW  PORTD &= ~MOTORPIN2;PORTD |= MOTORPIN1 


//Програма инициализации ШИМ
static void init_pwm (void)
{
  DDRB  |=  MOTORPWMPIN;
  MOTOROFF;
  TCCR1A=(1<<COM1A1)|(1<<WGM10); //На выводе OC1A единица, когда OCR1A==TCNT1, восьмибитный ШИМ
  TCCR1B=(1<<CS10);		 //Делитель= /1
  PWM(0x00);			//Начальная Мощность нулевая
}

static void init_motor(void){
	init_pwm ();

	DDRD  |= MOTORPIN1;
	DDRD  |= MOTORPIN2;
	MOTORCCW;
}




static void init_joy( void )
{

	

}


int adc(uchar adctouse)
{
    int ADCval;

    ADMUX = adctouse;         // use #1 ADC
    ADMUX |= (1 << REFS0);    // use AVcc as the reference
    ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= (1 << ADEN);    // Enable the ADC

    ADCSRA |= (1 << ADSC);    // Start the ADC conversion

    while(ADCSRA & (1 << ADSC));      // Thanks T, this line waits for the ADC to finish 


    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again

    return ADCval;
}



static void read_joy( void )
{
	//report [0] = 0;
	//report [1] = 0;
	report [2] = 0;
	
	int8_t dx = encode_read1();
	if(dx!=0){
		Xpos +=(int)dx;
		int Xp = (multiplier * Xpos) / divider;

		if(Xp<127&&Xp>-127){
		report [0] = (int8_t)Xp;
		#ifdef USE_FORCEFEEDBACK
		PWM(0x00);MOTOROFF;
		#endif
		}
		#ifdef USE_FORCEFEEDBACK
		if(Xp<-127){MOTORON;MOTORCW;PWM(0xFF);}
		if(Xp>127){MOTORON;MOTORCCW;PWM(0xFF);}
		#endif
			}
	
	
	Ypos = adc(YPIN1);
	Ypos -=adc(YPIN2);
	#ifdef USE_YPOS
	report [1] = (int8_t)(Ypos/8);
	#endif
	
	// Buttons

	//if ( ! (PINB & 0x01) ) report [2] |= 0x04;
	//if ( ! (PINB & 0x04) ) report [2] |= 0x08;

	
	
	
	/*
	 if(encode_readKey()==1){
	
	 	divider = 1+(divider&3);
	 	multiplier = divider-1;
	 	if(multiplier<1)multiplier=1;
    
    }
	 */

}

// X/Y joystick w/ 8-bit readings (-127 to +127), 8 digital buttons
PROGMEM const char usbHidReportDescriptor [USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	0x05, 0x01,     // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,     // USAGE (Game Pad)
	0xa1, 0x01,     // COLLECTION (Application)
	0x09, 0x01,     //   USAGE (Pointer)
	0xa1, 0x00,     //   COLLECTION (Physical)
	0x09, 0x30,     //     USAGE (X)
	0x09, 0x31,     //     USAGE (Y)
	0x15, 0x81,     //   LOGICAL_MINIMUM (-127)
	0x25, 0x7f,     //   LOGICAL_MAXIMUM (127)
	0x75, 0x08,     //   REPORT_SIZE (8)
	0x95, 0x02,     //   REPORT_COUNT (2)
	0x81, 0x02,     //   INPUT (Data,Var,Abs)
	0xc0,           // END_COLLECTION
	0x05, 0x09,     // USAGE_PAGE (Button)
	0x19, 0x01,     //   USAGE_MINIMUM (Button 1)
	0x29, 0x08,     //   USAGE_MAXIMUM (Button 8)
	0x15, 0x00,     //   LOGICAL_MINIMUM (0)
	0x25, 0x01,     //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,     // REPORT_SIZE (1)
	0x95, 0x08,     // REPORT_COUNT (8)
	0x81, 0x02,     // INPUT (Data,Var,Abs)
	0xc0            // END_COLLECTION
};

uint8_t usbFunctionSetup( uint8_t data [8] )
{
	usbRequest_t const* rq = (usbRequest_t const*) data;

	if ( (rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS )
		return 0;
	
	switch ( rq->bRequest )
	{
	case USBRQ_HID_GET_REPORT: // HID joystick only has to handle this
		usbMsgPtr = (usbMsgPtr_t) report_out;
		return sizeof report_out;
	
	//case USBRQ_HID_SET_REPORT: // LEDs on joystick?
	
	default:
		return 0;
	}
}



int main( void )
{
	encode_init();
	usbInit();
	sei();
	
	init_joy();
	#ifdef USE_FORCEFEEDBACK
	init_motor();
	#endif
	for ( ;; )
	{
		usbPoll();
		
		// Don't bother reading joy if previous changes haven't gone out yet.
		// Forces delay after changes which serves to debounce controller as well.
		if ( usbInterruptIsReady() )
		{
			read_joy();

			
			// Don't send update unless joystick changed
			if ( memcmp( report_out, report, sizeof report ) )
			{
				memcpy( report_out, report, sizeof report );
				usbSetInterrupt( report_out, sizeof report_out );
				//toggle_led();
			}
		}
	}
	
	return 0;
}
