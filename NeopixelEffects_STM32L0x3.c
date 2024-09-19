/*
 *  Created on: Sep 2, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_PixelBanger
 *  Processor: STM32L053R8
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Program version: 1.0
 *  File: NeopixelEffects_STM32L0x3.c
 *  Change history: N/A
 */

#include "NeopixelEffects_STM32L0x3.h"

//1)
void NeopixelPublish(void){

	/*
	 * This function publishes the data matrix onto the neopixel strip using the bitband machine
	 *
	 */

      Bitbang_TIM2CH1DMA1CH5_Restart();													//we shut everything off at the end of the strip
	  while(Bitbang_signal_generated == 0);
	  Bitbang_signal_generated = 0;
	  Delay_us(80);															//we add a small delay for the signal to be generated past the two first rows and to have enough "off time"
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	//Note: not adding a delay leads to overrun where we publish the pixel matrix while changing it (occurs if we change matrix immediately after publishing it)
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	//		also, we may not generate enough off time otherwise

}

//2)
void NeopixelRoll_32bit(uint8_t strip_length, uint16_t roll_speed_in_ms){

	/*
	 * This function rolls the pixels.
	 * Note: it needs a starting matrix value which should be generated externally.
	 *
	 */

	  uint8_t temp [4] = {Bitbang_signal_source_matrix_32bit[0][0], Bitbang_signal_source_matrix_32bit[0][1], Bitbang_signal_source_matrix_32bit[0][2], Bitbang_signal_source_matrix_32bit[0][3]};

	  for(int i = 0; i < (strip_length - 1); i++){

		  for(int j = 0; j < 4; j++){

			  Bitbang_signal_source_matrix_32bit[i][j] = Bitbang_signal_source_matrix_32bit[i+1][j];						//we copy the i+1 pixel to the i place. This replaces all pixels, except pixel 8.

		  }

	  }

	  Bitbang_signal_source_matrix_32bit[(strip_length - 1)][0] = temp [0];											//then we copy the 1st pixel to the 8th one
	  Bitbang_signal_source_matrix_32bit[(strip_length - 1)][1] = temp [1];
	  Bitbang_signal_source_matrix_32bit[(strip_length - 1)][2] = temp [2];
	  Bitbang_signal_source_matrix_32bit[(strip_length - 1)][3] = temp [3];

	  Delay_ms(roll_speed_in_ms);

	  //we need to pre-load the first two values we will be publishing so the ping-pong buffer will be full with the new matrix first two rows
		Bitbang_signal_MSB_32bit(Bitbang_signal_source_matrix_32bit[0][0],Bitbang_signal_source_matrix_32bit[0][1],Bitbang_signal_source_matrix_32bit[0][2],Bitbang_signal_source_matrix_32bit[0][3]);
		Bitbang_signal_LSB_32bit(Bitbang_signal_source_matrix_32bit[1][0],Bitbang_signal_source_matrix_32bit[1][1],Bitbang_signal_source_matrix_32bit[1][2],Bitbang_signal_source_matrix_32bit[1][3]);
}

