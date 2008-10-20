/**
 * Teletype15 controller code.
 *
 * The teletype protocol consists of:
 * 1 start bit (coil off)
 * 5 data bits
 * 1 stop bit (coil on)
 *
 * Pin assignments:
 * PB1 -- coil on when high, off when low.
 *
 * Serial on RXD, TXD.
 *
 * Initial code: test pattern.
 */

#include "asciiToTTY.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <string.h>
#define F_CPU 8000000  // 8 MHz

#define SET_OUTPUT(P,B) { DDR##P |= _BV(B); }
#define SET_INPUT(P,B) { DDR##P &= ~_BV(B); }

#define SET_PIN(P,B) { PORT##P |= _BV(B); }
#define CLR_PIN(P,B) { PORT##P &= ~_BV(B); }

#define HIGH() SET_PIN(B,1)
#define LOW()  CLR_PIN(B,1)

#define GET_PIN(P,B) ((PIN##P & _BV(B)) != 0)

// Number of milliseconds per bit
#define MS_PER_BIT 22

// We need a pulse every (MS_PER_BIT*F_CPU)/1000 cycles.
// We'll use a prescaler of 1/64 to bring this into the
// 16-bit range.
#define PRESCALE 64
#define SCALED_CYCLES_PER_BIT ((MS_PER_BIT*(F_CPU/1000)/PRESCALE))

#define BUFFER_SIZE 128
char textBuffer[BUFFER_SIZE];
int bufStart;
int bufEnd;
int bufCursor;
int bitSending;

void initBuffer() {
  bufStart = 0;
  bufEnd = 0;
  bufCursor = 0;
  bitSending = 0;
}

void initClock()
{
  // We need a pulse every (MS_PER_BIT*F_CPU)/1000 cycles.
  // We'll use a prescaler of 1/64 to bring this into the
  // 16-bit range.
  
  // Reset on OCR1A match.
  // WGM1: 0100  CS1: 011
  TCCR1A = 0x00;
  TCCR1B = 0x0b;
  //TCCR1B = 0x0d; //test
  TCCR1C = 0x00; // not using the force matches

  OCR1A = SCALED_CYCLES_PER_BIT;

  TIMSK1 = 0x02; // turn on interrupt
}

/*
 * Serial setup
 */
#define FLAG_PENDING_COMMAND 0x01
char flags = 0;

#define RX_BUF_SIZE 64
char rxBuf[RX_BUF_SIZE];
int rxOffset;

#define TX_BUF_SIZE 64
char txBuf[TX_BUF_SIZE];
char* pNextTx = NULL;

void putString( char* pStr ) {
  pNextTx = pStr;
  UCSR0B |= (1 << UDRIE0);
}

void initSerial() {
  // Setting to 9600 baud, 8N1.
  // 9600 baud @ 8MHz
  // U2X = 1, UBRR = 51
  UBRR0H = 0;
  UBRR0L = 18;
  UCSR0A = 0x02;
  // enable TX/RX
  UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);
  // 8 bits, no parity, 1 stop bit
  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
  rxOffset = 0;
}

void initPins()
{
  SET_OUTPUT(B,1);  // coil control
  HIGH();
}

void init()
{
  cli();
  initPins();
  initClock();
  initBuffer();
  initSerial();
  set_sleep_mode( SLEEP_MODE_IDLE );
  sei();
}

void processCommand()
{
  flags &= ~FLAG_PENDING_COMMAND;
  rxBuf[RX_BUF_SIZE-1] = '\0';
  char* pR = rxBuf;
  if ( *(pR++) != '+' ) return;  
}

int main( void )
{
  init();
  putString("Clock up.\n");
  while (1) {
    sleep_cpu();
    if ( (flags & FLAG_PENDING_COMMAND) != 0 ) {
      processCommand();
    }
  }
  return 0;
}

ISR(USART_UDRE_vect)
{
  if (pNextTx == 0 || *pNextTx == '\0') {
    UCSR0B &= ~(1 << UDRIE0);
  } else {
    UDR0=*pNextTx;
    pNextTx++;
  }
}

ISR(USART_RX_vect)
{  
  /*
  rxBuf[ rxOffset ] = UDR0;
  if ( rxBuf[ rxOffset ] == '\n' ) {
    rxBuf[ rxOffset ] = '\0';
    flags |= FLAG_PENDING_COMMAND;
    rxOffset = 0;
  } else {
    rxOffset = (rxOffset+1) % RX_BUF_SIZE;
  }
  */
  textBuffer[ bufEnd ] = UDR0;
  bufEnd = (bufEnd+1) % BUFFER_SIZE;
}

ISR(TIMER1_COMPA_vect)
{
  if (bufStart != bufEnd) {
    if ( bitSending == 0 ) {
      // send start bit
      LOW();
    } else if ( bitSending == 6 ) {
      // send stop bit
      HIGH();
      bufCursor = (bufCursor + 1) % BUFFER_SIZE;
    } else if ( bitSending < 6 ) {
      int bit = 5-(bitSending);
      unsigned char c = textBuffer[bufCursor];
      unsigned char code = 0;
      if ( c >= 'A' && c <= 'Z' ) {
	code = pgm_read_byte(textCodes + (c-'A') );
      } else if ( c >= 'a' && c <= 'z') {
	code = pgm_read_byte(textCodes + (c-'a') );
      } else if ( c >= '0' && c <= '9') {
	code = pgm_read_byte(numCodes  + (c-'0') );
      }
      if ( (code >> bit) & 0x01 ) {
	HIGH();
      } else {
	LOW();
      }
    }
    bitSending = (bitSending+1)%40;
  }
}
