
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

extern "C" {
#include "../image_misc.h"
#include "../fi_save_blocks_search_workers.h"
}

#define IMAGE_MAXCUDABLOCKSIZE 64

static void ShowMeSomeCUDAInfo(void);

typedef struct {
	unsigned int dx; // Смещение по x доменного блока
	unsigned int dy; // Смещение по у доменного блока
	unsigned int tr; // Трансформация
	unsigned int u; // Контрастность
	int v; // Яркость
	unsigned int diff;
} kernel_block_type;

__device__ void fiCalcReversedTransformationNone(long long &range_sum, long long &domain_sum, long long &rd_sum, long long &domain_disp, int *range_block,
	unsigned int dx, unsigned int dy,
	unsigned int rx, unsigned int ry, unsigned int blocksize,
	unsigned int w, unsigned int h,
	int *scaled_data_cuda,
	int *data_cuda
	)
{
	unsigned int l, m;

	rd_sum = domain_disp = domain_sum = range_sum = 0;

	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = data_cuda[(ry+l)*w+rx+m];
			int bd = scaled_data_cuda[(dy+l)*w/2+dx+m];

			range_block[l*blocksize+m] = br;

			range_sum += range_block[l*blocksize+m];

			domain_sum += bd;
			domain_disp += bd*bd;
			rd_sum += br*bd;
		}
	}

	domain_disp = domain_disp*(int)(blocksize*blocksize)-domain_sum*domain_sum;
}

__device__ void fiCalcReversedTransformation90Right(long long &range_sum, long long &domain_sum, long long &rd_sum, long long &domain_disp, int *range_block,
	unsigned int dx, unsigned int dy,
	unsigned int rx, unsigned int ry, unsigned int blocksize,
	unsigned int w, unsigned int h,
	int *scaled_data_cuda,
	int *data_cuda
	)
{
	unsigned int l, m;

	rd_sum = domain_disp = domain_sum = range_sum = 0;

	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = data_cuda[(ry+m)*w+rx+blocksize-1-l];
			int bd = scaled_data_cuda[(dy+l)*w/2+dx+m];

			range_block[l*blocksize+m] = br;

			range_sum += range_block[l*blocksize+m];

			domain_sum += bd;
			domain_disp += bd*bd;
			rd_sum += br*bd;
		}
	}

	domain_disp = domain_disp*(int)(blocksize*blocksize)-domain_sum*domain_sum;
}

__device__ void fiCalcReversedTransformation180Right(long long &range_sum, long long &domain_sum, long long &rd_sum, long long &domain_disp, int *range_block,
	unsigned int dx, unsigned int dy,
	unsigned int rx, unsigned int ry, unsigned int blocksize,
	unsigned int w, unsigned int h,
	int *scaled_data_cuda,
	int *data_cuda
	)
{
	unsigned int l, m;

	rd_sum = domain_disp = domain_sum = range_sum = 0;

	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = data_cuda[(ry+blocksize-1-l)*w+rx+blocksize-1-m];
			int bd = scaled_data_cuda[(dy+l)*w/2+dx+m];

			range_block[l*blocksize+m] = br;

			range_sum += range_block[l*blocksize+m];

			domain_sum += bd;
			domain_disp += bd*bd;
			rd_sum += br*bd;
		}
	}

	domain_disp = domain_disp*(int)(blocksize*blocksize)-domain_sum*domain_sum;
}

__device__ void fiCalcReversedTransformation270Right(long long &range_sum, long long &domain_sum, long long &rd_sum, long long &domain_disp, int *range_block,
	unsigned int dx, unsigned int dy,
	unsigned int rx, unsigned int ry, unsigned int blocksize,
	unsigned int w, unsigned int h,
	int *scaled_data_cuda,
	int *data_cuda
	)
{
	unsigned int l, m;

	rd_sum = domain_disp = domain_sum = range_sum = 0;

	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = data_cuda[(ry+blocksize-1-m)*w+rx+l];
			int bd = scaled_data_cuda[(dy+l)*w/2+dx+m];

			range_block[l*blocksize+m] = br;

			range_sum += range_block[l*blocksize+m];

			domain_sum += bd;
			domain_disp += bd*bd;
			rd_sum += br*bd;
		}
	}

	domain_disp = domain_disp*(int)(blocksize*blocksize)-domain_sum*domain_sum;
}

