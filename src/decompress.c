
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fi_load.h"
#include "image_misc.h"
#include "tga_save.h"

int main(int argc, char **argv)
{
	char *input_filename, *output_filename;
	image_pc_type image_pc;
	image_uc_type image_uc;
	unsigned int i, scale;
	char *program_name;

	program_name = strrchr(argv[0], '\\');

	if(program_name == 0)
		program_name = strrchr(argv[0], '/');

	if(program_name == 0)
		program_name = argv[0];
	else
		program_name++;

	if(argc < 3) {
		printf("\t%s input.fi output.tga [scale=1]\n\n"
			"\t\tYou must specify input and output filenames\n", program_name);

		return 0;
	}
	input_filename = argv[1];
	output_filename = argv[2];
	if(argc > 3) {
		scale = abs(atoi(argv[3]));
		if(scale < 1)
			scale = 1;

		printf("Scale changed to %u\n", scale);
	} else
		scale = 1;

	switch(fiLoad(input_filename, &image_uc, scale)) {
		case FILOAD_OKAY:
			break;
		case FILOAD_CANTOPENFILE:
			printf("Can\'t open %s\n", input_filename);
			return 1;
		case FILOAD_DAMAGEDFILE:
			printf("Can\'t open %s, file is damaged\n", input_filename);
			return 1;
		case FILOAD_MEMORYALLOCERROR:
			printf("Can\'t open %s, memory allocation error\n", input_filename);
			return 1;
		default:
			printf("Undefined error while opening %s\n", input_filename);
			return 1;
	}

	YCBCRtoBGR(&image_uc);

	if(!PackChannels(&image_uc, &image_pc)) {
		printf("Can\'t unpack image channels\n");
		return 3;
	}

	for(i = 0; i < image_uc.nof_channels; i++)
		free(image_uc.data[i]);
	free(image_uc.data);

	switch(tgaSave(output_filename, &image_pc)) {
		case TGASAVE_OKAY:
			break;
		case TGASAVE_CANTOPENFILE:
			printf("Can\'t save %s\n", output_filename);
			return 4;
		case TGASAVE_DAMAGEDIMAGESTRUCT:
			printf("Can\'t save %s, internal error, damaged image structure\n", output_filename);
			return 4;
		case TGASAVE_TOOMANYCHANNELS:
			printf("Can\'t save %s, too many channels in input file\n", output_filename);
			return 4;
		default:
			printf("Undefined error while saving %s\n", output_filename);
			return 4;
	}

	free(image_pc.data);

	return 0;
}
