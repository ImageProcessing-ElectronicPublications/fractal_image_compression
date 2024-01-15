
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tga_file.h"
#include "tga_save.h"

int tgaSave(char *fname, image_pc_type *image)
{
	FILE *f;
	TGAHEADER head;

	f = fopen(fname, "wb");
	if(!f)
		return TGASAVE_CANTOPENFILE;

	if(image->w*image->h*image->nof_channels != image->alloc_memory)
		return TGASAVE_DAMAGEDIMAGESTRUCT;

	if(image->nof_channels > 4 || image->nof_channels == 2 || image->nof_channels == 0)
		return TGASAVE_TOOMANYCHANNELS;

	memset(&head, 0, sizeof(TGAHEADER));

	head.TGAWidth = image->w;
	head.TGAHeight = image->h;
	if(image->nof_channels == 1)
		head.DataType = 3;
	else
		head.DataType = 2;
	head.BitPerPel = image->nof_channels*8;

	fwrite(&head, sizeof(head), 1, f);
	fwrite(image->data, 1, image->alloc_memory, f);

	fclose(f);

	return TGASAVE_OKAY;
}