//3)
void NeopixelColourSpin_GRWB(uint8_t strip_length, uint8_t colour_pitch_active, uint16_t spin_speed_in_ms, uint8_t brightness_divider){

	/*
	 * This function spins the colours on the strip.
	 * A one-pixel pitch can be added to the output for additional effects.
	 *
	 */

	if(ColourSpin_green > 0 & (ColourSpin_blue == 0)){
		ColourSpin_green = ColourSpin_green - 1;
		ColourSpin_red = ColourSpin_red + 1;
	} else if (ColourSpin_red > 0 & (ColourSpin_green == 0)){

		ColourSpin_red = ColourSpin_red - 1;
		ColourSpin_blue = ColourSpin_blue + 1;

	} else if (ColourSpin_blue > 0 & (ColourSpin_red == 0)){

		ColourSpin_blue = ColourSpin_blue - 1;
		ColourSpin_green = ColourSpin_green + 1;

	} else {

		//do nothing

	}

	uint32_t colour_32bit = (ColourSpin_green << 16) | (ColourSpin_red << 8) | (ColourSpin_blue << 0);

	Bitbang_signal_source_matrix_32bit[0][0] = gamma_corr_8bit[(colour_32bit >> 16) & (uint32_t)0xFF] / brightness_divider;										//then we copy the 1st pixel to the 8th one
	Bitbang_signal_source_matrix_32bit[0][1] = gamma_corr_8bit[(colour_32bit >> 8) & (uint32_t)0xFF] / brightness_divider;
	Bitbang_signal_source_matrix_32bit[0][2] = gamma_corr_8bit[(colour_32bit >> 0) & (uint32_t)0xFF] / brightness_divider;
	Bitbang_signal_source_matrix_32bit[0][3] = 0;

	  for(int i = 1; i < strip_length; i++){

		  if(colour_pitch_active == 0){

			Bitbang_signal_source_matrix_32bit[i][0] = Bitbang_signal_source_matrix_32bit[i-1][0];										//then we copy the 1st pixel to the 8th one
			Bitbang_signal_source_matrix_32bit[i][1] = Bitbang_signal_source_matrix_32bit[i-1][1];
			Bitbang_signal_source_matrix_32bit[i][2] = Bitbang_signal_source_matrix_32bit[i-1][2];
			Bitbang_signal_source_matrix_32bit[i][3] = 0;

		  } else if (colour_pitch_active == 1){

			Bitbang_signal_source_matrix_32bit[i][0] = Bitbang_signal_source_matrix_32bit[i-1][1];										//then we copy the 1st pixel to the 8th one
			Bitbang_signal_source_matrix_32bit[i][1] = Bitbang_signal_source_matrix_32bit[i-1][2];
			Bitbang_signal_source_matrix_32bit[i][2] = Bitbang_signal_source_matrix_32bit[i-1][0];
			Bitbang_signal_source_matrix_32bit[i][3] = 0;

		  } else {

			  //do nothing

		  }

	  }

	Delay_us(((spin_speed_in_ms * 1000) / brightness_divider));

	//we need to pre-load the first two values we will be publishing so the ping-pong buffer will be full with the new matrix first two rows
	Bitbang_signal_MSB_32bit(Bitbang_signal_source_matrix_32bit[0][0],Bitbang_signal_source_matrix_32bit[0][1],Bitbang_signal_source_matrix_32bit[0][2],Bitbang_signal_source_matrix_32bit[0][3]);
	Bitbang_signal_LSB_32bit(Bitbang_signal_source_matrix_32bit[1][0],Bitbang_signal_source_matrix_32bit[1][1],Bitbang_signal_source_matrix_32bit[1][2],Bitbang_signal_source_matrix_32bit[1][3]);

}

//4)
void NeopixelFillup_GRWB(uint8_t pixel_number, uint16_t fill_step_speed_in_ms){

	/*
	 * This function fills up the strip from bottom to top until a designated pixel with the colour on the first pixel.
	 *
	 */

	  for(int i = 1; i <= (pixel_number - 1); i++){

		Bitbang_signal_source_matrix_32bit[i][0] = Bitbang_signal_source_matrix_32bit[i-1][0];										//then we copy the 1st pixel to the 8th one
		Bitbang_signal_source_matrix_32bit[i][1] = Bitbang_signal_source_matrix_32bit[i-1][1];
		Bitbang_signal_source_matrix_32bit[i][2] = Bitbang_signal_source_matrix_32bit[i-1][2];
		Bitbang_signal_source_matrix_32bit[i][3] = Bitbang_signal_source_matrix_32bit[i-1][3];

	  }

	Delay_ms(fill_step_speed_in_ms);

	//we need to pre-load the first two values we will be publishing so the ping-pong buffer will be full with the new matrix first two rows
	Bitbang_signal_MSB_32bit(Bitbang_signal_source_matrix_32bit[0][0],Bitbang_signal_source_matrix_32bit[0][1],Bitbang_signal_source_matrix_32bit[0][2],Bitbang_signal_source_matrix_32bit[0][3]);
	Bitbang_signal_LSB_32bit(Bitbang_signal_source_matrix_32bit[1][0],Bitbang_signal_source_matrix_32bit[1][1],Bitbang_signal_source_matrix_32bit[1][2],Bitbang_signal_source_matrix_32bit[1][3]);

}

