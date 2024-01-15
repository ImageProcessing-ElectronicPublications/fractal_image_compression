
#ifndef _IMAGE_MISC_H
#define _IMAGE_MISC_H

#include <stdbool.h>

#include "image.h"

extern void GetDxyBitsSize(unsigned int w, unsigned int h, unsigned int *dxy_bitssize);
extern unsigned int GetCblockBitsSize(unsigned int dxy_bitssize);
extern void SetBlocksPointers(image_rangeblock_type *blocks, unsigned int blocksize, unsigned int nof_blocks, unsigned int w, unsigned int h);
extern void Scale2to1(unsigned char *src, unsigned char *dst, unsigned int w, unsigned int h);
extern void ApplyReversedTransformToRangeBlock(unsigned char *src, unsigned char *dst, unsigned int tr, unsigned int blocksize);
extern bool UnpackChannels(image_pc_type *in, image_uc_type *out);
extern bool PackChannels(image_uc_type *in, image_pc_type *out);
extern void BGRtoYCBCR(image_uc_type *img);
extern void YCBCRtoBGR(image_uc_type *img);

#endif
