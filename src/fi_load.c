
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fi_file.h"
#include "fi_load.h"
#include "image_misc.h"
#include "arrays.h"

static size_t GetFileLength(FILE *f)
{
	size_t cur_pos, file_len;

	cur_pos = ftell(f);
	fseek(f, 0, SEEK_END);
	file_len = ftell(f);
	fseek(f, cur_pos, SEEK_SET);

	return file_len;
}

static bool fiDecompressBlocks(unsigned char *cb, unsigned int cb_size, image_rangeblock_type *b, unsigned int w, unsigned int h, unsigned int blocksize, unsigned int channels, unsigned int scale, unsigned int dxy_bitssize);
static unsigned int fiCoverRangeBlock(image_rangeblock_type *block_p, unsigned int blocksize, unsigned char *range_p, unsigned char *prev_data, unsigned int w, unsigned int h);

int fiLoad(char *fname, image_uc_type *image, unsigned int scale)
{
	FILE *f;
	FI_HEADER head;
	size_t file_len;
	unsigned int i, j, k, channels, alloc_memory, nof_blocks, blocksize, compressed_blocks_size;
	unsigned char *prev_data; // Данные об изображении на предыдущей итерации
	unsigned char *compressed_blocks;
	unsigned int dxy_bitssize, cblock_bitssize; // Размеры dx, dy и всего блока в битах
	image_rangeblock_type *blocks;

	if(scale < 1)
		scale = 1;

	f = fopen(fname, "rb");

	if(!f)
		return FILOAD_CANTOPENFILE;

	file_len = GetFileLength(f);

	if(fread(&head, sizeof(FI_HEADER), 1, f) != 1)
		return FILOAD_DAMAGEDFILE;

	if(head.sign != FI_SIGN)
		return FILOAD_DAMAGEDFILE;

	if(head.blocksize < 2)
		return FILOAD_DAMAGEDFILE;

	blocksize = head.blocksize;

	if((head.w%blocksize) != 0 || (head.h%blocksize) != 0 || head.w < (blocksize*2) || head.h < (blocksize*2)) {
		return FILOAD_DAMAGEDFILE;
	}

	GetDxyBitsSize(head.w, head.h, &dxy_bitssize);
	cblock_bitssize = GetCblockBitsSize(dxy_bitssize);

	image->w = head.w*scale;
	image->h = head.h*scale;
	image->nof_channels = channels = head.noc;
	image->alloc_memory = alloc_memory = image->w*image->h;
	blocksize *= scale;
	nof_blocks = image->w*image->h*image->nof_channels/(blocksize*blocksize);

	// Тут начинается код выделения памяти
	compressed_blocks = malloc((cblock_bitssize*image->w*image->h*image->nof_channels/4+nof_blocks*(blocksize*blocksize-1)/3)/8+1);
	printf("compressed_blocks = malloc(%u)\n", (cblock_bitssize*image->w*image->h*image->nof_channels/4+nof_blocks*(blocksize*blocksize-1)/3)/8+1);
	// Из рассчёта, что все блоки 2х2 и имеют размер cblock_bitssize бит
	// + добавляем дополнительные биты разбиения блоков
	// Каждый блок может быть разбит на 4. Т.е. для блока размера n (если n - степень двойки) у нас есть s=1+4+16+32+...+pow(4, log2(n)-1)
	// Т.е. s = 1*(1-pow(4,log2(n)))/(1-4) = (pow(2*2, log2(n))-1)/3 = (n*n-1)/3
	// Для n не в степени двойки (т.е. вида pow(2,x)*y) будем иметь s=1+4+16+32+...+pow(4, log2(n/y))
	// Т.е. s = 1*(1-pow(4,log2(n/y)))/(1-4) = (pow(2*2, log2(n/y))-1)/3 = ((n/y)*(n/y)-1)/3, что меньше чем (n*n-1)/3.
	// Таким образом, достаточно рассмотреть случай, когда n в степени двойки.
	// В нашем случае получаем nof_blocks*(blocksize*blocksize-1)/3 бит
	// + 1 байт на случай, если количество бит не кратно 8
	if(!compressed_blocks)
		return FILOAD_MEMORYALLOCERROR;

	// Немного кода для чтения блоков из файла
	compressed_blocks_size = file_len-sizeof(FI_HEADER);
	printf("compressed_blocks_size %u\n", compressed_blocks_size);
	if(fread(compressed_blocks, 1, compressed_blocks_size, f) != compressed_blocks_size) {
		free(compressed_blocks);

		return FILOAD_DAMAGEDFILE;
	}
	fclose(f);

	// И снова начинается код выделения памяти
	blocks = malloc(sizeof(image_rangeblock_type)*nof_blocks*(blocksize*blocksize-1)/3); // (blocksize*blocksize-1)/3 - количество разбиений одного блока в худшем случае
	printf("blocks = malloc(%u)\n", (unsigned int)(sizeof(image_rangeblock_type)*nof_blocks*(blocksize*blocksize-1)/3));
	if(!blocks) {
		free(compressed_blocks);
		return FILOAD_MEMORYALLOCERROR;
	}
	SetBlocksPointers(blocks, blocksize, nof_blocks, image->w, image->h);

	prev_data = malloc(alloc_memory/4);
	printf("prev_data = malloc(%u)\n", alloc_memory/4);
	if(!prev_data) {
		free(compressed_blocks);
		free(blocks);
		return FILOAD_MEMORYALLOCERROR;
	}

	image->data = malloc(channels*sizeof(void *));
	printf("image->data = malloc(%u)\n", (unsigned int)(channels*sizeof(void *)));
	if(!image->data) {
		free(compressed_blocks);
		free(blocks);
		free(prev_data);
		return FILOAD_MEMORYALLOCERROR;
	}

	for(i = 0; i < channels; i++) {
		image->data[i] = malloc(alloc_memory);
		printf("image->data[%u] = malloc(%u)\n",  i, alloc_memory);
		if(!image->data[i]) {
			unsigned int j;

			for(j = 0; j < i; j++)
				free(image->data[j]);
			free(image->data);
			free(compressed_blocks);
			free(blocks);
			free(prev_data);

			return FILOAD_MEMORYALLOCERROR;
		}
		memset(image->data[i], 0, alloc_memory);
	}
	// А тут он заканчивается

	if(!fiDecompressBlocks(compressed_blocks, compressed_blocks_size, blocks, head.w, head.h, head.blocksize, head.noc, scale, dxy_bitssize)) {
		free(compressed_blocks);
		free(blocks);
		free(prev_data);
		for(i = 0; i < channels; i++)
			free(image->data[i]);
		free(image->data);

		return FILOAD_DAMAGEDFILE;
	}
	free(compressed_blocks);

	for(i = 0; i < channels; i++) {
		unsigned char *data_p;
		unsigned int nof_iterations = 0;

		data_p = image->data[i];

		while(1) {
			unsigned long long diff = 0;
			image_rangeblock_type *block_p;

			block_p = blocks+i*nof_blocks/image->nof_channels;
			Scale2to1(data_p, prev_data, image->w, image->h);
			for(j = 0; j < image->h/blocksize; j++) {
				for(k = 0; k < image->w/blocksize; k++) {
					unsigned char *range_p;

					range_p = &data_p[j*blocksize*image->w+k*blocksize];

					diff += fiCoverRangeBlock(block_p, blocksize, range_p, prev_data, image->w, image->h);

					block_p++;
				}
			}

			diff /= image->w*image->h;
			printf("diff %llu\n", diff);
			nof_iterations++;
			if(nof_iterations >= 1024 || diff == 0)
				break;
		}
		printf("\nchannel %d completed after %d iterations\n", i+1, nof_iterations);
	}

	free(blocks);
	free(prev_data);

	return FILOAD_OKAY;
}

