
#ifndef _FI_SAVE_BLOCKS_SEARCH_H
#define _FI_SAVE_BLOCKS_SEARCH_H

#include "image.h"

typedef struct {
	image_rangeblock_type *blocks;
	unsigned int nof_blocks;
} fi_data_type;

extern int fiFindBestDomainBlocks(image_uc_type *image, unsigned int blocksize, unsigned int nof_threads, unsigned int worst_diff, fi_data_type *data);

#endif
