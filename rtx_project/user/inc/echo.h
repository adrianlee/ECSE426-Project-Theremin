#ifndef ECHO_H
#define ECHO_H
//Includes
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f4_discovery_audio_codec.h"

void addecho(uint16_t*, int, int, double);
void init_Echo_Filter(int, float);
int getRemainingSize(void);
void reset(void);
void filter(uint16_t*, int, int);
short getSample(uint16_t*, int);
void setSample(uint16_t*, int, short);

#endif