__device__ void fiCalcReversedTransformationVFlip(long long &range_sum, long long &domain_sum, long long &rd_sum, long long &domain_disp, int *range_block,
	unsigned int dx, unsigned int dy,
	unsigned int rx, unsigned int ry, unsigned int blocksize,
	unsigned int w, unsigned int h,
	int *scaled_data_cuda,
	int *data_cuda
	)
{
	unsigned int l, m;

	rd_sum = domain_disp = domain_sum = range_sum = 0;

	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = data_cuda[(ry+blocksize-1-l)*w+rx+m];
			int bd = scaled_data_cuda[(dy+l)*w/2+dx+m];

			range_block[l*blocksize+m] = br;

			range_sum += range_block[l*blocksize+m];

			domain_sum += bd;
			domain_disp += bd*bd;
			rd_sum += br*bd;
		}
	}

	domain_disp = domain_disp*(int)(blocksize*blocksize)-domain_sum*domain_sum;
}

__device__ void fiCalcReversedTransformationHFlip(long long &range_sum, long long &domain_sum, long long &rd_sum, long long &domain_disp, int *range_block,
	unsigned int dx, unsigned int dy,
	unsigned int rx, unsigned int ry, unsigned int blocksize,
	unsigned int w, unsigned int h,
	int *scaled_data_cuda,
	int *data_cuda
	)
{
	unsigned int l, m;

	rd_sum = domain_disp = domain_sum = range_sum = 0;

	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = data_cuda[(ry+l)*w+rx+blocksize-1-m];
			int bd = scaled_data_cuda[(dy+l)*w/2+dx+m];

			range_block[l*blocksize+m] = br;

			range_sum += range_block[l*blocksize+m];

			domain_sum += bd;
			domain_disp += bd*bd;
			rd_sum += br*bd;
		}
	}

	domain_disp = domain_disp*(int)(blocksize*blocksize)-domain_sum*domain_sum;
}

__device__ void fiCalcReversedTransformationMainDiagFlip(long long &range_sum, long long &domain_sum, long long &rd_sum, long long &domain_disp, int *range_block,
	unsigned int dx, unsigned int dy,
	unsigned int rx, unsigned int ry, unsigned int blocksize,
	unsigned int w, unsigned int h,
	int *scaled_data_cuda,
	int *data_cuda
	)
{
	unsigned int l, m;

	rd_sum = domain_disp = domain_sum = range_sum = 0;

	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = data_cuda[(ry+m)*w+rx+l];
			int bd = scaled_data_cuda[(dy+l)*w/2+dx+m];

			range_block[l*blocksize+m] = br;

			range_sum += range_block[l*blocksize+m];

			domain_sum += bd;
			domain_disp += bd*bd;
			rd_sum += br*bd;
		}
	}

	domain_disp = domain_disp*(int)(blocksize*blocksize)-domain_sum*domain_sum;
}

__device__ void fiCalcReversedTransformationAntiDiagFlip(long long &range_sum, long long &domain_sum, long long &rd_sum, long long &domain_disp, int *range_block,
	unsigned int dx, unsigned int dy,
	unsigned int rx, unsigned int ry, unsigned int blocksize,
	unsigned int w, unsigned int h,
	int *scaled_data_cuda,
	int *data_cuda
	)
{
	unsigned int l, m;

	rd_sum = domain_disp = domain_sum = range_sum = 0;

	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = data_cuda[(ry+blocksize-1-m)*w+rx+blocksize-1-l];
			int bd = scaled_data_cuda[(dy+l)*w/2+dx+m];

			range_block[l*blocksize+m] = br;

			range_sum += range_block[l*blocksize+m];

			domain_sum += bd;
			domain_disp += bd*bd;
			rd_sum += br*bd;
		}
	}

	domain_disp = domain_disp*(int)(blocksize*blocksize)-domain_sum*domain_sum;
}

