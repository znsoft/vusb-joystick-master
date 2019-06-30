#include <avr/io.h>
#include <avr/interrupt.h>

// target: ATmega328
// ------------------------------------------------------------------------
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
