/**
 * RS232_SERCOM4.c
 *
 * Created: 4/7/2020 12:45:56 PM
 * Author : Emmanuel Benard, Kaleb Croft
 */ 

#include "RS232_SERCOM4.h"
#include "saml21j18b.h"

//variable declarations for extra verification
unsigned char* ARRAY_PORT_PINCFG1 = (unsigned char*)&REG_PORT_PINCFG1;
unsigned char* ARRAY_PORT_PMUX1 = (unsigned char*)&REG_PORT_PMUX1;

//function prototypes for extra verification
void UART4_init(void);
void UART4_write(char data);
char UART4_read(void);


/**
 * initialize UART4 to transmit at 9600 Baud
 */
void UART4_init(void) {
    REG_GCLK_PCHCTRL22 = 0x00000040;  //SERCOM4 Core clock enabled

    REG_SERCOM4_USART_CTRLA |= 1;               /* reset SERCOM4 */
    while (REG_SERCOM4_USART_SYNCBUSY & 1) {}   /* wait for reset to complete */
    REG_SERCOM4_USART_CTRLA = 0x40106004;       /* LSB first, async, no parity,
        PAD[1]-Rx, PAD[0]-Tx, BAUD uses fraction, 8x oversampling, internal clock */
    REG_SERCOM4_USART_CTRLB = 0x00030000;    /* enable Tx, Rx, one stop bit, 8 bit */
    REG_SERCOM4_USART_BAUD  = 52;               /* 4000000 / 8 / 9600 = 52.02 */
    REG_SERCOM4_USART_CTRLA |= 2;               /* enable SERCOM4 */
    while (REG_SERCOM4_USART_SYNCBUSY & 2) {}   /* wait for enable to complete */

    ARRAY_PORT_PINCFG1[8] |= 1;     /* allow pmux to set PB08 pin configuration */
    ARRAY_PORT_PINCFG1[9] |= 1;     /* allow pmux to set PB09 pin configuration */
    ARRAY_PORT_PMUX1[4] = 0x33;     /* PB08 = TxD, PB09 = RxD */
}

/**
 * Send a data byte to UART4
 */
void UART4_write(char data) {
    while(!(REG_SERCOM4_USART_INTFLAG & 1)) {}  /* wait for data register empty */
    REG_SERCOM4_USART_DATA = data;              /* send a char */
}

/**
 * Read a data byte from UART4
 */
char UART4_read(void) {
    while(!(REG_SERCOM4_USART_INTFLAG & 4)) {}  /* wait until receive complete */
    return REG_SERCOM4_USART_DATA;       /* read the receive char and return it */
}