__global__ void fiFindBestDomainBlockKernel(unsigned int rx, unsigned int ry,
	kernel_block_type *kernel_blocks_cuda, unsigned int blocksize,
	unsigned int w, unsigned int h,
	int *scaled_data_cuda,
	int *data_cuda
	)
{
	/*__shared__ */int range_block[IMAGE_MAXCUDABLOCKSIZE*IMAGE_MAXCUDABLOCKSIZE];
	long long range_sum, domain_sum, rd_sum; // Сумма пикселей рангового блока, доменного блока, пикселя рангового на пиксель доменного блока
	long long domain_disp;
	unsigned int diff;
	int u; int v;
	unsigned int mono_diff;
	int mono_u; int mono_v;
	unsigned int local_diff;
	__shared__ unsigned int best_diff;
	int local_u; int local_v;
	unsigned int dx, dy; // Координаты доменного блока
	unsigned int tr; // Трансформация, вычисляемая в текущем потоке
	kernel_block_type *kernel_block_cuda; // Текущий доменный блок для текущего блока cuda

	unsigned int l, m;

	atomicExch(&best_diff, 255*255*blocksize*blocksize*(blockDim.x*blockDim.y)+(blockDim.x*blockDim.y));
	__syncthreads();

	kernel_block_cuda = kernel_blocks_cuda+blockIdx.z*gridDim.y*gridDim.x+blockIdx.y*gridDim.x+blockIdx.x;

	dx = (blockIdx.x*blockDim.x+threadIdx.x)%(w/2-blocksize+1);
	dy = (blockIdx.y*blockDim.y+threadIdx.y)%(h/2-blocksize+1);
	tr = blockIdx.z+threadIdx.z;

	switch(tr) {
		case IMAGE_RB_TRANSFORMATION_NONE:
			fiCalcReversedTransformationNone(range_sum, domain_sum, rd_sum, domain_disp, range_block,
				dx, dy,
				rx, ry, blocksize,
				w, h,
				scaled_data_cuda,
				data_cuda
				);
			break;
		case IMAGE_RB_TRANSFORMATION_90RIGHT:
			fiCalcReversedTransformation90Right(range_sum, domain_sum, rd_sum, domain_disp, range_block,
				dx, dy,
				rx, ry, blocksize,
				w, h,
				scaled_data_cuda,
				data_cuda
				);
			break;
		case IMAGE_RB_TRANSFORMATION_180RIGHT:
			fiCalcReversedTransformation180Right(range_sum, domain_sum, rd_sum, domain_disp, range_block,
				dx, dy,
				rx, ry, blocksize,
				w, h,
				scaled_data_cuda,
				data_cuda
				);
			break;
		case IMAGE_RB_TRANSFORMATION_270RIGHT:
			fiCalcReversedTransformation270Right(range_sum, domain_sum, rd_sum, domain_disp, range_block,
				dx, dy,
				rx, ry, blocksize,
				w, h,
				scaled_data_cuda,
				data_cuda
				);
			break;
		case IMAGE_RB_TRANSFORMATION_VFLIP:
			fiCalcReversedTransformationVFlip(range_sum, domain_sum, rd_sum, domain_disp, range_block,
				dx, dy,
				rx, ry, blocksize,
				w, h,
				scaled_data_cuda,
				data_cuda
				);
			break;
		case IMAGE_RB_TRANSFORMATION_HFLIP:
			fiCalcReversedTransformationHFlip(range_sum, domain_sum, rd_sum, domain_disp, range_block,
				dx, dy,
				rx, ry, blocksize,
				w, h,
				scaled_data_cuda,
				data_cuda
				);
			break;
		case IMAGE_RB_TRANSFORMATION_MAINDIAGFLIP:
			fiCalcReversedTransformationMainDiagFlip(range_sum, domain_sum, rd_sum, domain_disp, range_block,
				dx, dy,
				rx, ry, blocksize,
				w, h,
				scaled_data_cuda,
				data_cuda
				);
			break;
		case IMAGE_RB_TRANSFORMATION_ANTIDIAGFLIP:
			fiCalcReversedTransformationAntiDiagFlip(range_sum, domain_sum, rd_sum, domain_disp, range_block,
				dx, dy,
				rx, ry, blocksize,
				w, h,
				scaled_data_cuda,
				data_cuda
				);
			break;
	}

	mono_u = 0; mono_v = (int)( ((range_sum/(blocksize*blocksize))*63+128)/255 ); // Значения на случай, если совсем ни один блок не подойдёт

	if(domain_disp) {
		u = (int)( 32*((int)(blocksize*blocksize)*rd_sum-range_sum*domain_sum)/domain_disp ); // яркость в интервале [0, 1.0), т.е. от [0 до 32)
		if(u < 0) u = 0; if(u > 31) u = 31;
		v = (int)( (range_sum-domain_sum*u/32)/(int)(blocksize*blocksize) );
		if(v < -255 || v > 255) {
			u = mono_u;
			v = mono_v;
		} else {
			if(v < 0)
				v = (v*63-128)/255;
			else
				v = (v*63+128)/255;
		}
	} else {
		u = mono_u;
		v = mono_v;
	}

	mono_diff = 0; // Найдём разницу для mono_v и mono_u (mono_diff)
	diff = 0; // Найдём разницу для v и u блока dx, dy (diff)
	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = range_block[l*blocksize+m];
			int bd = scaled_data_cuda[(dy+l)*w/2+dx+m];

			int mult;

			mult = (mono_v*255/63-br);
			mono_diff += mult*mult;

			mult = (bd*u/32+v*255/63-br);
			diff += mult*mult;
		}
	}

	if(diff < mono_diff) {
		local_diff = diff;
		local_u = u;
		local_v = v;
	} else {
		local_diff = mono_diff;
		local_u = mono_u;
		local_v = mono_v;
	}

	unsigned int uniq_diff = (unsigned int)sqrtf(local_diff)*(blockDim.x*blockDim.y)+(threadIdx.y*blockDim.x+threadIdx.x); // 255*(IMAGE_MAXCUDABLOCKSIZE=64)*64*64=3FC000 < FFFFFFFF
	atomicMin(&best_diff, uniq_diff);
	__syncthreads();

	if(best_diff == uniq_diff) {
		kernel_block_cuda->dx = dx;
		kernel_block_cuda->dy = dy;
		kernel_block_cuda->tr = tr;
		kernel_block_cuda->u = local_u;
		kernel_block_cuda->v = local_v;
		kernel_block_cuda->diff = local_diff;
	}
}

