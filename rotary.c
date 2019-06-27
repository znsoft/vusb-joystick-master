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

// target: ATmega8
// ------------------------------------------------------------------------

// Port and pins for rotary encoder.
// If you change these change encode_init(), too!
#define PHASE_A     (PINC & 1<<PC1)
#define PHASE_B     (PINC & 1<<PC2)
#define KEY         (PINC & 1<<PC0)


volatile int8_t enc_delta;      // -128 ... 127
volatile uint8_t key_press;
static int8_t last;


void encode_init(void)
{
  int8_t new;

  // Make Phase A / B and key input and enable pull-ups
  DDRC &= ~(_BV(PC1)) & ~(_BV(PC2)) & ~(_BV(PC0));
  PORTC |= ~(_BV(PC1)) | ~(_BV(PC2)) | _BV(PC0);

  new = 0;
  if (PHASE_A)
    new = 3;
  if (PHASE_B)
    new ^= 1;                   // convert gray to binary
  last = new;                   // power on state
  enc_delta = 0;

  key_press = 0;

  TCCR2 = _BV(WGM21) | _BV(CS22);       // Timer 2: CTC, F_CPU / 64
  OCR2 = (uint8_t) (F_CPU / 64.0 * 1e-3 - 0.5);         // 1ms
  TIMSK |= _BV(OCIE2);
}

ISR(TIMER2_COMP_vect)           // 1ms for manual movement
{
  int8_t new, diff;
  static uint8_t key_state = 0;

  new = 0;
  if (PHASE_A)
    new = 3;
  if (PHASE_B)
    new ^= 1;                   // convert gray to binary
  diff = last - new;            // difference last - new
  if (diff & 1) {               // bit 0 = value (1)
    last = new;                 // store new as next last
    enc_delta += (diff & 2) - 1;        // bit 1 = direction (+/-)
  }

  // shift new value in (debouncing)
  key_state = (key_state << 1) | 0xe0;
  if (KEY)
    key_state |= 0x01;

  // if pressed four times (4 ms) it is be really pressed
  if (key_state == 0xf0)
    key_press = 1;
}

int8_t encode_readKey(void)       // read single step encoders
{

  return key_press;                   // counts since last call
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