static unsigned int fiCoverRangeBlock(image_rangeblock_type *block_p, unsigned int blocksize, unsigned char *range_p, unsigned char *prev_data, unsigned int w, unsigned int h)
{
	unsigned int l, m;
	unsigned int diff = 0;

	//printf("%d", block_p->tr);
	if(block_p->is_divided) {
		//printf("Quadtrees unimplemented in decoder!11\n");
		//range_p = &data_p[j*blocksize*image->w+k*blocksize];
		//domain_p = &prev_data[block_p->dy*image->w/2+block_p->dx];

		diff += fiCoverRangeBlock((image_rangeblock_type *)block_p->divided_into, blocksize/2, range_p, prev_data, w, h);
		diff += fiCoverRangeBlock((image_rangeblock_type *)block_p->divided_into+1, blocksize/2, range_p+blocksize/2, prev_data, w, h);
		diff += fiCoverRangeBlock((image_rangeblock_type *)block_p->divided_into+2, blocksize/2, range_p+w*blocksize/2, prev_data, w, h);
		diff += fiCoverRangeBlock((image_rangeblock_type *)block_p->divided_into+3, blocksize/2, range_p+(w+1)*blocksize/2, prev_data, w, h);
	} else {
		unsigned char *domain_p;

		domain_p = &prev_data[block_p->dy*w/2+block_p->dx];

		switch(block_p->tr) {
			case IMAGE_RB_TRANSFORMATION_NONE:
				for(l = 0; l < blocksize; l++) {
					for(m = 0; m < blocksize; m++) {
						int loc_diff, res = (int)(*domain_p)*(int)block_p->u/32+(int)block_p->v;


						if(res < 0) res = 0;if(res > 255) res = 255;

						loc_diff = (int)(*range_p)-res;
						diff += loc_diff*loc_diff;

						*range_p = res;

						range_p++;
						domain_p++;
					}

					range_p += w-blocksize;
					domain_p += w/2-blocksize;
				}

				break;
			case IMAGE_RB_TRANSFORMATION_90RIGHT:
				for(l = 0; l < blocksize; l++) {
					for(m = 0; m < blocksize; m++) {
						int loc_diff, res = (int)(domain_p[(blocksize-1-m)*w/2+l])*(int)block_p->u/32+(int)block_p->v;

						if(res < 0) res = 0;if(res > 255) res = 255;

						loc_diff = (int)(*range_p)-res;
						diff += loc_diff*loc_diff;

						*range_p = res;

						range_p++;
					}

					range_p += w-blocksize;
				}

				break;
			case IMAGE_RB_TRANSFORMATION_180RIGHT:
				for(l = 0; l < blocksize; l++) {
					for(m = 0; m < blocksize; m++) {
						int loc_diff, res = (int)(domain_p[(blocksize-1-l)*w/2+blocksize-1-m])*(int)block_p->u/32+(int)block_p->v;

						if(res < 0) res = 0;if(res > 255) res = 255;

						loc_diff = (int)(*range_p)-res;
						diff += loc_diff*loc_diff;

						*range_p = res;

						range_p++;
					}

					range_p += w-blocksize;
				}

				break;
			case IMAGE_RB_TRANSFORMATION_270RIGHT:
				for(l = 0; l < blocksize; l++) {
					for(m = 0; m < blocksize; m++) {
						int loc_diff, res = (int)(domain_p[m*w/2+blocksize-1-l])*(int)block_p->u/32+(int)block_p->v;

						if(res < 0) res = 0;if(res > 255) res = 255;

						loc_diff = (int)(*range_p)-res;
						diff += loc_diff*loc_diff;

						*range_p = res;

						range_p++;
					}

					range_p += w-blocksize;
				}
				break;
			case IMAGE_RB_TRANSFORMATION_VFLIP:
				for(l = 0; l < blocksize; l++) {
					for(m = 0; m < blocksize; m++) {
						int loc_diff, res = (int)(domain_p[(blocksize-1-l)*w/2+m])*(int)block_p->u/32+(int)block_p->v;

						if(res < 0) res = 0;if(res > 255) res = 255;

						loc_diff = (int)(*range_p)-res;
						diff += loc_diff*loc_diff;

						*range_p = res;

						range_p++;
					}

					range_p += w-blocksize;
				}
				break;
			case IMAGE_RB_TRANSFORMATION_HFLIP:
				for(l = 0; l < blocksize; l++) {
					for(m = 0; m < blocksize; m++) {
						int loc_diff, res = (int)(domain_p[l*w/2+blocksize-1-m])*(int)block_p->u/32+(int)block_p->v;

						if(res < 0) res = 0;if(res > 255) res = 255;

						loc_diff = (int)(*range_p)-res;
						diff += loc_diff*loc_diff;

						*range_p = res;

						range_p++;
					}

					range_p += w-blocksize;
				}

				break;
			case IMAGE_RB_TRANSFORMATION_MAINDIAGFLIP:
				for(l = 0; l < blocksize; l++) {
					for(m = 0; m < blocksize; m++) {
						int loc_diff, res = (int)(domain_p[m*w/2+l])*(int)block_p->u/32+(int)block_p->v;

						if(res < 0) res = 0;if(res > 255) res = 255;

						loc_diff = (int)(*range_p)-res;
						diff += loc_diff*loc_diff;

						*range_p = res;

						range_p++;
					}

					range_p += w-blocksize;
				}

				break;
			case IMAGE_RB_TRANSFORMATION_ANTIDIAGFLIP:
				for(l = 0; l < blocksize; l++) {
					for(m = 0; m < blocksize; m++) {
						int loc_diff, res = (int)(domain_p[(blocksize-1-m)*w/2+blocksize-1-l])*(int)block_p->u/32+(int)block_p->v;

						if(res < 0) res = 0;if(res > 255) res = 255;

						loc_diff = (int)(*range_p)-res;
						diff += loc_diff*loc_diff;

						*range_p = res;

						range_p++;
					}

					range_p += w-blocksize;
				}

				break;
		}
	}

	return diff;
}

