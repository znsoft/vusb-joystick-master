/**************************************************************************
 * Reading rotary encoder
 * one, two and four step encoders are supported
 *
 * Author: Peter Dannegger
 * License: Creative Commons Attribution-ShareAlike 2.0
 *
 * Modified for ATmega8 and encoder with push key by Markus Dolze
 *************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#define USE_XPOS 1
// target: ATmega328
// ------------------------------------------------------------------------

// Port and pins for rotary encoder.
// If you change these change encode_init(), too!
#define PIN_A     (_BV(1))
#define PIN_B     (_BV(0))
#define PHASE_A     (PINB & PIN_A)
#define PHASE_B     (PINB & PIN_B)


volatile int8_t enc_delta;      // -128 ... 127

static int8_t last;


void encode_init(void)
{
  int8_t new;

  // Make Phase A / B  input 
  DDRB &= ~(PIN_A) & ~(PIN_B) ;
  PORTB &= ~(PIN_A) & ~(PIN_B) ;

  new = 0;
  if (PHASE_A)
    new = 3;
  if (PHASE_B)
    new ^= 1;                   // convert gray to binary
  last = new;                   // power on state
  
  enc_delta = 0;

//#ifdef USE_XPOS

    PCICR |= (1 << PCIE0);    // set PCIE0 to enable PCMSK0 scan PortB
    PCMSK0 |= (1 << PIN_A)|(1 << PIN_B);  // set PCINT to trigger an interrupt on state change 
//#endif

// Set the Timer Mode to CTC
  //  TCCR0A |= (1 << WGM01);

    // Set the value that you want to count to
  //  OCR0A = 0xF9;

    //TIMSK0 |= (1 << OCIE0A);    //Set the ISR COMPA vect


   // TCCR0B |= (1 << CS01);
    // set prescaler to 8 and start the timer


}

ISR (PCINT0_vect)
//#ifdef USE_XPOS
// 0,1,2,3
//ISR (TIMER0_COMPA_vect)
{
   int8_t new, diff=0;
  
 new = 0;
  if (PHASE_A)    new = 1;
  if (PHASE_B)    new |= 2;
  

if(last == 0&&new==1)diff = 1;
if(last == 0&&new==2)diff =-1;
if(last == 3&&new==1)diff =-1;
if(last == 3&&new==2)diff =1;

enc_delta += diff;
    last = new;                 // store new as next last
return;
                 // convert gray to binary
  diff = last - new;            // difference last - new
  if (diff & 1) {               // bit 0 = value (1)
    last = new;                 // store new as next last
    enc_delta += (diff & 2) - 1;        // bit 1 = direction (+/-)
  
	//enc_delta = 1;
  }
}
//#endif


 void calcEncode()
{
  int8_t new, diff;
 
 new = 0;
  if (PHASE_A)    new = 3;
  if (PHASE_B)    new ^= 1;                   // convert gray to binary
  diff = last - new;            // difference last - new
  if (diff != 0) {               // bit 0 = value (1)
    last = new;                 // store new as next last
    enc_delta += (diff & 2) - 1;        // bit 1 = direction (+/-)
	enc_delta = 1;
  }
//enc_delta = new;

}

int8_t encode_read1(void)       // read single step encoders
{
  int8_t val;

  cli();
  val = enc_delta;
  enc_delta = 0;
  sei();
  return val;                   // counts since last call
}


int8_t encode_read2(void)       // read two step encoders
{
  int8_t val;

  cli();
  val = enc_delta;
  enc_delta = val & 1;
  sei();
  return val >> 1;
}


int8_t encode_read4(void)       // read four step encoders
{
  int8_t val;

  cli();
  val = enc_delta;
  enc_delta = val & 3;
  sei();
  return val >> 2;
}
