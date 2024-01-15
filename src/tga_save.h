
#ifndef _TGA_SAVE_H
#define _TGA_SAVE_H

#include "image.h"

#define TGASAVE_OKAY 0
#define TGASAVE_CANTOPENFILE 1
#define TGASAVE_DAMAGEDIMAGESTRUCT 2
#define TGASAVE_TOOMANYCHANNELS 3

extern int tgaSave(char *fname, image_pc_type *image);

#endif
