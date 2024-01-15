
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef I_USE_MPI
#include <mpi.h>
#endif

#include "arrays.h"
#include "fi_file.h"
#include "fi_save.h"
#include "fi_save_blocks_compression.h"
#include "fi_save_blocks_search.h"
#include "image_misc.h"

static int fiConvertImage(image_uc_type *image, fi_compresseddata_type *cdat, unsigned int blocksize, unsigned int nof_threads, unsigned int worst_diff);
static int fiSaveCompressedToFile(char *fname, fi_compresseddata_type *cdat);

int fiSave(char *fname, image_uc_type *image, unsigned int blocksize, unsigned int nof_threads, unsigned int worst_diff)
{
	int result;
	fi_compresseddata_type cdat;
	clock_t compression_time;
#ifdef I_USE_MPI
	int my_rank;
#endif

	compression_time = clock();
	result = fiConvertImage(image, &cdat, blocksize, nof_threads, worst_diff);
	compression_time = clock()-compression_time;
	printf("compression time %d ms\n", (int)compression_time);

	if(result != FISAVE_OKAY)
		return result;

#ifdef I_USE_MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	if(my_rank == 0)
		return fiSaveCompressedToFile(fname, &cdat);
	else
		return FISAVE_OKAY;
#else
	return fiSaveCompressedToFile(fname, &cdat);
#endif
}

static int fiConvertImage(image_uc_type *image, fi_compresseddata_type *cdat, unsigned int blocksize, unsigned int nof_threads, unsigned int worst_diff)
{
	FI_HEADER head;
	unsigned int result;
	fi_data_type data;
#ifdef I_USE_MPI
	int my_rank, nof_procs;
#endif

	result = fiFindBestDomainBlocks(image, blocksize, nof_threads, worst_diff, &data);
	if(result != FISAVE_OKAY)
		return result;

#ifdef I_USE_MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nof_procs);
	if(nof_procs > 1) {
		image_rangeblock_type *resulted_blocks = 0;

		if(my_rank == 0) {
			resulted_blocks = malloc(sizeof(image_rangeblock_type)*data.nof_blocks*(blocksize*blocksize-1)/3); // Почему такой размер - см. fi_save_blocks_search.c

			if(!resulted_blocks) {
				result = FISAVE_MEMORYALLOCERROR;
				goto EXIT_STATE;
			}
		}

		MPI_Barrier(MPI_COMM_WORLD);

		// Склейка блоков. Почему такой размер - см. fi_save_blocks_search.c
		MPI_Reduce(data.blocks, resulted_blocks, sizeof(image_rangeblock_type)*data.nof_blocks*(blocksize*blocksize-1)/3, MPI_BYTE, MPI_BOR, 0, MPI_COMM_WORLD);

		if(my_rank == 0) {
			free(data.blocks);
			data.blocks = resulted_blocks;

			// Пересчитываем указатели, так как при вызове MPI_Reduce
			SetBlocksPointers(data.blocks, blocksize, data.nof_blocks, image->w, image->h);
		}
	}

	if(my_rank > 0) {
		cdat->len = 0;
		result = FISAVE_OKAY;

		goto EXIT_STATE;
	}
#endif

	// Сжимаем полученные блоки для записи в файл
	head.sign = FI_SIGN;
	head.blocksize = blocksize;
	head.noc = image->nof_channels;
	head.w = image->w;
	head.h = image->h;
	result = fiCompressBlocksAndAddHeader(&head ,cdat, data.blocks, data.nof_blocks);

#ifdef I_USE_MPI
EXIT_STATE:
#endif

	free(data.blocks);

	return result;
}

static int fiSaveCompressedToFile(char *fname, fi_compresseddata_type *cdat)
{
	FILE *f;

	f = fopen(fname, "wb");
	if(!f) {
		free(cdat->data);
		return FISAVE_CANTOPENFILE;
	}

	fwrite(cdat->data, 1, cdat->len, f);

	fclose(f);
	free(cdat->data);
	cdat->len = 0;

	return FISAVE_OKAY;
}
