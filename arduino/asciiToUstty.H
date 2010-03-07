#ifndef  __ASCIITOUSTTY_H__
#define  __ASCIITOUSTTY_H__

#include <avr/pgmspace.h>

#define FIGURES_CODE 0x11
#define LETTERS_CODE 0x12

// Return true if the character exists in the baudot
// character set
bool hasChar(char character);

// True for letters mode; false for figures mode
bool getMode(char character);

// 5-bit baudot code, MSB first
char getBaudot(char character);

#endif /* __ASCIITOUSTTY_H__ */
