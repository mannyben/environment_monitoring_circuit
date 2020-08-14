/**
 * main.c
 *
 * Created: 5/6/2020 12:01:25 PM
 * Author : Emmanuel Benard, Kaleb Croft
 */ 


#include "saml21j18b.h"
#include "bme680.h"
#include "RS232_SERCOM4.h"
#include "DOGM163W_A_SERCOM1.h"
#include <stdio.h>
#include <stdbool.h>

#define DUMMY_VAL 0x00

unsigned char* ARRAY_PORT_PINCFG0;
unsigned char* ARRAY_PORT_PMUX0;
unsigned char* ARRAY_PORT_PINCFG1;
unsigned char* ARRAY_PORT_PMUX1;

/**
 * Writes a '0' to PB07
 */
static void assert_CSB() {
	REG_PORT_OUTCLR1 |= 1 << 7;		//Write a '0' to PB07
}

/**
 * Writes a '1' to PB07
 */
static void unassert_CSB() {
	REG_PORT_OUTSET1 |= 1 << 7;
}

/**
 * Sends a byte of data to SERCOM1 SPI DATA register
 */
static uint8_t spi_transfer(uint8_t data) {
	//uint8_t rslt = 0x00;
	
	while (!(REG_SERCOM1_SPI_INTFLAG & 1)) {} // DRE (bit 0) ready
	REG_SERCOM1_SPI_DATA = data; //send data

	while (!(REG_SERCOM1_SPI_INTFLAG & 4)) {} //wait until Rx (bit 2) ready
	return REG_SERCOM1_SPI_DATA;
}

/**
 * Passes address and dereferenced dummy data pointer(s) (# of pointers depending on len) to spi transfer () 
 * Param dev_id and Return val unused
 */  	
int8_t user_spi_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	int8_t rslt = 0; /* Return 0 for Success, non-­zero for failure */
	
	reg_addr |= 0x80;						//setting rw bit to '1'
	assert_CSB();
	spi_transfer(reg_addr);
	for (uint16_t i = 0; i < len; i++) {
		*reg_data = spi_transfer(0x00);
		reg_data++;
	}
	unassert_CSB();
	
	return rslt;
}

/**
 * Passes address and dereferenced data pointer(s) (# of pointers depending on len) to spi transfer () 
 * Param dev_id and Return val unused
 */
int8_t user_spi_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	int8_t rslt = 0; /* Return 0 for Success, non­zero for failure */
	
	assert_CSB();
	spi_transfer(reg_addr);
	for (uint16_t i = 0; i < len; i++) {
		spi_transfer(reg_data[i]);
	}
	unassert_CSB();
	
	return rslt;
}

/*Variables to be used with BME R/W fxns*/
static uint8_t id = DUMMY_VAL;		//Chip ID of BME680, not used anymore for lab10
static const uint16_t length = 0x01; //In our program we will not be using multi-byte transfer so length can be a  const 1

/**
 * Initializing SERCOM1 and BME680 for SPI transfer
 */