static void fiFindBestDomainBlock(unsigned int rx, unsigned int ry,
	image_rangeblock_type *block_p, kernel_block_type *kernel_blocks_cuda, kernel_block_type *kernel_blocks, unsigned int blocksize,
	unsigned int w, unsigned int h,
	dim3 threads, dim3 blocks,
	int *scaled_data_cuda,
	int *data_cuda,
	unsigned int worst_diff
	)
{
	unsigned int best_diff, i;
	kernel_block_type *kernel_block;

	memset(kernel_blocks, 0, sizeof(kernel_block_type)*blocks.x*blocks.y*blocks.z);
	cudaMemcpy(kernel_blocks_cuda, kernel_blocks, sizeof(kernel_block_type)*blocks.x*blocks.y*blocks.z, cudaMemcpyHostToDevice);

	fiFindBestDomainBlockKernel <<<blocks, threads>>>(rx, ry, kernel_blocks_cuda, blocksize, w, h, scaled_data_cuda, data_cuda);

	cudaMemcpy(kernel_blocks, kernel_blocks_cuda, sizeof(kernel_block_type)*blocks.x*blocks.y*blocks.z, cudaMemcpyDeviceToHost);

	best_diff = kernel_blocks->diff;
	kernel_block = kernel_blocks;
	for(i = 0; i < blocks.x*blocks.y*blocks.z; i++) {
		if(kernel_blocks[i].diff < best_diff) {
			best_diff = kernel_blocks[i].diff;
			kernel_block = kernel_blocks+i;

		}
		//printf("-? rx %u ry %u i %u dx %u dy %u tr %u l_u %u l_v %d l_diff %u\n", rx, ry, i, kernel_blocks[i].dx, kernel_blocks[i].dy, kernel_blocks[i].tr, kernel_blocks[i].u, kernel_blocks[i].v, kernel_blocks[i].diff);
	}

	block_p->dx = kernel_block->dx;
	block_p->dy = kernel_block->dy;
	block_p->u = kernel_block->u;
	block_p->v = kernel_block->v;
	block_p->tr = kernel_block->tr;
	//printf("-! rx %u ry %u dx %u dy %u tr %u l_u %u l_v %d l_diff %u\n", rx, ry, kernel_block->dx, kernel_block->dy, kernel_block->tr, kernel_block->u, kernel_block->v, kernel_block->diff);

	if(best_diff > worst_diff && blocksize > 2 && blocksize%2 == 0) {
		printf("best_diff %d/%d (=rms^2*blocksize^2), divided block size %d to %d\n", best_diff, worst_diff, blocksize, blocksize/2);

		fiFindBestDomainBlock(rx, ry, // Позиция нового блока блока
			(image_rangeblock_type *)block_p->divided_into, kernel_blocks_cuda, kernel_blocks, // Указатель на новый блок, на блоки для вычислений на cuda, память под них на cpu
			blocksize/2, // Уменьшаем размер блока на 2
			w, h, threads, blocks, scaled_data_cuda, data_cuda,
			worst_diff/4); // Уменьшаем разницу на 4 (т.к. новый блок будет в 4 раза меньше)

		fiFindBestDomainBlock(rx+blocksize/2, ry, // Позиция нового блока блока
			(image_rangeblock_type *)block_p->divided_into+1, kernel_blocks_cuda, kernel_blocks, // Указатель на новый блок, на блоки для вычислений на cuda, память под них на cpu
			blocksize/2, // Уменьшаем размер блока на 2
			w, h, threads, blocks, scaled_data_cuda, data_cuda,
			worst_diff/4); // Уменьшаем разницу на 4 (т.к. новый блок будет в 4 раза меньше)

		fiFindBestDomainBlock(rx, ry+blocksize/2, // Позиция нового блока блока
			(image_rangeblock_type *)block_p->divided_into+2, kernel_blocks_cuda, kernel_blocks, // Указатель на новый блок, на блоки для вычислений на cuda, память под них на cpu
			blocksize/2, // Уменьшаем размер блока на 2
			w, h, threads, blocks, scaled_data_cuda, data_cuda,
			worst_diff/4); // Уменьшаем разницу на 4 (т.к. новый блок будет в 4 раза меньше)

		fiFindBestDomainBlock(rx+blocksize/2, ry+blocksize/2, // Позиция нового блока блока
			(image_rangeblock_type *)block_p->divided_into+3, kernel_blocks_cuda, kernel_blocks, // Указатель на новый блок, на блоки для вычислений на cuda, память под них на cpu
			blocksize/2, // Уменьшаем размер блока на 2
			w, h, threads, blocks, scaled_data_cuda, data_cuda,
			worst_diff/4); // Уменьшаем разницу на 4 (т.к. новый блок будет в 4 раза меньше)

		block_p->is_divided = true;
	}
}

