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

#define reportLen 5
// Report format: Y, X, buttons (up to 8)
static uint8_t report [reportLen]; // current
static uint8_t report_out [reportLen]; // last sent over USB
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
	Ypos = 127 - adc;

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

/*  first ver work
PROGMEM const char usbHidReportDescriptor [] = {
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
	 0x81, 0x02,     // INPUT (Data,Var,Abs)   // 16 bytes
	0xc0 ,           // END_COLLECTION	42 bytes */


// X/Y joystick w/ 8-bit readings (-127 to +127), 8 digital buttons 42 bytes+19  USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH
PROGMEM const char usbHidReportDescriptor [] = {
	0x05, 0x01,     // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,     // USAGE (Game Pad)
	0xa1, 0x01,     // COLLECTION (Application)
	 0x09, 0x01,     //   USAGE (Pointer)
	 0xa1, 0x00,     //   COLLECTION (Physical)
	  0x09, 0x30,     //     USAGE (X)
	  0x09, 0x31,     //     USAGE (Y)
	  0x09, 0xbb, // USAGE (Throttle)
	  0x09, 0x34,         //          Usage (Ry),                
	  0x15, 0x81,     //   LOGICAL_MINIMUM (-127)
	  0x25, 0x7f,     //   LOGICAL_MAXIMUM (127)
	  0x75, 0x08,     //   REPORT_SIZE (8)
	  0x95, 0x02,     //   REPORT_COUNT (2)
	  0x81, 0x02,     //   INPUT (Data,Var,Abs)
	 0xc0,           // END_COLLECTION //29
	

	
	
	 0x05, 0x09,     // USAGE_PAGE (Button)
	 0x19, 0x01,     //   USAGE_MINIMUM (Button 1)
	 0x29, 0x08,     //   USAGE_MAXIMUM (Button 8)
	 0x15, 0x00,     //   LOGICAL_MINIMUM (0)
	 0x25, 0x01,     //   LOGICAL_MAXIMUM (1)
	 0x75, 0x01,     // REPORT_SIZE (1)
	 0x95, 0x08,     // REPORT_COUNT (8)
	 0x81, 0x02,     // INPUT (Data,Var,Abs)   // 16 bytes
	0xc0 ,           // END_COLLECTION	46 bytes
	
	
	
	      // ====== Virtual PID force feedback ======= //

    0x05,0x0F,        //    Usage Page Physical Interface
    0x09,0x92,        //    Usage ES Playing
    0xA1,0x02,        //    Collection Datalink
    0x85,0x20,    //    Report ID 20h
    0x09,0x9F,    //    Usage (Device Paused)
    0x09,0xA0,    //    Usage (Actuators Enabled)
    0x09,0xA4,    //    Usage (Safety Switch)
    0x09,0xA5,    //    Usage (Actuator Override Switch)
    0x09,0xA6,    //    Usage (Actuator Power)
    0x15,0x00,    //    Logical Minimum 0
    0x25,0x01,    //    Logical Maximum 1
    0x35,0x00,    //    Physical Minimum 0
    0x45,0x01,    //    Physical Maximum 1
    0x75,0x01,    //    Report Size 1
    0x95,0x05,    //    Report Count 5
    0x81,0x02,    //    Input (Variable)
    0x95,0x03,    //    Report Count 3
    0x75,0x01,    //    Report Size 1
    0x81,0x03,    //    Input (Constant, Variable)
    0x09,0x94,    //    Usage (Effect Playing)
    0x15,0x00,    //    Logical Minimum 0
    0x25,0x01,    //    Logical Maximum 1
    0x35,0x00,    //    Physical Minimum 0
    0x45,0x01,    //    Physical Maximum 1
    0x75,0x01,    //    Report Size 1
    0x95,0x01,    //    Report Count 1
    0x81,0x02,    //    Input (Variable)
    0x09,0x22,    //    Usage Effect Block Index
    0x15,0x01,    //    Logical Minimum 1
    0x25,0x28,    //    Logical Maximum 28h (40d)
    0x35,0x01,    //    Physical Minimum 1
    0x45,0x28,    //    Physical Maximum 28h (40d)
    0x75,0x07,    //    Report Size 7
    0x95,0x01,    //    Report Count 1
    0x81,0x02,    //    Input (Variable)
    0xC0    ,    // End Collection


    0x09,0x21,    //    Usage Set Effect Report
    0xA1,0x02,    //    Collection Datalink
    0x85,0x21,    //    Report ID 21h
    0x09,0x22,    //    Usage Effect Block Index
    0x15,0x01,    //    Logical Minimum 1
    0x25,0x28,    //    Logical Maximum 28h (40d)
    0x35,0x01,    //    Physical Minimum 1
    0x45,0x28,    //    Physical Maximum 28h (40d)
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0x91,0x02,    //    Output (Variable)
    0x09,0x25,    //    Usage Effect Type
    0xA1,0x02,    //    Collection Datalink
    0x09,0x26,    //    Usage ET Constant Force
    0x09,0x27,    //    Usage ET Ramp
    0x09,0x30,    //    Usage ET Square
    0x09,0x31,    //    Usage ET Sine
    0x09,0x32,    //    Usage ET Triangle
    0x09,0x33,    //    Usage ET Sawtooth Up
    0x09,0x34,    //    Usage ET Sawtooth Down
    0x09,0x40,    //    Usage ET Spring
    0x09,0x41,    //    Usage ET Damper
    0x09,0x42,    //    Usage ET Inertia
    0x09,0x43,    //    Usage ET Friction
    0x09,0x28,    //    Usage ET Custom Force Data
    0x25,0x0C,    //    Logical Maximum Ch (12d)
    0x15,0x01,    //    Logical Minimum 1
    0x35,0x01,    //    Physical Minimum 1
    0x45,0x0C,    //    Physical Maximum Ch (12d)
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0x91,0x00,    //    Output
    0xC0    ,          //    End Collection

    0x09,0x50,         //    Usage Duration
    0x09,0x54,         //    Usage Trigger Repeat Interval
    0x09,0x51,         //    Usage Sample Period
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x7F,    //    Logical Maximum 7FFFh (32767d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0xFF,0x7F,    //    Physical Maximum 7FFFh (32767d)
    0x66,0x03,0x10,    //    Unit 1003h (4099d)
    0x55,0xFD,         //    Unit Exponent FDh (253d)
    0x75,0x10,         //    Report Size 10h (16d)
    0x95,0x03,         //    Report Count 3
    0x91,0x02,         //    Output (Variable)
    0x55,0x00,         //    Unit Exponent 0
    0x66,0x00,0x00,    //    Unit 0
    0x09,0x52,         //    Usage Gain
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x00,    //    Logical Maximum FFh (255d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x09,0x53,         //    Usage Trigger Button
    0x15,0x01,         //    Logical Minimum 1
    0x25,0x08,         //    Logical Maximum 8
    0x35,0x01,         //    Physical Minimum 1
    0x45,0x08,         //    Physical Maximum 8
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x09,0x55,         //    Usage Axes Enable
    0xA1,0x02,         //    Collection Datalink
    0x05,0x01,    //    Usage Page Generic Desktop
    0x09,0x30,    //    Usage X
    0x09,0x31,    //    Usage Y
    0x15,0x00,    //    Logical Minimum 0
    0x25,0x01,    //    Logical Maximum 1
    0x75,0x01,    //    Report Size 1
    0x95,0x02,    //    Report Count 2
    0x91,0x02,    //    Output (Variable)
    0xC0     ,    // End Collection
    0x05,0x0F,    //    Usage Page Physical Interface
    0x09,0x56,    //    Usage Direction Enable
    0x95,0x01,    //    Report Count 1
    0x91,0x02,    //    Output (Variable)
    0x95,0x05,    //    Report Count 5
    0x91,0x03,    //    Output (Constant, Variable)
    0x09,0x57,    //    Usage Direction
    0xA1,0x02,    //    Collection Datalink
    0x0B,0x01,0x00,0x0A,0x00,    //    Usage Ordinals: Instance 1
    0x0B,0x02,0x00,0x0A,0x00,    //    Usage Ordinals: Instance 2
    0x66,0x14,0x00,              //    Unit 14h (20d)
    0x55,0xFE,                   //    Unit Exponent FEh (254d)
    0x15,0x00,                   //    Logical Minimum 0
    0x26,0xFF,0x00,              //    Logical Maximum FFh (255d)
    0x35,0x00,                   //    Physical Minimum 0
    0x47,0xA0,0x8C,0x00,0x00,    //    Physical Maximum 8CA0h (36000d)
    0x66,0x00,0x00,              //    Unit 0
    0x75,0x08,                   //    Report Size 8
    0x95,0x02,                   //    Report Count 2
    0x91,0x02,                   //    Output (Variable)
    0x55,0x00,                   //    Unit Exponent 0
    0x66,0x00,0x00,              //    Unit 0
    0xC0     ,         //    End Collection
    0x05,0x0F,         //    Usage Page Physical Interface
    0x09,0xA7,         //    Usage Undefined
    0x66,0x03,0x10,    //    Unit 1003h (4099d)
    0x55,0xFD,         //    Unit Exponent FDh (253d)
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x7F,    //    Logical Maximum 7FFFh (32767d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0xFF,0x7F,    //    Physical Maximum 7FFFh (32767d)
    0x75,0x10,         //    Report Size 10h (16d)
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x66,0x00,0x00,    //    Unit 0
    0x55,0x00,         //    Unit Exponent 0
    0xC0     ,    //    End Collection
    0x05,0x0F,    //    Usage Page Physical Interface
    0x09,0x5A,    //    Usage Set Envelope Report
    0xA1,0x02,    //    Collection Datalink
    0x85, 0x20,    //    Report ID 20h
    0x09,0x22,         //    Usage Effect Block Index
    0x15,0x01,         //    Logical Minimum 1
    0x25,0x28,         //    Logical Maximum 28h (40d)
    0x35,0x01,         //    Physical Minimum 1
    0x45,0x28,         //    Physical Maximum 28h (40d)
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x09,0x5B,         //    Usage Attack Level
    0x09,0x5D,         //    Usage Fade Level
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x00,    //    Logical Maximum FFh (255d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x95,0x02,         //    Report Count 2
    0x91,0x02,         //    Output (Variable)
    0x09,0x5C,         //    Usage Attack Time
    0x09,0x5E,         //    Usage Fade Time
    0x66,0x03,0x10,    //    Unit 1003h (4099d)
    0x55,0xFD,         //    Unit Exponent FDh (253d)
    0x26,0xFF,0x7F,    //    Logical Maximum 7FFFh (32767d)
    0x46,0xFF,0x7F,    //    Physical Maximum 7FFFh (32767d)
    0x75,0x10,         //    Report Size 10h (16d)
    0x91,0x02,         //    Output (Variable)
    0x45,0x00,         //    Physical Maximum 0
    0x66,0x00,0x00,    //    Unit 0
    0x55,0x00,         //    Unit Exponent 0
    0xC0     ,            //    End Collection
    0x09,0x5F,    //    Usage Set Condition Report
    0xA1,0x02,    //    Collection Datalink
    0x85,0x03,    //    Report ID 3
    0x09,0x22,    //    Usage Effect Block Index
    0x15,0x01,    //    Logical Minimum 1
    0x25,0x28,    //    Logical Maximum 28h (40d)
    0x35,0x01,    //    Physical Minimum 1
    0x45,0x28,    //    Physical Maximum 28h (40d)
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0x91,0x02,    //    Output (Variable)
    0x09,0x23,    //    Usage Parameter Block Offset
    0x15,0x00,    //    Logical Minimum 0
    0x25,0x01,    //    Logical Maximum 1
    0x35,0x00,    //    Physical Minimum 0
    0x45,0x01,    //    Physical Maximum 1
    0x75,0x04,    //    Report Size 4
    0x95,0x01,    //    Report Count 1
    0x91,0x02,    //    Output (Variable)
    0x09,0x58,    //    Usage Type Specific Block Off...
    0xA1,0x02,    //    Collection Datalink
    0x0B,0x01,0x00,0x0A,0x00,    //    Usage Ordinals: Instance 1
    0x0B,0x02,0x00,0x0A,0x00,    //    Usage Ordinals: Instance 2
    0x75,0x02,                   //    Report Size 2
    0x95,0x02,                   //    Report Count 2
    0x91,0x02,                   //    Output (Variable)
    0xC0     ,         //    End Collection
    0x15,0x80,         //    Logical Minimum 80h (-128d)
    0x25,0x7F,         //    Logical Maximum 7Fh (127d)
    0x36,0xF0,0xD8,    //    Physical Minimum D8F0h (-10000d)
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x09,0x60,         //    Usage CP Offset
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x36,0xF0,0xD8,    //    Physical Minimum D8F0h (-10000d)
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x09,0x61,         //    Usage Positive Coefficient
    0x09,0x62,         //    Usage Negative Coefficient
    0x95,0x02,         //    Report Count 2
    0x91,0x02,         //    Output (Variable)
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x00,    //    Logical Maximum FFh (255d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x09,0x63,         //    Usage Positive Saturation
    0x09,0x64,         //    Usage Negative Saturation
    0x75,0x08,         //    Report Size 8
    0x95,0x02,         //    Report Count 2
    0x91,0x02,         //    Output (Variable)
    0x09,0x65,         //    Usage Dead Band
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0xC0     ,    //    End Collection
    0x09,0x6E,    //    Usage Set Periodic Report
    0xA1,0x02,    //    Collection Datalink
    0x85,0x04,                   //    Report ID 4
    0x09,0x22,                   //    Usage Effect Block Index
    0x15,0x01,                   //    Logical Minimum 1
    0x25,0x28,                   //    Logical Maximum 28h (40d)
    0x35,0x01,                   //    Physical Minimum 1
    0x45,0x28,                   //    Physical Maximum 28h (40d)
    0x75,0x08,                   //    Report Size 8
    0x95,0x01,                   //    Report Count 1
    0x91,0x02,                   //    Output (Variable)
    0x09,0x70,                   //   Usage Magnitude
    0x15,0x00,                   //    Logical Minimum 0
    0x26,0xFF,0x00,              //    Logical Maximum FFh (255d)
    0x35,0x00,                   //    Physical Minimum 0
    0x46,0x10,0x27,              //    Physical Maximum 2710h (10000d)
    0x75,0x08,                   //    Report Size 8
    0x95,0x01,                   //    Report Count 1
    0x91,0x02,                   //    Output (Variable)
    0x09,0x6F,                   //   Usage Offset
    0x15,0x80,                   //    Logical Minimum 80h (-128d)
    0x25,0x7F,                   //    Logical Maximum 7Fh (127d)
    0x36,0xF0,0xD8,              //    Physical Minimum D8F0h (-10000d)
    0x46,0x10,0x27,              //    Physical Maximum 2710h (10000d)
    0x95,0x01,                   //    Report Count 1
    0x91,0x02,                   //    Output (Variable)
    0x09,0x71,                   //   Usage Phase
    0x66,0x14,0x00,              //    Unit 14h (20d)
    0x55,0xFE,                   //    Unit Exponent FEh (254d)
    0x15,0x00,                   //    Logical Minimum 0
    0x26,0xFF,0x00,              //    Logical Maximum FFh (255d)
    0x35,0x00,                   //    Physical Minimum 0
    0x47,0xA0,0x8C,0x00,0x00,    //    Physical Maximum 8CA0h (36000d)
    0x91,0x02,                   //    Output (Variable)
    0x09,0x72,                   //   Usage Period
    0x26,0xFF,0x7F,              //    Logical Maximum 7FFFh (32767d)
    0x46,0xFF,0x7F,              //    Physical Maximum 7FFFh (32767d)
    0x66,0x03,0x10,              //    Unit 1003h (4099d)
    0x55,0xFD,                   //    Unit Exponent FDh (253d)
    0x75,0x10,                   //    Report Size 10h (16d)
    0x95,0x01,                   //    Report Count 1
    0x91,0x02,                   //    Output (Variable)
    0x66,0x00,0x00,              //    Unit 0
    0x55,0x00,                   //    Unit Exponent 0
    0xC0     ,    // End Collection
    0x09,0x73,    //    Usage Set Constant Force Rep...
    0xA1,0x02,    //    Collection Datalink
    0x85,0x05,         //    Report ID 5
    0x09,0x22,         //    Usage Effect Block Index
    0x15,0x01,         //    Logical Minimum 1
    0x25,0x28,         //    Logical Maximum 28h (40d)
    0x35,0x01,         //    Physical Minimum 1
    0x45,0x28,         //    Physical Maximum 28h (40d)
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x09,0x70,         //    Usage Magnitude
    0x16,0x01,0xFF,    //    Logical Minimum FF01h (-255d)
    0x26,0xFF,0x00,    //    Logical Maximum FFh (255d)
    0x36,0xF0,0xD8,    //    Physical Minimum D8F0h (-10000d)
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x75,0x10,         //    Report Size 10h (16d)
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0xC0     ,    //    End Collection
    0x09,0x74,    //    Usage Set Ramp Force Report
    0xA1,0x02,    //    Collection Datalink
    0x85,0x06,         //    Report ID 6
    0x09,0x22,         //    Usage Effect Block Index
    0x15,0x01,         //    Logical Minimum 1
    0x25,0x28,         //    Logical Maximum 28h (40d)
    0x35,0x01,         //    Physical Minimum 1
    0x45,0x28,         //    Physical Maximum 28h (40d)
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x09,0x75,         //    Usage Ramp Start
    0x09,0x76,         //    Usage Ramp End
    0x15,0x80,         //    Logical Minimum 80h (-128d)
    0x25,0x7F,         //    Logical Maximum 7Fh (127d)
    0x36,0xF0,0xD8,    //    Physical Minimum D8F0h (-10000d)
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x75,0x08,         //    Report Size 8
    0x95,0x02,         //    Report Count 2
    0x91,0x02,         //    Output (Variable)
    0xC0     ,    //    End Collection
    0x09,0x68,    //    Usage Custom Force Data Rep...
    0xA1,0x02,    //    Collection Datalink
    0x85,0x07,         //    Report ID 7
    0x09,0x22,         //    Usage Effect Block Index
    0x15,0x01,         //    Logical Minimum 1
    0x25,0x28,         //    Logical Maximum 28h (40d)
    0x35,0x01,         //    Physical Minimum 1
    0x45,0x28,         //    Physical Maximum 28h (40d)
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x09,0x6C,         //    Usage Custom Force Data Offset
    0x15,0x00,         //    Logical Minimum 0
    0x26,0x10,0x27,    //    Logical Maximum 2710h (10000d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x75,0x10,         //    Report Size 10h (16d)
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x09,0x69,         //    Usage Custom Force Data
    0x15,0x81,         //    Logical Minimum 81h (-127d)
    0x25,0x7F,         //    Logical Maximum 7Fh (127d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0xFF,0x00,    //    Physical Maximum FFh (255d)
    0x75,0x08,         //    Report Size 8
    0x95,0x0C,         //    Report Count Ch (12d)
    0x92,0x02,0x01,    //       Output (Variable, Buffered)
    0xC0     ,    //    End Collection
    0x09,0x66,    //    Usage Download Force Sample
    0xA1,0x02,    //    Collection Datalink
    0x85,0x08,         //    Report ID 8
    0x05,0x01,         //    Usage Page Generic Desktop
    0x09,0x30,         //    Usage X
    0x09,0x31,         //    Usage Y
    0x15,0x81,         //    Logical Minimum 81h (-127d)
    0x25,0x7F,         //    Logical Maximum 7Fh (127d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0xFF,0x00,    //    Physical Maximum FFh (255d)
    0x75,0x08,         //    Report Size 8
    0x95,0x02,         //    Report Count 2
    0x91,0x02,         //    Output (Variable)
    0xC0     ,   //    End Collection
    0x05,0x0F,   //    Usage Page Physical Interface
    0x09,0x77,   //    Usage Effect Operation Report
    0xA1,0x02,   //    Collection Datalink
    0x85,0x0A,    //    Report ID Ah (10d)
    0x09,0x22,    //    Usage Effect Block Index
    0x15,0x01,    //    Logical Minimum 1
    0x25,0x28,    //    Logical Maximum 28h (40d)
    0x35,0x01,    //    Physical Minimum 1
    0x45,0x28,    //    Physical Maximum 28h (40d)
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0x91,0x02,    //    Output (Variable)
    0x09,0x78,    //    Usage Operation
    0xA1,0x02,    //    Collection Datalink
    0x09,0x79,    //    Usage Op Effect Start
    0x09,0x7A,    //    Usage Op Effect Start Solo
    0x09,0x7B,    //    Usage Op Effect Stop
    0x15,0x01,    //    Logical Minimum 1
    0x25,0x03,    //    Logical Maximum 3
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0x91,0x00,    //    Output
    0xC0     ,         //    End Collection
    0x09,0x7C,         //    Usage Loop Count
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x00,    //    Logical Maximum FFh (255d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0xFF,0x00,    //    Physical Maximum FFh (255d)
    0x91,0x02,         //    Output (Variable)
    0xC0     ,    //    End Collection
    0x09,0x90,    //    Usage PID State Report (PID Block Free Report)
    0xA1,0x02,    //    Collection Datalink
    0x85,0x0B,    //    Report ID Bh (11d)
    0x09,0x22,    //    Usage Effect Block Index
    0x25,0x28,    //    Logical Maximum 28h (40d)
    0x15,0x01,    //    Logical Minimum 1
    0x35,0x01,    //    Physical Minimum 1
    0x45,0x28,    //    Physical Maximum 28h (40d)
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0x91,0x02,    //    Output (Variable)
    0xC0     ,    //    End Collection
    0x09,0x96,    //    Usage DC Disable Actuators (PID Device Control)
    0xA1,0x02,    //    Collection Datalink
    0x85,0x0C,    //    Report ID Ch (12d)
    0x09,0x97,    //    Usage DC Stop All Effects (DC Enable Actuators)
    0x09,0x98,    //    Usage DC Device Reset (DC Disable Actuators)
    0x09,0x99,    //    Usage DC Device Pause (DC Stop All Effects)
    0x09,0x9A,    //    Usage DC Device Continue (DC Device Reset?)
    0x09,0x9B,    //    Usage PID Device State (DC Device Pause)
    0x09,0x9C,    //    Usage DS Actuators Enabled
    0x15,0x01,    //    Logical Minimum 1
    0x25,0x06,    //    Logical Maximum 6
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0x91,0x00,    //    Output
    0xC0     ,    //    End Collection
    0x09,0x7D,    //    Usage PID Pool Report (Device Gain Report)
    0xA1,0x02,    //    Collection Datalink
    0x85,0x0D,         //    Report ID Dh (13d)
    0x09,0x7E,         //    Usage RAM Pool Size (Device Gain)
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x00,    //    Logical Maximum FFh (255d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0x10,0x27,    //    Physical Maximum 2710h (10000d)
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0xC0     ,            //    End Collection
    0x09,0x6B,    //    Usage Set Custom Force Report
    0xA1,0x02,    //    Collection Datalink
    0x85,0x0E,         //    Report ID Eh (14d)
    0x09,0x22,         //    Usage Effect Block Index
    0x15,0x01,         //    Logical Minimum 1
    0x25,0x28,         //    Logical Maximum 28h (40d)
    0x35,0x01,         //    Physical Minimum 1
    0x45,0x28,         //    Physical Maximum 28h (40d)
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x09,0x6D,         //    Usage Sample Count
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x00,    //    Logical Maximum FFh (255d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0xFF,0x00,    //    Physical Maximum FFh (255d)
    0x75,0x08,         //    Report Size 8
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x09,0x51,         //    Usage Sample Period
    0x66,0x03,0x10,    //    Unit 1003h (4099d)
    0x55,0xFD,         //    Unit Exponent FDh (253d)
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x7F,    //    Logical Maximum 7FFFh (32767d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0xFF,0x7F,    //    Physical Maximum 7FFFh (32767d)
    0x75,0x10,         //    Report Size 10h (16d)
    0x95,0x01,         //    Report Count 1
    0x91,0x02,         //    Output (Variable)
    0x55,0x00,         //    Unit Exponent 0
    0x66,0x00,0x00,    //    Unit 0
    0xC0     ,    //    End Collection
    0x09,0xAB,    //    Usage Undefined << Create New Effect Report
    0xA1,0x02,    //    Collection Datalink
    0x85,0x09,    //    Report ID 9
    0x09,0x25,    //    Usage Effect Type
    0xA1,0x02,    //    Collection Datalink
    0x09,0x26,    //    Usage ET Constant Force
    0x09,0x27,    //    Usage ET Ramp
    0x09,0x30,    //    Usage ET Square
    0x09,0x31,    //    Usage ET Sine
    0x09,0x32,    //    Usage ET Triangle
    0x09,0x33,    //    Usage ET Sawtooth Up
    0x09,0x34,    //    Usage ET Sawtooth Down
    0x09,0x40,    //    Usage ET Spring
    0x09,0x41,    //    Usage ET Damper
    0x09,0x42,    //    Usage ET Inertia
    0x09,0x43,    //    Usage ET Friction
    0x09,0x28,    //    Usage ET Custom Force Data
    0x25,0x0C,    //    Logical Maximum Ch (12d)
    0x15,0x01,    //    Logical Minimum 1
    0x35,0x01,    //    Physical Minimum 1
    0x45,0x0C,    //    Physical Maximum Ch (12d)
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0xB1,0x00,    //    Feature
    0xC0     ,    // End Collection
    0x05,0x01,         //    Usage Page Generic Desktop
    0x09,0x3B,         //    Usage Byte Count
    0x15,0x00,         //    Logical Minimum 0
    0x26,0xFF,0x01,    //    Logical Maximum 1FFh (511d)
    0x35,0x00,         //    Physical Minimum 0
    0x46,0xFF,0x01,    //    Physical Maximum 1FFh (511d)
    0x75,0x0A,         //    Report Size Ah (10d)
    0x95,0x01,         //    Report Count 1
    0xB1,0x02,         //    Feature (Variable)
    0x75,0x06,         //    Report Size 6
    0xB1,0x01,         //    Feature (Constant)
    0xC0     ,    //    End Collection
    0x05,0x0F,    //    Usage Page Physical Interface
    0x09,0x89,    //    Usage Block Load Status (PID Block Load Report)
    0xA1,0x02,    //    Collection Datalink
    0x85, 0x20,    //    Report ID 20h (32d)
    0x09,0x22,    //    Usage Effect Block Index
    0x25,0x28,    //    Logical Maximum 28h (40d)
    0x15,0x01,    //    Logical Minimum 1
    0x35,0x01,    //    Physical Minimum 1
    0x45,0x28,    //    Physical Maximum 28h (40d)
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0xB1,0x02,    //    Feature (Variable)
    0x09,0x8B,    //    Usage Block Load Full (Block Load Status)
    0xA1,0x02,    //    Collection Datalink
    0x09,0x8C,    //    Usage Block Load Error
    0x09,0x8D,    //    Usage Block Handle
    0x09,0x8E,    //    Usage PID Block Free Report
    0x25,0x03,    //    Logical Maximum 3
    0x15,0x01,    //    Logical Minimum 1
    0x35,0x01,    //    Physical Minimum 1
    0x45,0x03,    //    Physical Maximum 3
    0x75,0x08,    //    Report Size 8
    0x95,0x01,    //    Report Count 1
    0xB1,0x00,    //    Feature
    0xC0     ,                   // End Collection
    0x09,0xAC,                   //    Usage Undefined
    0x15,0x00,                   //    Logical Minimum 0
    0x27,0xFF,0xFF,0x00,0x00,    //    Logical Maximum FFFFh (65535d)
    0x35,0x00,                   //    Physical Minimum 0
    0x47,0xFF,0xFF,0x00,0x00,    //    Physical Maximum FFFFh (65535d)
    0x75,0x10,                   //    Report Size 10h (16d)
    0x95,0x01,                   //    Report Count 1
    0xB1,0x00,                   //    Feature
    0xC0     ,    //    End Collection
    0x09,0x7F,    //    Usage ROM Pool Size (PID Pool Report?)
    0xA1,0x02,    //    Collection Datalink
    0x85,0x03,                   //    Report ID 3
    0x09,0x80,                   //    Usage ROM Effect Block Count (RAM Pool Size?)
    0x75,0x10,                   //    Report Size 10h (16d)
    0x95,0x01,                   //    Report Count 1
    0x15,0x00,                   //    Logical Minimum 0
    0x35,0x00,                   //    Physical Minimum 0
    0x27,0xFF,0xFF,0x00,0x00,    //    Logical Maximum FFFFh (65535d)
    0x47,0xFF,0xFF,0x00,0x00,    //    Physical Maximum FFFFh (65535d)
    0xB1,0x02,                   //    Feature (Variable)
    0x09,0x83,                   //    Usage PID Pool Move Report (Simultaneous Effects Max?)
    0x26,0xFF,0x00,              //    Logical Maximum FFh (255d)
    0x46,0xFF,0x00,              //    Physical Maximum FFh (255d)
    0x75,0x08,                   //    Report Size 8
    0x95,0x01,                   //    Report Count 1
    0xB1,0x02,                   //    Feature (Variable)
    0x09,0xA9,                   //    Usage Undefined (Device Managed Pool?)
    0x09,0xAA,                   //    Usage Undefined (Shared Parameter Blocks?)
    0x75,0x01,                   //    Report Size 1
    0x95,0x02,                   //    Report Count 2
    0x15,0x00,                   //    Logical Minimum 0
    0x25,0x01,                   //    Logical Maximum 1
    0x35,0x00,                   //    Physical Minimum 0
    0x45,0x01,                   //    Physical Maximum 1
    0xB1,0x02,                   //    Feature (Variable)
    0x75,0x06,                   //    Report Size 6
    0x95,0x01,                   //    Report Count 1
    0xB1,0x03,                   //    Feature (Constant, Variable)
    0xC0,    //    End Collection

        // ====== End of virtual PID force feedback ======= //

    0xC0,               /*  End Collection,                     */

    0x05, 0x0C,         /*  Usage Page (Consumer),              */
    0x09, 0x01,         /*  Usage (Consumer Control),           */  // Virtual consumer control device
    0xA1, 0x01,         /*  Collection (Application),           */
    0x85, 0x1E,         /*      Report ID (30),                 */
    0x15, 0x00,         /*      Logical Minimum (0),            */
    0x25, 0x01,         /*      Logical Maximum (1),            */
    0x75, 0x03,         /*      Report Size (3),                */
    0x95, 0x01,         /*      Report Count (1),               */
    0x81, 0x03,         /*      Input (Constant, Variable),     */
    0x75, 0x01,         /*      Report Size (1),                */
    0x95, 0x02,         /*      Report Count (2),               */
    //0x09, 0xE2,         /*      Usage (Mute),                   */
    0x09, 0xE9,         /*      Usage (Volume Inc),             */
    0x09, 0xEA,         /*      Usage (Volume Dec),             */
    //0x09, 0x30,         /*      Usage (Power),                  */
    //0x0A, 0x24, 0x02,   /*      Usage (AC Back),                */
    //0x0A, 0x23, 0x02,   /*      Usage (AC Home),                */
    0x81, 0x02,         /*      Input (Variable),               */
    0x75, 0x03,         /*      Report Size (3),                */
    0x95, 0x01,         /*      Report Count (1),               */
    0x81, 0x03,         /*      Input (Constant, Variable),     */
    0xA1, 0x01,         /*      Collection (Application),       */
        0x19, 0x01,         /*          Usage Minimum (01h),        */ // HACK: Without this deliberately DirectInput-incompatible collection the customer control device would get detected as a gamepad, go figure..
        0x29, 0x03,         /*          Usage Maximum (03h),        */
        0x15, 0x00,         /*          Logical Minimum (0),        */
        0x26, 0xFF, 0xFF,   /*          Logical Maximum (65536),       */
        0x95, 0x03,         /*          Report Count (3),           */
        0x75, 0x10,         /*          Report Size (16),           */
        0x91, 0x02,         /*          Output (Variable),          */
        0xC0,               /*      End Collection,                 */
    0xC0,               /*  End Collection,                     */

    0x05, 0x01,         /*  Usage Page (Desktop),               */
    0x09, 0x02,         /*  Usage (Mouse),                      */
    0xA1, 0x01,         /*  Collection (Application),           */
    0x85, 0x02,         /*      Report ID (2),                  */
    0x09, 0x01,         /*      Usage (Pointer),                */
    0xA1, 0x00,         /*      Collection (Physical),          */
    0x05, 0x09,         /*          Usage Page (Button),        */
    0x19, 0x01,         /*          Usage Minimum (01h),        */
    0x29, 0x03,         /*          Usage Maximum (03h),        */
    0x25, 0x01,         /*          Logical Maximum (1),        */
    0x75, 0x01,         /*          Report Size (1),            */
    0x95, 0x03,         /*          Report Count (3),           */
    0x81, 0x02,         /*          Input (Variable),           */
    0x05, 0x09,         /*          Usage Page (Button),        */
    0x09, 0x05,         /*          Usage (05h),                */
    0x95, 0x01,         /*          Report Count (1),           */
    0x81, 0x02,         /*          Input (Variable),           */
    0x75, 0x04,         /*          Report Size (4),            */
    0x81, 0x01,         /*          Input (Constant),           */
    0x05, 0x01,         /*          Usage Page (Desktop),       */
    0x09, 0x30,         /*          Usage (X),                  */
    0x09, 0x31,         /*          Usage (Y),                  */
    0x15, 0x81,         /*          Logical Minimum (-127),     */
    0x25, 0x7F,         /*          Logical Maximum (127),      */
    0x35, 0x81,                   //    Physical Minimum -127
    0x45, 0x7F,                   //    Physical Maximum 127
    0x75, 0x10,         /*          Report Size (16),           */
    0x95, 0x02,         /*          Report Count (2),           */
    0x81, 0x06,         /*          Input (Variable, Relative), */
    0xC0,               /*      End Collection,                 */
    0xC0,               /*  End Collection,                     */
    0x06, 0xDE, 0xFF,   /*  Usage Page (FFDEh),                 */
    0x09, 0x01,         /*  Usage (01h),                        */
    0xA1, 0x01,         /*  Collection (Application),           */
    0x05, 0xFF,         /*      Usage Page (FFh),               */
    0x19, 0x01,         /*      Usage Minimum (01h),            */
    0x29, 0x40,         /*      Usage Maximum (40h),            */
    0x85, 0xFD,         /*      Report ID (253),                */
    0x15, 0x00,         /*      Logical Minimum (0),            */
    0x25, 0xFF,         /*      Logical Maximum (-1),           */
    0x95, 0x40,         /*      Report Count (64),              */
    0x75, 0x08,         /*      Report Size (8),                */
    0x81, 0x02,         /*      Input (Variable),               */
    0xC0,               /*  End Collection,                     */
    0x06, 0xDE, 0xFF,   /*  Usage Page (FFDEh),                 */
    0x09, 0x03,         /*  Usage (03h),                        */
    0xA1, 0x01,         /*  Collection (Application),           */
    0x19, 0x01,         /*      Usage Minimum (01h),            */
    0x29, 0x40,         /*      Usage Maximum (40h),            */
    0x85, 0xFC,         /*      Report ID (252),                */
    0x95, 0x40,         /*      Report Count (64),              */
    0x75, 0x08,         /*      Report Size (8),                */
    0xB1, 0x02,         /*      Feature (Variable),             */
    0xC0                /*  End Collection                      */
	
	
	

	
	
	
};

#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH sizeof(usbHidReportDescriptor)



static uchar    idleRate;           /* in 4 ms units */
char _awaitReport;

usbMsgLen_t usbFunctionSetup( uint8_t data [8] )
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