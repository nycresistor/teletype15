#include "asciiToTTY.h"

#define LOWER( v ) ( 0x00 | v )
#define UPPER( v ) ( 0x80 | v )
// These are the full seven-bit codes, MSB first.
// 0x00 indicates an unmapped character.
// The high bit is set for upper case; cleared for
// lower case.
// 0 = spacing impulse
// 1 = marking impulse

prog_uchar textCodes[26] = {
  LOWER( 0b11000 ), // A
  LOWER( 0b10011 ), // B
  LOWER( 0b01110 ), // C
  LOWER( 0b10010 ), // D
  LOWER( 0b10000 ), // E
  LOWER( 0b10110 ), // F
  LOWER( 0b01011 ), // G
  LOWER( 0b00101 ), // H
  LOWER( 0b01100 ), // I
  LOWER( 0b11010 ), // J
  LOWER( 0b11110 ), // K
  LOWER( 0b01001 ), // L
  LOWER( 0b00111 ), // M
  LOWER( 0b00110 ), // N
  LOWER( 0b00011 ), // O
  LOWER( 0b01101 ), // P
  LOWER( 0b11101 ), // Q
  LOWER( 0b01010 ), // R
  LOWER( 0b10100 ), // S
  LOWER( 0b00001 ), // T
  LOWER( 0b11100 ), // U
  LOWER( 0b01111 ), // V
  LOWER( 0b11001 ), // W
  LOWER( 0b10111 ), // X
  LOWER( 0b10101 ), // Y
  LOWER( 0b10001 )  // Z
};

prog_uchar numCodes[10] = {
  UPPER( 0b01101 ), // 0
  UPPER( 0b11101 ), // 1
  UPPER( 0b11001 ), // 2
  UPPER( 0b10000 ), // 3
  UPPER( 0b01010 ), // 4
  UPPER( 0b00001 ), // 5
  UPPER( 0b10101 ), // 6
  UPPER( 0b11100 ), // 7
  UPPER( 0b01100 ), // 8
  UPPER( 0b00011 )  // 9
};