static bool fiDecompressOneBlock(array_type *cbarray, image_rangeblock_type *bp, unsigned int blocksize, unsigned int w, unsigned int h, unsigned int scale, unsigned int dxy_bitssize)
{
	unsigned int is_block_divided, dxy, tr, v, u;

	if(((blocksize % 2) != 1) && (blocksize > 3)) {
		if(!arrayReadBits(cbarray, 1, &is_block_divided))
			return false;
	} else
		is_block_divided = 0;

	if(is_block_divided) {
		if(!fiDecompressOneBlock(cbarray, (image_rangeblock_type *)bp->divided_into, blocksize/2, w, h, scale, dxy_bitssize))
			return false;

		if(!fiDecompressOneBlock(cbarray, (image_rangeblock_type *)bp->divided_into+1, blocksize/2, w, h, scale, dxy_bitssize))
			return false;

		if(!fiDecompressOneBlock(cbarray, (image_rangeblock_type *)bp->divided_into+2, blocksize/2, w, h, scale, dxy_bitssize))
			return false;

		if(!fiDecompressOneBlock(cbarray, (image_rangeblock_type *)bp->divided_into+3, blocksize/2, w, h, scale, dxy_bitssize))
			return false;

		bp->is_divided = true;
	} else {
		if(!arrayReadBits(cbarray, 5, &u))
			return false;
		bp->u = u;

		if(bp->u > 0) {
			if(!arrayReadBits(cbarray, 7, &v))
				return false;
			if(v & 64)
				bp->v = -1;
			else
				bp->v = 1;
			bp->v *= (int)v&63;
			bp->v = (bp->v*255)/63;

			if(!arrayReadBits(cbarray, dxy_bitssize, &dxy))
				return false;

			bp->dx = dxy%(w/2-1);
			if(bp->dx > w/2-blocksize)
				return false;
			bp->dx *= scale;

			bp->dy = dxy/(w/2-1);
			if(bp->dy > h/2-blocksize)
				return false;
			bp->dy *= scale;

			if(!arrayReadBits(cbarray, 3, &tr))
				return false;
			bp->tr = tr;
		} else {
			if(!arrayReadBits(cbarray, 6, &v))
				return false;
			bp->v = v&255;
			bp->v = (bp->v*255)/63;

			bp->dx = 0;
			bp->dy = 0;
			bp->tr = 0;
		}

		bp->is_divided = false;
	}

	return true;
}

static bool fiDecompressBlocks(unsigned char *cb, unsigned int cb_size, image_rangeblock_type *b, unsigned int w, unsigned int h, unsigned int blocksize, unsigned int channels, unsigned int scale, unsigned int dxy_bitssize)
{
	unsigned int i, j, k;
	image_rangeblock_type *bp;
	array_type cbarray;

	cbarray.bp = cbarray.bytes = cb;
	cbarray.bytes_offset = cbarray.bits_offset = 0;
	cbarray.bytes_size = cb_size;

	//memcpy(b, cb, nof_blocks*IMAGE_COMPRESSEDRB_SIZE);

	bp = b;
	for(i = 0; i < channels; i++) {
		for(j = 0; j < h/blocksize; j++) {
			for(k = 0; k < w/blocksize; k++) {
				if(!fiDecompressOneBlock(&cbarray, bp, blocksize, w, h, scale, dxy_bitssize))
					return false;
				bp++;
			}
		}
	}

	return true;
}
