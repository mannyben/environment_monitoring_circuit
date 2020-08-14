/**
 * RS232_SERCOM4.h
 *
 * Created: 4/7/2020 1:26:38 PM
 *  Author: Emmanuel Benard, Kaleb Croft
 */ 

#ifndef RS232_H_
#define RS232_H_

#include "saml21j18b.h"

unsigned char* ARRAY_PORT_PINCFG1;
unsigned char* ARRAY_PORT_PMUX1;

/**
 * initialize UART4 to transmit at 9600 Baud
 */
void UART4_init(void);

/**
 * Send a data byte to UART4
 */
void UART4_write(char data);

/**
 * Read a data byte from UART4
 */
char UART4_read(void);

#endif /* RS232_H_ */