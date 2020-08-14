/**
 * DOGM163W_A_SERCOM1.h
 *
 * Created: 4/7/2020 1:27:27 PM
 * Author : Emmanuel Benard, Kaleb Croft
 */ 

#ifndef LCD_H_
#define LCD_H_

#include "saml21j18b.h"

//variable declaration
extern char dsp_buff1[17];
extern char dsp_buff2[17];
extern char dsp_buff3[17];

/**
 * Initializes LCD screen 
 */
extern void init_lcd_dog(void);

/**
 * Updates lcd screen using display buffers
 */
extern void update_lcd_dog(void);

#endif /* LCD_H_ */