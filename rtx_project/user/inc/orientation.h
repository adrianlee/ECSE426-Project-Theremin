#ifndef ORIENTATION_H
#define ORIENTATION_H

#include <stdint.h>

typedef struct
{
	int8_t x;
	int8_t y;
	int8_t z;
} orientation;

typedef void (*orientation_callback)(const orientation* orient);

void init_orientation(orientation_callback callback);

#endif