#ifdef WIN32
unsigned int __stdcall fiWorkerFunc(void *arg)
#else
void *fiWorkerFunc(void *arg)
#endif
{
	unsigned int i;
	unsigned int rx, ry, channel, copied_channel;

	unsigned char *scaled_data = ((fi_worker_arg_type *)arg)->scaled_data;
	unsigned char **data = ((fi_worker_arg_type *)arg)->data;
	int *data_in_int = 0;
	image_rangeblock_type *block_start_p = ((fi_worker_arg_type *)arg)->block_p, *block_p;
	unsigned int nof_blocks_per_channel = ((fi_worker_arg_type *)arg)->nof_blocks_per_channel;
	unsigned int w = ((fi_worker_arg_type *)arg)->w;
	unsigned int h = ((fi_worker_arg_type *)arg)->h;
	unsigned int nof_channels = ((fi_worker_arg_type *)arg)->nof_channels;
	unsigned int alloc_memory = ((fi_worker_arg_type *)arg)->alloc_memory;
	unsigned int blocksize = ((fi_worker_arg_type *)arg)->blocksize;
	unsigned int worker_start = ((fi_worker_arg_type *)arg)->worker_start;
	unsigned int worker_step = ((fi_worker_arg_type *)arg)->worker_step;
	unsigned int worst_diff = ((fi_worker_arg_type *)arg)->worst_diff;

	unsigned int nof_blocks_per_image = nof_blocks_per_channel*nof_channels;

	int *scaled_data_cuda;
	kernel_block_type *kernel_blocks_cuda;
	kernel_block_type *kernel_blocks;
	int *data_cuda;

	cudaEvent_t start, stop;
	double cuda_working_time = 0;

	cudaDeviceProp device_prop;
	dim3 threads, blocks;

	ShowMeSomeCUDAInfo();

	if(blocksize > IMAGE_MAXCUDABLOCKSIZE) {
		printf("Sorry, but IMAGE_MAXCUDABLOCKSIZE=%d < blocksize=%d\b", IMAGE_MAXCUDABLOCKSIZE, blocksize);

		return 0;
	}

	printf("cudaMalloc(&scaled_data_cuda, alloc_memory/4=%d)\n", sizeof(int)*alloc_memory/4);
	if(cudaMalloc(&scaled_data_cuda, sizeof(int)*alloc_memory/4) != cudaSuccess) {
		printf("Can\'t allocate memory for scaled_data_cuda\n");

		return 0;
	}
	printf("cudaMalloc(&data_cuda, alloc_memory=%d)\n", sizeof(int)*alloc_memory);
	if(cudaMalloc(&data_cuda, sizeof(int)*alloc_memory) != cudaSuccess) {
		printf("Can\'t allocate memory for data_cuda_p\n");

		cudaFree(scaled_data_cuda);

		return 0;
	}

	if(cudaEventCreate(&start) != cudaSuccess) {
		printf("Can\'t create event \"start\"\n");

		cudaFree(scaled_data_cuda);
		cudaFree(data_cuda);

		return 0;
	}
	if(cudaEventCreate(&stop) != cudaSuccess) {
		printf("Can\'t create event \"stop\"\n");

		cudaFree(scaled_data_cuda);
		cudaFree(data_cuda);

		cudaEventDestroy(start);

		return 0;
	}

	cudaGetDeviceProperties(&device_prop, 0);

	int number_of_working_threads = device_prop.maxThreadsPerBlock/2; // Магия
	int number_of_working_threads_multiplier = 1;
	int threads_z = 1;

	if(device_prop.maxGridSize[2] < 8) {
		number_of_working_threads /= 8;
		threads_z = 8;
	}

	while( ((unsigned int)(sqrt((float)number_of_working_threads)+0.5)*(unsigned int)(sqrt((float)number_of_working_threads)+0.5)) != number_of_working_threads) {
		number_of_working_threads /= 2;
		number_of_working_threads_multiplier *= 2;
	}

	threads = dim3((unsigned int)(sqrt((float)number_of_working_threads)+0.5)*number_of_working_threads_multiplier, (unsigned int)(sqrt((float)number_of_working_threads)+0.5), threads_z);
	blocks = dim3(((w/2-blocksize+1)+threads.x-1)/threads.x, ((h/2-blocksize+1)+threads.y-1)/threads.y, 8/threads.z);

	printf("threads(%u,%u,%u) blocks(%u,%u,%u)\n", threads.x, threads.y, threads.z, blocks.x, blocks.y, blocks.z);

	if((int)(threads.x*threads.y*threads.z) > device_prop.maxThreadsPerBlock) {
		printf("threads.x*threads.y*threads.z is greater than %d, exiting...\n", device_prop.maxThreadsPerBlock);

		cudaFree(scaled_data_cuda);
		cudaFree(data_cuda);

		cudaEventDestroy(start);
		cudaEventDestroy(stop);

		return 0;
	}

	printf("cudaMalloc(&kernel_blocks_cuda, sizeof(image_rangeblock_type)*blocks.x*blocks.y*blocks.z=%d)\n", sizeof(kernel_block_type)*blocks.x*blocks.y*blocks.z);
	if(cudaMalloc(&kernel_blocks_cuda, sizeof(kernel_block_type)*blocks.x*blocks.y*blocks.z) != cudaSuccess) {
		printf("Can\'t allocate memory for block_start_cuda_p\n");

		cudaFree(scaled_data_cuda);
		cudaFree(data_cuda);

		cudaEventDestroy(start);
		cudaEventDestroy(stop);

		return 0;
	}

	printf("kernel_blocks = malloc(sizeof(kernel_block_type)*blocks.x*blocks.y*blocks.z=%d), sizeof(kernel_block_type)=%d\n", sizeof(kernel_block_type)*blocks.x*blocks.y*blocks.z, sizeof(kernel_block_type));
	kernel_blocks = (kernel_block_type *)malloc(sizeof(kernel_block_type)*blocks.x*blocks.y*blocks.z);
	if(kernel_blocks == 0) {
		printf("Can\'t allocate memory for block_start_cuda_p\n");

		cudaFree(scaled_data_cuda);
		cudaFree(data_cuda);
		cudaFree(kernel_blocks_cuda);

		cudaEventDestroy(start);
		cudaEventDestroy(stop);

		return 0;
	}

	copied_channel = nof_channels+1;
	block_p = block_start_p+worker_start;

	for(i = worker_start; i < nof_blocks_per_image; i += worker_step) {
		float one_block_time;

		channel = i / nof_blocks_per_channel;
		rx = ((i % nof_blocks_per_channel) % (w/blocksize)) * blocksize;
		ry = ((i % nof_blocks_per_channel) / (w/blocksize)) * blocksize;

		if(copied_channel != channel) {
			unsigned int j;

			if(!data_in_int)
				data_in_int = (int *)malloc(sizeof(int)*alloc_memory);

			if(!data_in_int) {
				printf("Can\'t allocate memory for data_in_int\n");

				break;
			}

			copied_channel = channel;

			for(j = 0; j < alloc_memory/4; j++) {
				data_in_int[j] = scaled_data[channel*alloc_memory/4+j];
			}
			cudaMemcpy(scaled_data_cuda, data_in_int, sizeof(int)*alloc_memory/4, cudaMemcpyHostToDevice);

			for(j = 0; j < alloc_memory; j++) {
				data_in_int[j] = data[channel][j];
			}
			cudaMemcpy(data_cuda, data_in_int, sizeof(int)*alloc_memory, cudaMemcpyHostToDevice);
		}

		if(((i-worker_start)/worker_step)%10 == 0)
			printf("worker %d %d rbs (%f) ok\n", worker_start+1, (i-worker_start)/worker_step,
				(float)( 100.0 * (double)((i-worker_start)/worker_step) / (double)((nof_blocks_per_image-worker_start-1)/worker_step+1) ));

		cudaEventRecord(start, 0);

		fiFindBestDomainBlock(rx, ry,
			block_p, kernel_blocks_cuda, kernel_blocks, blocksize,
			w, h,
			threads, blocks,
			scaled_data_cuda,
			data_cuda,
			worst_diff);

		cudaEventRecord(stop, 0);
		cudaEventSynchronize(stop);
		cudaEventElapsedTime(&one_block_time, start, stop);
		cuda_working_time += one_block_time;

		block_p += worker_step;
	}

	if(data_in_int)
		free(data_in_int);
	cudaFree(scaled_data_cuda);
	cudaFree(data_cuda);
	cudaFree(kernel_blocks_cuda);
	free(kernel_blocks);

	cudaEventDestroy(start);
	cudaEventDestroy(stop);

	if(worker_step == 1) { // Сбрасываем устройство, только если имеется только один поток
		if(cudaDeviceReset() != cudaSuccess) {
			printf("Can\'t reset cuda device\n");
		}
	}

	printf("cuda_working_time %f ms\n", cuda_working_time);

	return 0;
}

