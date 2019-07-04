/* Minimal V-USB joystick example. Runs on USBasp hardware.

Copyright (C) 2014 Shay Green
Licensed under GPL v2 or later. See License.txt. */
//#define USB_CFG_LONG_TRANSFERS	1

#define USE_FORCEFEEDBACK 1
#define USE_YPOS 1
#define USE_XPOS 1
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
volatile int8_t adc; 
#define YPIN1 PC5

#define BUTTON0 (_BV(0))
#define BUTTON1 (_BV(1))
#define BUTTON2 (_BV(2))
#define BUTTON3 (_BV(3))
#define BUTTON4 (_BV(4))
#define BUTTON5 (_BV(5))
#define BUTTON6 (_BV(6))
#define BUTTON7 (_BV(7))


//motor pin l298n
#define MOTORPWMPIN ( _BV(6))
#define MOTORPIN1 ( _BV(5))
#define MOTORPIN2 ( _BV(7))
//motor controll l298n
#define PWM(x) OCR2A=x 
#define MOTOROFF PORTD &= ~MOTORPWMPIN 
#define MOTORON  PORTD |=  MOTORPWMPIN 
#define MOTORCCW PORTD &= ~MOTORPIN1;PORTD |= MOTORPIN2 
#define MOTORCW  PORTD &= ~MOTORPIN2;PORTD |= MOTORPIN1 


//Програма инициализации ШИМ
static void init_pwm (void)
{

DDRD |= MOTORPWMPIN;
    // PD6 is now an output

    PWM(0x00);			//Начальная Мощность нулевая


    TCCR2A |= (1 << COM2A1);
    // set none-inverting mode

    TCCR2A |= (1 << WGM21) | (1 << WGM20);
    // set fast PWM Mode

    TCCR2B |= (1 << CS21);
    // set prescaler to 8 and starts PWM

  
  MOTOROFF;
 // TCCR1A=(1<<COM1A1)|(1<<WGM10); //На выводе OC1A единица, когда OCR1A==TCNT1, восьмибитный ШИМ
 // TCCR1B=(1<<CS10);		 //Делитель= /1

}

static void init_motor(void){
	init_pwm ();

	DDRD  |= MOTORPIN1;
	DDRD  |= MOTORPIN2;
	MOTORCCW;
}



void startadc(uchar adctouse)
{

	ADMUX	&=	0xf0;
    ADMUX |= adctouse;         // use #1 ADC
    ADMUX |= _BV(REFS0);    // use AVcc as the reference
    ADMUX &= ~_BV( ADLAR);   // clear for 10 bit resolution
    ADCSRA |= _BV( ADPS2) | _BV( ADPS1) | _BV( ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= _BV( ADEN);    // Enable the ADC
    ADCSRA |= _BV(ADIE);

	ADCSRA |= _BV(ADSC); // starts conversion
}

ISR(ADC_vect) //подпрограмма обработки прерывания от АЦП
{
	int ADCval;
	ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval; 
	adc=ADCval>>2; //считываем значение с АЦП и преобразуем его в значение от 0 до 8
	ADCSRA |= _BV(ADSC); // starts conversion
}

static void init_joy( void )
{

	

}


int _adc(uchar adctouse)
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
	//calcEncode();
	
	int8_t dx = encode_read1();
	//report [0] = dx;
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
		if(Xp<-127){MOTORON;MOTORCW;PWM(-(Xp+127)*48);}
		if(Xp>127){MOTORON;MOTORCCW;PWM((Xp-127)*48);}
		#endif
			}
	
	#ifdef USE_YPOS
	Ypos = adc-126;

	report [1] = (int8_t)(Ypos);
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

// X/Y joystick w/ 8-bit readings (-127 to +127), 8 digital buttons 42 bytes+28
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
	0xc0 /*,           // END_COLLECTION	
	
	
	
	0x09,0x66, // USAGE (Download Force Sample)
	0xA1,0x02, // COLLECTION (Logical)
	0x85,0x08, // REPORT_ID (08)
	0x05,0x01, // USAGE_PAGE (Generic Desktop)
	0x09,0x30, // USAGE (X)
	0x09,0x31, // USAGE (Y)
	0x15,0x81, // LOGICAL_MINIMUM (-127)
	0x25,0x7F, // LOGICAL_MAXIMUM (127)
	0x35,0x00, // PHYSICAL_MINIMUM (00)
	0x46,0xFF,0x00, // PHYSICAL_MAXIMUM (255)
	0x75,0x08, // REPORT_SIZE (08)
	0x95,0x02, // REPORT_COUNT (02)
	0x91,0x02, // OUTPUT (Data,Var,Abs)
	0xC0, // END COLLECTION ()
	*/
	
	
	
	
};


static uchar    idleRate;           /* in 4 ms units */
char _awaitReport;

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
	
	case USBRQ_HID_SET_REPORT: // LEDs on joystick?
	
	 _awaitReport = 1;
		return USB_NO_MSG;
	default:
		return 0;
	}
	
	

	
}


uchar   usbFunctionWrite(uchar *data, uchar len)
{
   if (!_awaitReport || len < 1)
      return 1;
   if(data[0]<127){MOTORON;MOTORCCW;PWM((data[0]));} ;
   if(data[0]>127){MOTORON;MOTORCW;PWM(127-(data[0]));} ;
   _awaitReport = 0;
    return 1;
}


int main( void )
{
	encode_init();
	startadc(YPIN1);
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