
#ifndef _FI_SAVE_BLOCKS_SEARCH_WORKERS_H
#define _FI_SAVE_BLOCKS_SEARCH_WORKERS_H

#include "image.h"

typedef struct {
	unsigned char *scaled_data;
	unsigned char **data;
	image_rangeblock_type *block_p;
	unsigned int nof_blocks_per_channel;
	unsigned int w;
	unsigned int h;
	unsigned int nof_channels;
	unsigned int alloc_memory;
	unsigned int blocksize;
	unsigned int worker_start;
	unsigned int worker_step;
	unsigned int worst_diff;
} fi_worker_arg_type;

#ifdef WIN32
	extern unsigned int __stdcall fiWorkerFunc(void *arg);
#else
	extern void *fiWorkerFunc(void *arg);
#endif



#endif
