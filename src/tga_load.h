
#ifndef _TGA_LOAD_H
#define _TGA_LOAD_H

#include "image.h"

#define TGALOAD_OKAY 0
#define TGALOAD_CANTOPENFILE 1
#define TGALOAD_DAMAGEDFILE 2
#define TGALOAD_UNSUPPORTEDFILETYPE 3
#define TGALOAD_MEMORYALLOCERROR 4

extern int tgaLoad(char *fname, image_pc_type *image);

#endif
