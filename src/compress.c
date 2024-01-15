
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifdef I_USE_MPI
#include <mpi.h>
#define RETURN_FROM_MAIN(v) {MPI_Abort(MPI_COMM_WORLD,v);return(v);}
#else
#define RETURN_FROM_MAIN(v) {return(v);}
#endif

#include "tga_load.h"
#include "image_misc.h"
#include "fi_save.h"

int main(int argc, char **argv)
{
	char *input_filename, *output_filename;
	image_pc_type image_pc;
	image_uc_type image_uc;
	unsigned int i, blocksize, nof_threads, rms_error, worst_diff;
	char *program_name;
#ifdef I_USE_MPI
	int my_rank, nof_procs;
#endif

#ifdef I_USE_MPI
	MPI_Init(0, 0);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nof_procs);

	printf("Using MPI. Program rank is %d+1/%d\n", my_rank, nof_procs);
#endif

	program_name = strrchr(argv[0], '\\');

	if(program_name == 0)
		program_name = strrchr(argv[0], '/');

	if(program_name == 0)
		program_name = argv[0];
	else
		program_name++;

	// Чтение параметров командной строки
	if(argc < 3) {
		printf("\t%s input.tga output.fi [block_size=8 [nof_threads=4 [rms_error=256]]]\n\n"
			"\t\tYou must specify input and output filenames\n", program_name);

		RETURN_FROM_MAIN(0);
	}

	input_filename = argv[1];
	output_filename = argv[2];
	if(argc > 3) {
		blocksize = abs(atoi(argv[3]));
		if(blocksize < 2)
			blocksize = 2;
		if(blocksize > IMAGE_MAXBLOCKSIZE)
			blocksize = IMAGE_MAXBLOCKSIZE;

		printf("Default block size changed to %d\n", blocksize);
	} else
		blocksize = 8;

	if(argc > 4) {
		nof_threads = abs(atoi(argv[4]));
		if(nof_threads == 0) nof_threads = 1;
	} else {
#ifdef SINGLE_THREAD_BY_DEFAULT
		nof_threads = 1;
#else
		nof_threads = 4;
#endif
	}
	printf("Number of threads set to %d\n", nof_threads);

	if(argc > 5)
		rms_error = abs(atoi(argv[5]));
	else
		rms_error = 256;
	worst_diff = rms_error*rms_error*blocksize*blocksize;
	printf("RMS error %d, worst difference for first block size will be %d\n", rms_error, worst_diff);

	// Чтение исходного изображения
	switch(tgaLoad(input_filename, &image_pc)) {
		case TGALOAD_OKAY:
			break;
		case TGALOAD_CANTOPENFILE:
			printf("Can\'t open %s\n", input_filename);
			RETURN_FROM_MAIN(1);
		case TGALOAD_DAMAGEDFILE:
			printf("Can\'t open %s, file is damaged\n", input_filename);
			RETURN_FROM_MAIN(1);
		case TGALOAD_UNSUPPORTEDFILETYPE:
			printf("Can\'t open %s, unsupported file type\n", input_filename);
			RETURN_FROM_MAIN(1);
		case TGALOAD_MEMORYALLOCERROR:
			printf("Can\'t open %s, memory allocation error\n", input_filename);
			RETURN_FROM_MAIN(1);
		default:
			printf("Undefined error while opening %s\n", input_filename);
			RETURN_FROM_MAIN(1);
	}

	if((image_pc.w%blocksize) != 0 || (image_pc.h%blocksize) != 0 || image_pc.w < (blocksize*2) || image_pc.h < (blocksize*2)) {
		printf("Image sizes must be divisions of %d and greater than %d\n", blocksize, blocksize*2);
		RETURN_FROM_MAIN(2);
	}

	if(!UnpackChannels(&image_pc, &image_uc)) {
		printf("Can\'t unpack image channels\n");
		RETURN_FROM_MAIN(3);
	}

	free(image_pc.data);

	BGRtoYCBCR(&image_uc);

	// Сохранение результирующего изображения
	switch(fiSave(output_filename, &image_uc, blocksize, nof_threads, worst_diff)) {
		case FISAVE_OKAY:
			break;
		case FISAVE_CANTOPENFILE:
			printf("Can\'t save %s\n", output_filename);
			RETURN_FROM_MAIN(4);
		case FISAVE_WRONGIMAGESIZE:
			printf("Can\'t save %s, wrong image size\n", output_filename);
			RETURN_FROM_MAIN(4);
		case FISAVE_DAMAGEDIMAGESTRUCT:
			printf("Can\'t save %s, internal error, damaged image structure\n", output_filename);
			RETURN_FROM_MAIN(4);
		default:
			printf("Undefined error while saving %s\n", output_filename);
			RETURN_FROM_MAIN(4);
	}

	for(i = 0; i < image_uc.nof_channels; i++)
		free(image_uc.data[i]);
	free(image_uc.data);

#ifdef I_USE_MPI
	printf("===\n\trank %d finished\n===\n", my_rank);

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
#endif

	return 0;
}
