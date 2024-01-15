
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arrays.h"
#include "fi_file.h"
#include "fi_save.h"
#include "image_misc.h"

#include "fi_save_blocks_compression.h"

static bool fiCompressOneBlock(array_type *cbarray, image_rangeblock_type *b, unsigned int blocksize, unsigned int w, unsigned int h, unsigned int dxy_bitssize);

static unsigned int g_statistics_nofdivisions, g_statistics_nofblocks;

int fiCompressBlocksAndAddHeader(FI_HEADER *head ,fi_compresseddata_type *cdat, image_rangeblock_type *blocks, unsigned int nof_blocks)
{
	unsigned int i;
	unsigned int dxy_bitssize, cblock_bitssize; // Размеры dx, dy и всего блока в битах
	array_type cbarray;

	GetDxyBitsSize(head->w, head->h, &dxy_bitssize);
	cblock_bitssize = GetCblockBitsSize(dxy_bitssize);

	cbarray.bytes_offset = cbarray.bits_offset = 0;
	cbarray.bytes_size = (cblock_bitssize*head->w*head->h*head->noc/4+nof_blocks*(head->blocksize*head->blocksize-1)/3)/8+1;
	// Из рассчёта, что все блоки 2х2 и имеют размер cblock_bitssize бит
	// + добавляем дополнительные биты разбиения блоков (см. fi_save_blocks_search.c)
	// + 1 байт на случай, если количество бит не кратно 8

	cdat->data = malloc(sizeof(FI_HEADER)+cbarray.bytes_size);
	if(!cdat->data) {
		return FISAVE_MEMORYALLOCERROR;
	}
	memcpy(cdat->data, head, sizeof(FI_HEADER));
	
	cbarray.bp = cbarray.bytes = cdat->data+sizeof(FI_HEADER);
	memset(cbarray.bytes, 0, cbarray.bytes_size);

	g_statistics_nofdivisions = 0;
	g_statistics_nofblocks = 0;

	for(i = 0; i < nof_blocks; i++) {
		if(!fiCompressOneBlock(&cbarray, blocks+i, head->blocksize, head->w, head->h, dxy_bitssize)) {
			free(cdat->data);
			return FISAVE_MEMORYALLOCERROR;
		}
	}

	cdat->len = sizeof(FI_HEADER)+cbarray.bytes_offset+((cbarray.bits_offset>0)?1:0);

	printf("statistics:\n\t%d blocks ~ %d bytes\n", g_statistics_nofblocks, (g_statistics_nofblocks*(cblock_bitssize+1)+7)/8);
	printf("\t%d divisions - %d bytes\n", g_statistics_nofdivisions, (g_statistics_nofdivisions+7)/8);
	printf("\ttotal ~ %d bytes\n", (g_statistics_nofblocks*(cblock_bitssize+1)+g_statistics_nofdivisions+7)/8);
	printf("\tcbarray bytes_offset - %d bits_offset - %d\n", cbarray.bytes_offset, cbarray.bits_offset);
	printf("\tcdat array size - %d bytes\n", (int)cdat->len);

	return FISAVE_OKAY;
}

static bool fiCompressOneBlock(array_type *cbarray, image_rangeblock_type *b, unsigned int blocksize, unsigned int w, unsigned int h, unsigned int dxy_bitssize)
{
	if(b->is_divided) {
		if(!arrayWriteBits(cbarray, 1, 1))
			return false;

		if(!fiCompressOneBlock(cbarray, (image_rangeblock_type *)b->divided_into,  blocksize/2, w, h, dxy_bitssize))
			return false;
		if(!fiCompressOneBlock(cbarray, (image_rangeblock_type *)b->divided_into+1,  blocksize/2, w, h, dxy_bitssize))
			return false;
		if(!fiCompressOneBlock(cbarray, (image_rangeblock_type *)b->divided_into+2,  blocksize/2, w, h, dxy_bitssize))
			return false;
		if(!fiCompressOneBlock(cbarray, (image_rangeblock_type *)b->divided_into+3,  blocksize/2, w, h, dxy_bitssize))
			return false;

		g_statistics_nofdivisions++;

		return true;
	} else {
		if(((blocksize % 2) != 1) && (blocksize > 3)) {
			if(!arrayWriteBits(cbarray, 1, 0))
				return false;
		}

		if(!arrayWriteBits(cbarray, 5, (unsigned int)b->u))
			return false;

		if(b->u > 0) {
			unsigned int dxy;

			if(!arrayWriteBits(cbarray, 7, (unsigned int)((b->v<0)?64-b->v:b->v)))
				return false;

			dxy = (w/2-1)*((unsigned int)b->dy)+(unsigned int)b->dx;

			if(!arrayWriteBits(cbarray, dxy_bitssize, dxy))
				return false;

			if(!arrayWriteBits(cbarray, 3, (unsigned int)b->tr))
				return false;
		} else {
			if(!arrayWriteBits(cbarray, 6, (unsigned int)(b->v&63)))
				return false;
		}

		g_statistics_nofblocks++;

		return true;
	}
}
