/**
 * DOGM163W_A_SERCOM1.c
 *
 * Created: 4/7/2020 1:52:32 PM
 *  Author: Emmanuel Benard, Kaleb Croft
 */ 

#include "saml21j18b.h"
#include "DOGM163W_A_SERCOM1.h"

#define FREQUENCY 4 //replace with frequency here//

 //Display buffer for DOG LCD
char dsp_buff1[17];
char dsp_buff2[17];
char dsp_buff3[17];

/**
 * Initializes SERCOM port for LCD screen
 */
void init_spi_lcd (void) {  //initalizes the SERCOM1 for command and data writes to LCD via SPI
	unsigned char* ARRAY_PORT_PINCFG0 = (unsigned char*)&REG_PORT_PINCFG0;
	unsigned char* ARRAY_PORT_PMUX0 = (unsigned char*)&REG_PORT_PMUX0;
	 
	//REG_MCLK_AHBMASK |= 0x00000004;	/* APBC bus clock enabled by default */
	//REG_MCLK_APBCMASK |= 0x00000002;	/* SERCOM1 APBC bus clock enabled by default */
	// Generic clock generator 0, enabled at reset @ 4MHz, is used for peripheral clock
	REG_GCLK_PCHCTRL19 = 0x00000040;	/* SERCOM1 core clock not enabled by default */

	for (int i = 16; i <= 19; i++) {
		ARRAY_PORT_PINCFG0[i] |= 1;    /* allow pmux to set PAi pin configuration */
	}
	 
	ARRAY_PORT_PMUX0[8] = 0x22;     /* PA16 = MOSI, PA17 = SCK */
	ARRAY_PORT_PMUX0[9] = 0x22;     /* PA18 = SS,   PA19 = MISO */

	REG_SERCOM1_SPI_CTRLA = 1;              /* reset SERCOM1 */
	while (REG_SERCOM1_SPI_CTRLA & 1) {}    /* wait for reset to complete */
	/* MISO-3, MOSI-0, SCK-1, SS-2, SPI master, CPOL = 1, CPOL = 1 */
	REG_SERCOM1_SPI_CTRLA = 0x3030000C;     
	REG_SERCOM1_SPI_CTRLB = 0x00002000;     /* Master SS, 8-bit */
	REG_SERCOM1_SPI_BAUD = 7;               /* SPI clock is 4MHz/(2 * (250 + 1)) = 1.6 MHz */
	REG_SERCOM1_SPI_CTRLA |= 2;             /* enable SERCOM1 */
} 

/*
 * Transmits a command byte to the LCD screen
 */
void lcd_spi_transmit_CMD (unsigned char cmd) {
	while (!(REG_SERCOM1_SPI_INTFLAG & 1)) {} //wait until Tx ready
	REG_PORT_OUTCLR1 = 0x40; //RS = 0 for command
	REG_SERCOM1_SPI_DATA = cmd; //send command
	while (!(REG_SERCOM1_SPI_INTFLAG&1)) {} //wait until Tx ready
}

/**
 * Transmits a data byte to the LCD screen
 */
 void lcd_spi_transmit_DATA (unsigned char data) {
	 while (!(REG_SERCOM1_SPI_INTFLAG & 1)) {} //wait until Tx ready
	 REG_PORT_OUTSET1 = 0x40; //RS = 1 for data
	 REG_SERCOM1_SPI_DATA = data; //send data
	 while (!(REG_SERCOM1_SPI_INTFLAG&1)) {} //wait until Tx ready
 }

/**
 * Creates a delay of 30 microseconds
 */
 void delay_30us(void) {
	 __asm("nop");
	 __asm("nop");
	 for (int i = 0; i<(15*FREQUENCY); i++)
	 __asm("nop");
 }

/**
 * This procedure will generate a variable delay for a fixed period of time
 * based on the passed value
 */
void var_delay(int delay_var) { //200 us times the var_delay ?
	delay_30us();
	int i;
	for (; delay_var > 0; delay_var--)
	for (i = 0; i < 50 * FREQUENCY; i++)
	__asm("nop");
}

/**
 * Creates a delay of 40 milliseconds
 */
void delay_40ms(void) {
	var_delay(40);
}

/**
 * Initializes LCD screen 
 */
void init_lcd_dog(void) {
	init_spi_lcd(); //init SERCOM port for DOG LCD
	delay_40ms();	//startup delay of 40 ms
	
	for (int i = 0; i < 2; i++) {   //start_bias
		lcd_spi_transmit_CMD(0x39);
		delay_30us();
	}
	lcd_spi_transmit_CMD(0x1E);	// Set bias value.
	delay_30us();
	lcd_spi_transmit_CMD(0x55);	// Power ctrl: 0x55 for 3.3V
	delay_30us();
	lcd_spi_transmit_CMD(0x6C);	// Follower mode on.
	delay_40ms();
	lcd_spi_transmit_CMD(0x7F); //contrast_set: 7F for 3.3V
	delay_30us();
	lcd_spi_transmit_CMD(0x0c); //display_on: cursor off, blink off
	delay_30us();
	lcd_spi_transmit_CMD(0x01); //clr_display: clear display, cursor home
	delay_30us();
	lcd_spi_transmit_CMD(0x06); //entry_mode: clear display, cursor home
	delay_30us();
}

/**
 * Updates lcd screen using display buffers
 */
void update_lcd_dog(void) {
		 
	init_lcd_dog();
		 
	lcd_spi_transmit_CMD(0x80);//init DDRAM addr-ctr
	delay_30us();
	for (int i =0; i<16; i++) { //send line 1 to LCD module
		lcd_spi_transmit_DATA(dsp_buff1[i]);
	};
	delay_30us();
		 
	lcd_spi_transmit_CMD(0x90);//init DDRAM addr-ctr
	delay_30us();
	for (int i =0; i<16; i++) { //send line 2 to LCD module
		lcd_spi_transmit_DATA(dsp_buff2[i]);
	};
	delay_30us();
		 
	lcd_spi_transmit_CMD(0xA0);//init DDRAM addr-ctr
	delay_30us();
	for (int i =0; i<16; i++) { //send line 3 to LCD module
		lcd_spi_transmit_DATA(dsp_buff3[i]);
	};
	delay_30us();
}