//5)
void NeopixelDraindown_GRWB(uint8_t pixel_number, uint8_t strip_length, uint16_t drain_step_speed_in_ms){

	/*
	 * This function drains down the strip from top to bottom until a designated pixel using a blank colour.
	 *
	 */

	  for(int i = strip_length; i >= pixel_number; i--){

		Bitbang_signal_source_matrix_32bit[i][0] = 0;										//then we copy the 1st pixel to the 8th one
		Bitbang_signal_source_matrix_32bit[i][1] = 0;
		Bitbang_signal_source_matrix_32bit[i][2] = 0;
		Bitbang_signal_source_matrix_32bit[i][3] = 0;

	  }

	Delay_ms(drain_step_speed_in_ms);

	//we need to pre-load the first two values we will be publishing so the ping-pong buffer will be full with the new matrix first two rows
	Bitbang_signal_MSB_32bit(Bitbang_signal_source_matrix_32bit[0][0],Bitbang_signal_source_matrix_32bit[0][1],Bitbang_signal_source_matrix_32bit[0][2],Bitbang_signal_source_matrix_32bit[0][3]);
	Bitbang_signal_LSB_32bit(Bitbang_signal_source_matrix_32bit[1][0],Bitbang_signal_source_matrix_32bit[1][1],Bitbang_signal_source_matrix_32bit[1][2],Bitbang_signal_source_matrix_32bit[1][3]);

}

//6)
void StripWipe_GRWB(uint8_t strip_length){

	/*
	 * This function resets the strip.
	 *
	 */

	  for(int i = 0; i <= (strip_length - 1); i++){

		Bitbang_signal_source_matrix_32bit[i][0] = 0;										//then we copy the 1st pixel to the 8th one
		Bitbang_signal_source_matrix_32bit[i][1] = 0;
		Bitbang_signal_source_matrix_32bit[i][2] = 0;
		Bitbang_signal_source_matrix_32bit[i][3] = 0;

	  }

	//we need to pre-load the first two values we will be publishing so the ping-pong buffer will be full with the new matrix first two rows
	Bitbang_signal_MSB_32bit(Bitbang_signal_source_matrix_32bit[0][0],Bitbang_signal_source_matrix_32bit[0][1],Bitbang_signal_source_matrix_32bit[0][2],Bitbang_signal_source_matrix_32bit[0][3]);
	Bitbang_signal_LSB_32bit(Bitbang_signal_source_matrix_32bit[1][0],Bitbang_signal_source_matrix_32bit[1][1],Bitbang_signal_source_matrix_32bit[1][2],Bitbang_signal_source_matrix_32bit[1][3]);

}

//7)
void StripFill_GRWB(uint8_t strip_length, uint8_t green, uint8_t red, uint8_t blue, uint8_t white, uint16_t fill_speed_in_ms){

	  //fill up
	  Bitbang_signal_source_matrix_32bit [0][0] = green;											//then we copy the 1st pixel to the 8th one
	  Bitbang_signal_source_matrix_32bit [0][1] = red;
	  Bitbang_signal_source_matrix_32bit [0][2] = blue;
	  Bitbang_signal_source_matrix_32bit [0][3] = white;

	  Bitbang_signal_MSB_32bit(Bitbang_signal_source_matrix_32bit[0][0],Bitbang_signal_source_matrix_32bit[0][1],Bitbang_signal_source_matrix_32bit[0][2],Bitbang_signal_source_matrix_32bit[0][3]);
	  Bitbang_signal_LSB_32bit(Bitbang_signal_source_matrix_32bit[1][0],Bitbang_signal_source_matrix_32bit[1][1],Bitbang_signal_source_matrix_32bit[1][2],Bitbang_signal_source_matrix_32bit[1][3]);

	  NeopixelPublish();

	  for(uint8_t i = 2; i <= strip_length; i++){

		  NeopixelFillup_GRWB(i, fill_speed_in_ms);

		  NeopixelPublish();

	  };

	  Delay_ms(fill_speed_in_ms);

}

//8)
void StripDrain_GRWB(uint8_t strip_length, uint16_t drain_speed_in_ms){

	  //drain down
	  for(uint8_t i = (strip_length - 1); i > 0; i--){

		  NeopixelDraindown_GRWB(i, strip_length, drain_speed_in_ms);

		  NeopixelPublish();

	  };

	  Delay_ms(drain_speed_in_ms);

}
