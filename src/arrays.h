
#ifndef _ARRAYS_H
#define _ARRAYS_H

#include <stdbool.h>

typedef struct {
	unsigned char *bytes;
	unsigned char *bp;
	unsigned int bytes_offset;
	unsigned int bits_offset;
	unsigned int bytes_size;
} array_type;

extern bool arrayReadBits(array_type *a, unsigned int size, unsigned int *out);
extern bool arrayWriteBits(array_type *a, unsigned int size, unsigned int out);

#endif
