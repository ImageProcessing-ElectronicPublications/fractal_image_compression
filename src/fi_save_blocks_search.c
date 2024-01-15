
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
	#include <Windows.h>
	#include <process.h>
#else
	#include <pthread.h>
#endif

#ifdef I_USE_MPI
#include <mpi.h>
#endif

#include "image_misc.h"
#include "fi_save.h"
#include "fi_save_blocks_search.h"
#include "fi_save_blocks_search_workers.h"

int fiFindBestDomainBlocks(image_uc_type *image, unsigned int blocksize, unsigned int nof_threads, unsigned int worst_diff, fi_data_type *data)
{
	unsigned int i, nof_blocks;
	unsigned char *scaled_data; // Данные об изображении, уменьшенном в 2 раза (для поиска доменных блоков) (Данные идут последовательно для каждого из каналов)
	image_rangeblock_type *blocks;
	fi_worker_arg_type *worker_args;
#ifdef WIN32
	HANDLE *worker_thread_handles;
#else
	pthread_t *worker_thread_handles;
#endif
#ifdef I_USE_MPI
	int my_rank, nof_procs;
#endif

	if(blocksize < 2 || blocksize > IMAGE_MAXBLOCKSIZE) {
		return FISAVE_WRONGIMAGESIZE;
	}

	if((image->w%blocksize) != 0 || (image->h%blocksize) != 0 || image->w < (blocksize*2) || image->h < (blocksize*2)) {
		return FISAVE_WRONGIMAGESIZE;
	}

	if(image->w*image->h != image->alloc_memory)
		return FISAVE_DAMAGEDIMAGESTRUCT;

#ifdef I_USE_MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nof_procs);
#endif

	memset(data, 0, sizeof(fi_data_type));

	nof_blocks = image->w*image->h*image->nof_channels/(blocksize*blocksize);

	// Тут начинается код выделения памяти
	blocks = malloc(sizeof(image_rangeblock_type)*nof_blocks*(blocksize*blocksize-1)/3);
	printf("blocks = malloc(%u), sizeof(image_rangeblock_type)=%u\n", (unsigned int)(sizeof(image_rangeblock_type)*nof_blocks*(blocksize*blocksize-1)/3), (unsigned int)(sizeof(image_rangeblock_type)));
	// Каждый блок может быть разбит на 4. Т.е. для блока размера n (если n - степень двойки) у нас есть s=1+4+16+32+...+pow(4, log2(n)-1)
	// Т.е. s = 1*(1-pow(4,log2(n)))/(1-4) = (pow(2*2, log2(n))-1)/3 = (n*n-1)/3
	// Для n не в степени двойки (т.е. вида pow(2,x)*y) будем иметь s=1+4+16+32+...+pow(4, log2(n/y))
	// Т.е. s = 1*(1-pow(4,log2(n/y)))/(1-4) = (pow(2*2, log2(n/y))-1)/3 = ((n/y)*(n/y)-1)/3, что меньше чем (n*n-1)/3.
	// Таким образом, достаточно рассмотреть случай, когда n в степени двойки.
	// В нашем случае получаем s=(blocksize*blocksize-1)/3
	if(!blocks)
		return FISAVE_MEMORYALLOCERROR;
	memset(blocks, 0, sizeof(image_rangeblock_type)*nof_blocks*(blocksize*blocksize-1)/3);
	SetBlocksPointers(blocks, blocksize, nof_blocks, image->w, image->h);

	scaled_data = malloc(image->nof_channels*image->alloc_memory/4);
	printf("scaled_data = malloc(%u)\n", image->nof_channels*image->alloc_memory/4);
	if(!scaled_data) {
		free(blocks);
		return FISAVE_MEMORYALLOCERROR;
	}
	for(i = 0; i < image->nof_channels; i++)
		Scale2to1(image->data[i], &scaled_data[i*image->alloc_memory/4], image->w, image->h);

	worker_args = malloc(nof_threads*sizeof(fi_worker_arg_type));
	if(!worker_args) {
		free(scaled_data);
		free(blocks);
		return FISAVE_MEMORYALLOCERROR;
	}
#ifdef WIN32
	worker_thread_handles = malloc(nof_threads*sizeof(HANDLE));
#else
	worker_thread_handles = malloc(nof_threads*sizeof(pthread_t));
#endif
	if(!worker_thread_handles) {
		free(worker_args);
		free(scaled_data);
		free(blocks);
		return FISAVE_MEMORYALLOCERROR;
	}
	// А тут он заканчивается

	// Заполняем структуру 1-го "рабочего"
	worker_args[0].scaled_data = scaled_data;
	worker_args[0].data = image->data;
	worker_args[0].block_p = blocks;
	worker_args[0].nof_blocks_per_channel = nof_blocks/image->nof_channels;
	worker_args[0].w = image->w;
	worker_args[0].h = image->h;
	worker_args[0].nof_channels = image->nof_channels;
	worker_args[0].alloc_memory = image->alloc_memory;
	worker_args[0].blocksize = blocksize;
#ifdef I_USE_MPI
	worker_args[0].worker_start = nof_threads*my_rank;
	worker_args[0].worker_step = nof_threads*nof_procs;
#else
	worker_args[0].worker_start = 0;
	worker_args[0].worker_step = nof_threads;
#endif
	worker_args[0].worst_diff = worst_diff;
	for(i = 1; i < nof_threads; i++) { // Заполняем структуры остальных
		memcpy(worker_args+i, worker_args, sizeof(fi_worker_arg_type));
		worker_args[i].worker_start = worker_args[0].worker_start+i;
	}

#ifdef WIN32
	// Создаём рабочие потоки
	for(i = 0; i < nof_threads; i++) {

		worker_thread_handles[i] = (HANDLE)_beginthreadex(NULL, 0, fiWorkerFunc, worker_args+i, CREATE_SUSPENDED, NULL);//CreateThread(NULL, 0, fiWorkerFunc, worker_args+i, 0, NULL);

		if(!worker_thread_handles[i]) {
			unsigned int j;

			for(j = 0; j < i; j++) {
				CloseHandle(worker_thread_handles[j]);
			}

			free(worker_thread_handles);
			free(worker_args);
			free(scaled_data);
			free(blocks);
			return FISAVE_MEMORYALLOCERROR;
		}
		//fiWorkerFunc(worker_args+i);

	}

	// Вызываем рабочие потоки
	for(i = 0; i < nof_threads; i++)
		ResumeThread(worker_thread_handles[i]);
#else
	// Создаём рабочие потоки
	for(i = 0; i < nof_threads; i++)
		pthread_create(worker_thread_handles+i, NULL, fiWorkerFunc, worker_args+i);
#endif

#ifdef WIN32
	WaitForMultipleObjects(nof_threads, worker_thread_handles, TRUE, INFINITE);

	for(i = 0; i < nof_threads; i++)
		CloseHandle(worker_thread_handles[i]);
#else
	for(i = 0; i < nof_threads; i++)
		pthread_join(worker_thread_handles[i], 0);
#endif

	free(scaled_data);
	free(worker_args);
	free(worker_thread_handles);

	data->blocks = blocks;
	data->nof_blocks = nof_blocks;

	return FISAVE_OKAY;
}
