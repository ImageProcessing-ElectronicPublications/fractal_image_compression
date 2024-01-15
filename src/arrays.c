
#include "arrays.h"

bool arrayReadBits(array_type *a, unsigned int size, unsigned int *out)
{
	unsigned int output, bits_offset, bytes_offset;
	unsigned char *bp;

	if(size > 32) return false;

	if((a->bytes_offset+(a->bits_offset+size)/8+((((a->bits_offset+size)%8)>0)?1:0)) > a->bytes_size)
		return false;

	bp = a->bp;
	bits_offset = a->bits_offset;
	bytes_offset = a->bytes_offset;
	output = 0;

	while(size > 0) {
		unsigned int this_step;

		if(size > (8-bits_offset))
			this_step = 8-bits_offset;
		else
			this_step = size;
		size -= this_step;
		bits_offset += this_step;

		output = (output << this_step)+((unsigned int)(*bp) >> (8-bits_offset))%(1<<this_step);
		
		if(bits_offset & 8) {
			bits_offset = 0;
			bytes_offset++;
			bp++;
		}
	}

	*out = output;

	a->bp = bp;
	a->bits_offset = bits_offset;
	a->bytes_offset = bytes_offset;

	return true;
}

bool arrayWriteBits(array_type *a, unsigned int size, unsigned int out)
{
	unsigned int bits_offset, bytes_offset;
	unsigned char *bp;

	if(size > 32) return false;

	if((a->bytes_offset+(a->bits_offset+size)/8+((((a->bits_offset+size)%8)>0)?1:0)) > a->bytes_size)
		return false;

	bp = a->bp;
	bits_offset = a->bits_offset;
	bytes_offset = a->bytes_offset;

	while(size > 0) {
		unsigned int this_step;

		if(size > (8-bits_offset))
			this_step = 8-bits_offset;
		else
			this_step = size;
		size -= this_step;
		bits_offset += this_step;

		*bp += ( (out>>size)%(1<<this_step) ) << (8-bits_offset);
		//output = (output << this_step)+((unsigned int)(*bp) >> (8-bits_offset))%(1<<this_step);
		
		if(bits_offset & 8) {
			bits_offset = 0;
			bytes_offset++;
			bp++;
		}
	}

	a->bp = bp;
	a->bits_offset = bits_offset;
	a->bytes_offset = bytes_offset;

	return true;
}