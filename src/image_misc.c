
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "image_misc.h"

void GetDxyBitsSize(unsigned int w, unsigned int h, unsigned int *dxy_bitssize)
{
	// Нам нужно такое значение, чтобы поместились цифры от 0 до (w/2-(2-1))*(h/2-(2-1))-1.
	// В (2-1) число 2 - это размер минимального рангового блока, для которого ищутся доменные блоки
	// Чтобы найти это значение, надо найти log2(((w/2-1)*(h/2-1)-1)*2)
	// Например, чтобы сохранить 4, нам нужно 3 бита, 3 = log2(4*2)
	*dxy_bitssize = (unsigned int)log2(((w/2-1)*(h/2-1)-1)*2);
}

unsigned int GetCblockBitsSize(unsigned int dxy_bitssize)
{
	unsigned int cblock_bitssize;

	//15 = 3+5+7; 3 - поворот, 5 - контраст, 7 - яркость
	cblock_bitssize = 15+dxy_bitssize;

	return cblock_bitssize;
}

void SetBlocksPointers(image_rangeblock_type *blocks, unsigned int blocksize, unsigned int nof_blocks, unsigned int w, unsigned int h)
{
	unsigned int i, j, k, offset;

	offset = 0;
	k = nof_blocks;
	for(i = blocksize; i > 2 && i%2 == 0; i /= 2) {
		for(j = 0; j < nof_blocks; j++) {
			blocks[offset+j].divided_into = blocks+k+j*4;
		}
		offset = k;
		nof_blocks *= 4;
		k += nof_blocks;
	}
}

void Scale2to1(unsigned char *src, unsigned char *dst, unsigned int w, unsigned int h)
{
	unsigned int i, j, half_w, half_h;

	half_w = w/2;
	half_h = h/2;
	
	for(i = 0; i < half_h; i++) {
		for(j = 0; j < half_w; j++) {
			dst[i*half_w+j] = ( (int)src[(i*2)*w+j*2]+(int)src[(i*2)*w+j*2+1]+(int)src[(i*2+1)*w+j*2]+(int)src[(i*2+1)*w+j*2+1] )/4;
		}
	}
}

void ApplyReversedTransformToRangeBlock(unsigned char *src, unsigned char *dst, unsigned int tr, unsigned int blocksize)
{
	unsigned int k, l;
	switch(tr) {
		case IMAGE_RB_TRANSFORMATION_NONE:
			memcpy(dst, src, blocksize*blocksize);

			break;
		case IMAGE_RB_TRANSFORMATION_90RIGHT: // Поворачиваем на 90 влево (т.к. обратная трансформация)
			for(k = 0; k < blocksize; k++)
				for(l = 0; l < blocksize; l++) {
					dst[k*blocksize+l] =  src[l*blocksize+blocksize-1-k];
				}

			break;
		case IMAGE_RB_TRANSFORMATION_180RIGHT: // Поворачиваем на 180 влево
			for(k = 0; k < blocksize; k++)
				for(l = 0; l < blocksize; l++) {
					dst[k*blocksize+l] =  src[(blocksize-1-k)*blocksize+blocksize-1-l];
				}

			break;
		case IMAGE_RB_TRANSFORMATION_270RIGHT:  // Поворачиваем на 270 влево
			for(k = 0; k < blocksize; k++)
				for(l = 0; l < blocksize; l++) {
					dst[k*blocksize+l] =  src[(blocksize-1-l)*blocksize+k];
				}

			break;
		case IMAGE_RB_TRANSFORMATION_VFLIP:
			for(k = 0; k < blocksize; k++)
				for(l = 0; l < blocksize; l++) {
					dst[k*blocksize+l] =  src[(blocksize-1-k)*blocksize+l];
				}

			break;
		case IMAGE_RB_TRANSFORMATION_HFLIP:
			for(k = 0; k < blocksize; k++)
				for(l = 0; l < blocksize; l++) {
					dst[k*blocksize+l] =  src[k*blocksize+blocksize-1-l];
				}

			break;
		case IMAGE_RB_TRANSFORMATION_MAINDIAGFLIP:
			for(k = 0; k < blocksize; k++)
				for(l = 0; l < blocksize; l++) {
					dst[k*blocksize+l] =  src[l*blocksize+k];
				}

			break;
		case IMAGE_RB_TRANSFORMATION_ANTIDIAGFLIP:
			for(k = 0; k < blocksize; k++)
				for(l = 0; l < blocksize; l++) {
					dst[k*blocksize+l] =  src[(blocksize-1-l)*blocksize+blocksize-1-k];
				}

			break;
	}
}

