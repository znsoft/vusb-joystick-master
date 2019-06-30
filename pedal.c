
#include <avr/io.h>
#include <avr/interrupt.h>
// target: ATmega328

#define YPIN1 ( 1<<PC5)


volatile int8_t adc; 

void init_Pedals(){

	startadc(YPIN1);



}


int startadc(uchar adctouse)
{
    

    ADMUX = adctouse;         // use #1 ADC
    ADMUX |= (1 << REFS0);    // use AVcc as the reference
    ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= (1 << ADEN);    // Enable the ADC
    ADCSRA |= _BV(ADIE);
    ADCSRA |= (1 << ADSC);    // Start the ADC conversion
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


int8_t adc_read(void)       //
{

  return adc;                   // 
}
