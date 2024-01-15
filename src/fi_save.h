
#ifndef _FI_SAVE_H
#define _FI_SAVE_H

#include "image.h"

#define FISAVE_OKAY 0
#define FISAVE_CANTOPENFILE 1
#define FISAVE_WRONGIMAGESIZE 2
#define FISAVE_DAMAGEDIMAGESTRUCT 3
#define FISAVE_MEMORYALLOCERROR 4

extern int fiSave(char *fname, image_uc_type *image, unsigned int blocksize, unsigned int nof_threads, unsigned int worst_diff);

#endif