bool UnpackChannels(image_pc_type *in, image_uc_type *out)
{
	unsigned int i, channels, alloc_memory;

	out->w = in->w;
	out->h = in->h;
	out->nof_channels = channels = in->nof_channels;
	out->alloc_memory = alloc_memory = in->w*in->h;

	if(channels*alloc_memory != in->alloc_memory)
		return false;

	out->data = malloc(channels*sizeof(void *));
	if(!out->data)
		return false;

	for(i = 0; i < channels; i++) {
		out->data[i] = malloc(alloc_memory);
		if(!out->data[i]) {
			unsigned int j;
			
			for(j = 0; j < i; j++)
				free(out->data[j]);
			free(out->data);
			return false;
		}
	}

	for(i = 0; i < channels; i++) {
		unsigned char *p1, *p2;
		unsigned int j;

		p1 = in->data+i;
		p2 = out->data[i];

		for(j = 0; j < alloc_memory; j++) {
			*p2 = *p1;
			p1 += channels;
			p2++;
		}
	}

	return true;
}

bool PackChannels(image_uc_type *in, image_pc_type *out)
{
	unsigned int i, channels, alloc_memory, in_alloc_memory;

	out->w = in->w;
	out->h = in->h;
	out->nof_channels = channels = in->nof_channels;
	out->alloc_memory = alloc_memory = in->w*in->h*channels;
	in_alloc_memory = in->alloc_memory;

	if(alloc_memory != channels*in->alloc_memory)
		return false;

	out->data = malloc(alloc_memory);
	if(!out->data) return false;

	for(i = 0; i < channels; i++) {
		unsigned char *p1, *p2;
		unsigned int j;

		p1 = in->data[i];
		p2 = out->data+i;

		for(j = 0; j < in_alloc_memory; j++) {
			*p2 = *p1;
			p1++;
			p2 += channels;
		}
	}

	return true;
}

// http://www.w3.org/Graphics/JPEG/jfif3.pdf
void BGRtoYCBCR(image_uc_type *img)
{
	unsigned char *r, *g, *b;
	unsigned int i;

	if(img->nof_channels < 3)
		return;

	b = img->data[0];
	g = img->data[1];
	r = img->data[2];

	for(i = 0; i < img->w*img->h; i++) {
		double y, cb, cr;

		y = 0.299*(*r) + 0.587*(*g) + 0.114*(*b);
		cb = -0.1687*(*r) - 0.3313*(*g) + 0.5*(*b) + 128;
		cr = 0.5*(*r) - 0.4187*(*g) - 0.0813*(*b) + 128;

		if(y < 0.0)
			*b = 0;
		else if(y > 255.0)
			*b = 255;
		else
			*b = (unsigned char)(y+0.5);

		if(cb < 0.0)
			*g = 0;
		else if(cb > 255.0)
			*g = 255;
		else
			*g = (unsigned char)(cb+0.5);

		if(cr < 0.0)
			*r = 0;
		else if(cr > 255.0)
			*r = 255;
		else
			*r = (unsigned char)(cr+0.5);

		b++; g++; r++;
	}
}

// http://www.w3.org/Graphics/JPEG/jfif3.pdf
void YCBCRtoBGR(image_uc_type *img)
{
	unsigned char *y, *cb, *cr;
	unsigned int i;

	if(img->nof_channels < 3)
		return;

	y = img->data[0];
	cb = img->data[1];
	cr = img->data[2];

	for(i = 0; i < img->w*img->h; i++) {
		double b, g, r;

		b = (*y)+1.772*((int)(*cb)-128);
		g = (*y)-0.34414*((int)(*cb)-128)-0.71414*((int)(*cr)-128);
		r = (*y)+1.402*((int)(*cr)-128);

		if(b < 0.0)
			*y = 0;
		else if (b > 255.0)
			*y = 255;
		else
			*y = (unsigned char)(b+0.5);

		if(g < 0.0)
			*cb = 0;
		else if (g > 255.0)
			*cb = 255;
		else
			*cb = (unsigned char)(g+0.5);

		if(r < 0.0)
			*cr = 0;
		else if (r > 255.0)
			*cr = 255;
		else
			*cr = (unsigned char)(r+0.5);

		y++; cb++; cr++;
	}
}
