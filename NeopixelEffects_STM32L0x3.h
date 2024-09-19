/*
 *  Created on: Sep 2, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_PixelBanger
 *  Processor: STM32L053R8
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Header version: 1.0
 *  File: NeopixelEffects_STM32L0x3.h
 *  Change history: N/A
 */

#ifndef INC_NEOPIXELEFFECTS_STM32L0X3_H_
#define INC_NEOPIXELEFFECTS_STM32L0X3_H_

#include "stdint.h"
#include "ClockDriver_STM32L0x3.h"
#include "BitBang_STM32L0x3.h"

//LOCAL CONSTANT

//LOCAL VARIABLE
//below we define the starting values for the colour spin function
//the variables will hold the colour of the first pixel
//Note: the colour should be either green, red or blue. Due to the simple spin function, mixing will not work.
static uint8_t ColourSpin_green = 255;
static uint8_t ColourSpin_red = 0;
static uint8_t ColourSpin_blue = 0;

//EXTERNAL VARIABLE
extern uint8_t Bitbang_signal_source_matrix_32bit [][4];
extern uint8_t neopixel_activation_array[];

//FUNCTION PROTOTYPES
void NeopixelPublish(void);
void NeopixelRoll_32bit(uint8_t strip_length, uint16_t roll_speed_in_ms);
void NeopixelColourSpin_GRWB(uint8_t strip_length, uint8_t colour_pitch_active, uint16_t spin_speed_in_ms, uint8_t brightness_divider);
void NeopixelFillup_GRWB(uint8_t pixel_number, uint16_t fill_step_speed_in_ms);
void NeopixelDraindown_GRWB(uint8_t pixel_number, uint8_t strip_length, uint16_t drain_step_speed_in_ms);
void StripFill_GRWB(uint8_t strip_length, uint8_t green, uint8_t red, uint8_t blue, uint8_t white, uint16_t fill_speed_in_ms);
void StripDrain_GRWB(uint8_t strip_length, uint16_t drain_speed_in_ms);
void StripWipe_GRWB(uint8_t strip_length);

#endif /* INC_NEOPIXELEFFECTS_STM32L0X3_H_ */