static void ShowMeSomeCUDAInfo(void)
{
	int device_count;
	cudaDeviceProp device_prop;

	cudaGetDeviceCount(&device_count);

	printf("Device count: %d\n\n", device_count);

	for(int i = 0; i < device_count; i++) {
		cudaGetDeviceProperties(&device_prop, i);

		printf("Device name: %s\n", device_prop.name);
		printf("Compute capability: %d.%d\n", device_prop.major, device_prop.minor);
		printf("Total constant memory: %lld\n", (long long)(device_prop.totalConstMem));
		printf("Total global memory: %lld\n", (long long)(device_prop.totalGlobalMem));
		printf("Shared memory per block: %lld\n", (long long)(device_prop.sharedMemPerBlock));
		printf("Registers per block: %d\n", device_prop.regsPerBlock);
		printf("Warp size: %d\n", device_prop.warpSize);
		printf("Memory pitch: %lld\n", (long long)(device_prop.memPitch));
		printf("Max threads per block: %d\n", device_prop.maxThreadsPerBlock);
		printf("Max threads dimensions: x = %d, y = %d, z = %d\n",
			device_prop.maxThreadsDim[0],
			device_prop.maxThreadsDim[1],
			device_prop.maxThreadsDim[2]);

		printf("Max grid size: x = %d, y = %d, z = %d\n",
			device_prop.maxGridSize[0],
			device_prop.maxGridSize[1],
			device_prop.maxGridSize[2]);

		printf("Max Surface1D: %d\n", device_prop.maxSurface1D);
		printf("Max Surface2D: %d\n", device_prop.maxSurface2D);

		printf("Clock rate: %d\n", device_prop.clockRate);
		printf("Total constant memory: %d\n", device_prop.totalConstMem);
		printf("Compute capability: %d.%d\n", device_prop.major, device_prop.minor);
		printf("Texture alignment: %d\n", device_prop.textureAlignment);
		printf("Device overlap: %d\n", device_prop.deviceOverlap);
		printf("Multiprocessor count: %d\n", device_prop.multiProcessorCount);
		printf("Kernel execution timeout enabled: %s\n",
			device_prop.kernelExecTimeoutEnabled ? "true" : "false");
	}

	printf("\n");

}
