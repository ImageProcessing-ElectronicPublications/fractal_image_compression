
#ifndef _FI_SAVE_BLOCKS_COMPRESSION_H
#define _FI_SAVE_BLOCKS_COMPRESSION_H

typedef struct {
	unsigned char *data;
	unsigned int len;
} fi_compresseddata_type;

extern int fiCompressBlocksAndAddHeader(FI_HEADER *head ,fi_compresseddata_type *cdat, image_rangeblock_type *blocks, unsigned int nof_blocks);

#endif