static void init_spi_bme (void) {  //initializes the SERCOM1 for command and data writes to BME680 via SPI
	//REG_MCLK_AHBMASK |= 0x00000004;	/* APBC bus clock enabled by default */
	//REG_MCLK_APBCMASK |= 0x00000002;	/* SERCOM1 APBC bus clock enabled by default */
	// Generic clock generator 0, enabled at reset @ 4MHz, is used for peripheral clock
	REG_GCLK_PCHCTRL19 = 0x00000040;	/* SERCOM1 core clock not enabled by default */

	ARRAY_PORT_PINCFG0[16] |= 1;    /* allow pmux to set PA16 pin configuration */
	ARRAY_PORT_PINCFG0[17] |= 1;    /* allow pmux to set PA17 pin configuration */
	ARRAY_PORT_PINCFG0[18] |= 1;    /* allow pmux to set PA18 pin configuration */
	ARRAY_PORT_PINCFG0[19] |= 1;    /* allow pmux to set PA19 pin configuration */
	ARRAY_PORT_PINCFG1[7]  = 0;    /* setting PB07 as GPI0 */
	ARRAY_PORT_PMUX0[8] = 0x22;     /* PA16 = MOSI, PA17 = SCK */
	ARRAY_PORT_PMUX0[9] = 0x22;     /* PA18 = SS,   PA19 = MISO */
	//REG_PORT_DIRSET1 = 0x40;		//making PB06 (RS) O/P
	REG_PORT_DIRSET1 = 0x80;		//setting PB07 as O/P
	//REG_PORT_DIRSET1 = 0xC0;		//im testing if we need RS
	unassert_CSB();					//CSB idles at '1'

	REG_SERCOM1_SPI_CTRLA = 1;              /* reset SERCOM1 */
	while (REG_SERCOM1_SPI_CTRLA & 1) {}    /* wait for reset to complete */
	//SHOULD I TURN OF THE SS enable on CTRLB, What should i do about SS in CTRA
	REG_SERCOM1_SPI_CTRLA = 0x3030000C;     /* MISO-3, MOSI-0, SCK-1, SS-2, SPI master, CPOL = 1, CPOL = 1 */
	REG_SERCOM1_SPI_CTRLB = 0x00020000;     /* RXEN(receiver enable) asserted */
	//REG_SERCOM1_SPI_CTRLB = 0x00002000;     /* Master SS, 8-bit */
	REG_SERCOM1_SPI_BAUD = 0;               /* SPI clock is 2MHz */
	REG_SERCOM1_SPI_CTRLA |= 2;             /* enable SERCOM1 */
	
	/*Addresses and commands to send to BME680 for initialization*/
	uint8_t status;									//Stores values read from meas_status_0 (pg.36)
	uint8_t reset_addr_write = 0x60;				/*Address of reset register (spi_mem_page)*/
	uint8_t reset_cmd = 0xB6;
	uint8_t status_reg_addr = 0x73;					/*Address of reset status register*/
	//uint8_t chip_id_addr_read = 0x50;				/*Address of chip_id register and*/
	uint8_t status_page1_cmd = 0x10;				/*Setting spi_meme_page<4>, switches from page0 to page1*/
	uint8_t status_page0_cmd = 0x00;				/*Clearing spi_meme_page<4>, switches from page1 to page0*/
	
	user_spi_write(id, reset_addr_write, &reset_cmd, length);			/*software reset of BME680*/
	user_spi_write(id, status_reg_addr, &status_page0_cmd, length);	/*switch to page 0 of the memory map*/
	user_spi_read(id,status_reg_addr,&status,length);					/*read the BME680 status register*/
	//id = spi_read_BME680(chip_id_addr_read);						/*read the BME680 control register*/
	user_spi_write(id,status_reg_addr, &status_page1_cmd, length);	/*switch to page 1 of the memory map*/
	user_spi_read(id,status_reg_addr,&status,length);						/*read the BME680 status register*/
}

/**
 * Unselect both LCD and BME.
 * No data transfer taking place
 */
static void unselect_lcd_and_BME (void) {
	REG_SERCOM1_SPI_CTRLB = 0x00000000;     /* Turning off Master SS, 8-bit */
	unassert_CSB();					//sending GND to PB07
	//REG_PORT_DIRCLR1 = 0x80;		//set PB07 as I/P
}

/**
 * FXN to configure BME for SERCOM1
 */
static void reconfig_spi_bme(void) {
	REG_PORT_DIRSET1 = 0x80;		//setting PB07 back to O/P
	
	REG_SERCOM1_SPI_CTRLA = 1;              /* reset SERCOM1 */
	while (REG_SERCOM1_SPI_CTRLA & 1) {}    /* wait for reset to complete */
	//SHOULD I TURN OF THE SS enable on CTRLB, What should i do about SS in CTRA
	REG_SERCOM1_SPI_CTRLA = 0x3030000C;     /* MISO-3, MOSI-0, SCK-1, SS-2, SPI master, CPOL = 1, CPOL = 1 */
	REG_SERCOM1_SPI_CTRLB = 0x00020000;     /* RXEN(receiver enable) asserted */
	REG_SERCOM1_SPI_BAUD = 0;               /* SPI clock is 2MHz */
	REG_SERCOM1_SPI_CTRLA |= 2;             /* enable SERCOM1 */
	
	unassert_CSB();							// CSB idles at '1'
}

/**
 * Wait for a period amount of milliseconds
 */
void user_delay_ms(uint32_t period)
{
	uint32_t n = period;
	uint32_t i; 
	for (; n > 0; n--) 
	for (i = 0; i < 199; i++) 
	__asm("nop");
}

/**
 * Main function for task 2.
 * Reads temperature, pressure, humidity, and gas from the BME680.
 * Outputs the values to the LCD screen.
 */
