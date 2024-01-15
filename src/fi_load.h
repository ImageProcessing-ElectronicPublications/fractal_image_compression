
#ifndef _FI_LOAD_H
#define _FI_LOAD_H

#include "image.h"

#define FILOAD_OKAY 0
#define FILOAD_CANTOPENFILE 1
#define FILOAD_DAMAGEDFILE 2
#define FILOAD_MEMORYALLOCERROR 3

extern int fiLoad(char *fname, image_uc_type *image, unsigned int scale);

#endif
