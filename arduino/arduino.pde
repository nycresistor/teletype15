#include "asciiToUstty.H"
#include <avr/interrupt.h>
#include <avr/io.h>

#define F_CPU 16000000L
#define RELAY_PIN 13

/**
 * Teletype15 controller code.
 *
 * The teletype protocol consists of:
 * 1 start bit (coil off)
 * 5 data bits
 * 1 stop bit (coil on)
 *
 * Pin assignments:
 * 13 -- coil on when high, off when low.
 */

// Define sending states
enum {
  SEND_READY,
  SEND_START,
  SEND_DATA,
  SEND_END
} sendingState = SEND_READY;

volatile byte sendingBit = 0;
volatile byte sendingByte = 0;
volatile byte cyclesLeftForBit = 0;

// Set up timer interrupt interval on clock 2 at 10.99 ms.
// 175824 ticks/interval
// @ 1/1024 prescaler: 172 cycles
void initTimer() {
  TCCR2A = 0b00000010; // No pin overrides, CTC mode
  TCCR2B = 0b00000111; // CTC mode, 1/1024 prescaler
  OCR2A = 172; // 172 * 1024 ticks per interrupt
  TIMSK2 = 0b00000010; // Interrupt on OCR2A match
}

boolean isReady() {
  return sendingState == SEND_READY;
}

void writeByte(byte b);

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  initTimer();
}

void loop() {
  if (isReady()) {
    writeByte('y');
    delay(2000);
  }
}

void setRelay(boolean value) {
  digitalWrite(RELAY_PIN, value);
}

void writeByte(byte b) {
  sendingByte = getBaudot(b);
  sendingState = SEND_START;
}
  
ISR(TIMER2_COMPA_vect) {
  if ( cyclesLeftForBit != 0 ) {
    // Wait until we're done sending the current bit.
    cyclesLeftForBit--;
    return;
  }
  // The sendingState is the state just initiated.
  if (sendingState == SEND_READY) {
    // Nothing to do.
  } else if (sendingState == SEND_START) {
    setRelay(false);
    sendingBit = 5;
    cyclesLeftForBit = 1;
    sendingState = SEND_DATA;
  } else if (sendingState == SEND_DATA) {
    sendingBit--;
    setRelay((sendingByte & _BV(sendingBit)) != 0);
    //setRelay((sendingBit % 2) == 0);
    cyclesLeftForBit = 1;
    if (sendingBit <= 0) { sendingState = SEND_END; }
  } else if (sendingState == SEND_END) {
    setRelay(false);
    cyclesLeftForBit = 2;
    sendingState = SEND_READY;
  }
}