int main(void)
{
	ARRAY_PORT_PINCFG0 = (unsigned char*)&REG_PORT_PINCFG0;
	ARRAY_PORT_PMUX0 = (unsigned char*)&REG_PORT_PMUX0;
	ARRAY_PORT_PINCFG1 = (unsigned char*)&REG_PORT_PINCFG0;
	ARRAY_PORT_PMUX1 = (unsigned char*)&REG_PORT_PMUX0;

    /* Initialize the SAM system */
    SystemInit();

	ARRAY_PORT_PINCFG1 = (unsigned char*)&REG_PORT_PINCFG1;
	ARRAY_PORT_PMUX1 = (unsigned char*)&REG_PORT_PMUX1;
	
	REG_PORT_DIRSET0 = 0x00070000;
	REG_PORT_DIRCLR0 = 0x00080000;
	REG_PORT_DIRSET1 = 0x000001C0;

	REG_PORT_OUTSET0 = 0x00040000;

	/*Init RS232*/
	UART4_init();
	
	/*Init LCD*/
	init_lcd_dog();
	
	/*Init BME680*/
	init_spi_bme();

	/*Unselect LCD and BME680*/
	unselect_lcd_and_BME();

	reconfig_spi_bme();
	
	/* SPI 4-Wire */
	
	/* Initializing the sensor */
    struct bme680_dev gas_sensor;
	
	int8_t rslt;
	
	uint8_t required_settings;
	
	uint16_t meas_period;
	
	struct bme680_field_data data;
	
	gas_sensor.dev_id = 0;
	gas_sensor.intf = BME680_SPI_INTF;
	gas_sensor.read = user_spi_read;
	gas_sensor.write = user_spi_write;
	gas_sensor.delay_ms = user_delay_ms;
	gas_sensor.amb_temp = 25;

	rslt = BME680_OK;
	rslt = bme680_init(&gas_sensor);

	/* Configuring the sensor in forced mode */

	/* Set the temperature, pressure and humidity settings */
	gas_sensor.tph_sett.os_hum = BME680_OS_2X;
	gas_sensor.tph_sett.os_pres = BME680_OS_4X;
	gas_sensor.tph_sett.os_temp = BME680_OS_8X;
	gas_sensor.tph_sett.filter = BME680_FILTER_SIZE_3;

	/* Set the remaining sensor settings and link the heating profile */ 
	gas_sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
	/* Create a ramp heat waveform in 3 steps */    
	gas_sensor.gas_sett.heatr_temp = 320; /* degree Celsius */    
	gas_sensor.gas_sett.heatr_dur = 150; /* milliseconds */

	/* Select the power mode */
	/* Must be set before writing the sensor configuration */    
	gas_sensor.power_mode = BME680_FORCED_MODE;

	/* Set the required sensor settings needed */
	required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL |
			BME680_FILTER_SEL | BME680_GAS_SENSOR_SEL;

	/* Set the desired sensor configuration */
	rslt = bme680_set_sensor_settings(required_settings, &gas_sensor);

	/* Set the power mode */
	rslt = bme680_set_sensor_mode(&gas_sensor);

	/* Reading all sensor data */

	/* Get the total measurement duration so as to sleep or wait till the 
	 * measurement is complete */
	bme680_get_profile_dur(&meas_period, &gas_sensor);

    /* Replace with your application code */
    while (1) 
    {
		reconfig_spi_bme();				/*Config BME680 for SERCOM1*/
		
		user_delay_ms(meas_period);	/* Delay till the measurement is ready */
		
		rslt = bme680_get_sensor_data(&data, &gas_sensor);
		
		printf("T: %.2f degC, P: %.2f hPa, H %.2f %%rH ",
		data.temperature / 100.0f, data.pressure / 100.0f, data.humidity / 1000.0f);
		
		bool heating_setup_stable = false;

		/* Avoid using measurements from an unstable heating setup */
		if (data.status & BME680_GASM_VALID_MSK) {
			printf(", G: %d ohms", data.gas_resistance);
			heating_setup_stable = true;
		}

		printf("\r\n");
		
		unassert_CSB();
	
		/* Trigger the next measurement to read data out continuously */
		if (gas_sensor.power_mode == BME680_FORCED_MODE) {
			rslt = bme680_set_sensor_mode(&gas_sensor);
		}
		
		/* Print data to LCD */

		REG_PORT_OUTCLR0 = 0x00040000;

		init_lcd_dog();			/*Config LCD for SERCOM1*/

		sprintf(dsp_buff1, "T: %.2f degC", data.temperature / 100.0f);
		sprintf(dsp_buff2, "P: %.2f hPa", data.pressure / 100.0f);

		update_lcd_dog();

		user_delay_ms(1000);

		sprintf(dsp_buff1, "H: %.2f %%rH", data.humidity / 1000.0f);

		/* Avoid using measurements from an unstable heating setup */
		if (heating_setup_stable) {
			sprintf(dsp_buff2, "G: %d ohms", data.gas_resistance);
		}

		update_lcd_dog();

		user_delay_ms(1000);

		REG_PORT_OUTSET0 = 0x00040000;
    }
}

#include "sys_support.h"