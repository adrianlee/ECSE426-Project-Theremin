#ifndef SINE_H
#define SINE_H

#include <stdint.h>
#include "stm32f4_discovery_audio_codec.h"

#define SINE_TABLE_SIZE 1024
#define SINE_AMPLITUDE 1000

void init_sine_generator(uint32_t sampling_freq, uint8_t volume);
void generate_sine(float freq);

#endif
