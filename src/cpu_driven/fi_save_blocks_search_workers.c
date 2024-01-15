
#include <stdio.h>
#include <string.h>

#include "../image_misc.h"
#include "../fi_save_blocks_search_workers.h"

static void fiFindBestDomainBlock(unsigned int rx, unsigned int ry,
	image_rangeblock_type *block_p, unsigned int blocksize,
	unsigned int w, unsigned int h,
	unsigned char *scaled_data_p,
	unsigned char *data_p,
	unsigned int worst_diff
	)
{
	unsigned int best_diff;
	long long range_sum, domain_sum, rd_sum; // Сумма пикселей рангового блока, доменного блока, пикселя рангового на пиксель доменного блока
	long long domain_disp;
	unsigned int l, m, s, t, tr;
	unsigned char rotated_data_p[8*IMAGE_MAXBLOCKSIZE*IMAGE_MAXBLOCKSIZE];

	for(l = 0; l < blocksize; l++) {
		memcpy(rotated_data_p+blocksize*l, data_p+w*(ry+l)+rx, blocksize);
	}
	for(l = 1; l < 8; l++) {
		ApplyReversedTransformToRangeBlock(rotated_data_p, rotated_data_p+l*blocksize*blocksize, l, blocksize);
	}

	range_sum = 0;
	for(l = 0; l < blocksize; l++)
		for(m = 0; m < blocksize; m++) {
			range_sum += rotated_data_p[l*blocksize+m];
		}
	block_p->u = 0; block_p->v = (short)( ((range_sum/(blocksize*blocksize))*63+128)/255 ); // Значения на случай, если совсем ни один блок не подойдёт
	best_diff = 0; // Вообще, тут можно поставить любое число > 255*255*blocksize*blocksize=4161600(для 8), но мы найдём разницу для v и u выше
	for(l = 0; l < blocksize; l++) {
		for(m = 0; m < blocksize; m++) {
			int br = rotated_data_p[l*blocksize+m];
			int mult;

			mult = (block_p->v*255/63-br);
			best_diff += mult*mult;
		}
	}
	//printf("range_disp %d, range_sum %d\n", range_disp, range_sum);

	for(s = 0; s <= h/2-blocksize; s++) {
		for(t = 0; t <= w/2-blocksize; t++) {
			domain_disp = domain_sum = 0;
			for(l = 0; l < blocksize; l++) {
				for(m = 0; m < blocksize; m++) {
					int bd = scaled_data_p[(s+l)*w/2+t+m];

					domain_sum += bd;
					domain_disp += bd*bd;
				}
			}
			domain_disp = domain_disp*(int)(blocksize*blocksize)-domain_sum*domain_sum;

			for(tr = 0; tr < 8; tr++) {
				unsigned int cur_diff = 0;
				int u_int, v_int;

				if(domain_disp == 0) {
					u_int = 0;
					v_int = (int)( ((range_sum/(blocksize*blocksize))*63+128)/255 );
				} else {
					rd_sum = 0;
					for(l = 0; l < blocksize; l++) {
						for(m = 0; m < blocksize; m++) {
							int br = rotated_data_p[tr*blocksize*blocksize+l*blocksize+m];
							int bd = scaled_data_p[(s+l)*w/2+t+m];

							rd_sum += br*bd;
						}
					}

					u_int = (int)( 32*((int)(blocksize*blocksize)*rd_sum-range_sum*domain_sum)/domain_disp ); // яркость в интервале [0, 1.0), т.е. от [0 до 32)
					// av_u += u_int; nof_u++;
					//if(u_int < 0) {/*printf("u = %f\n", (float)(u_int/51.0));*/u_int = -u_int;} if(u_int > 255) {/*printf("u = %f\n", (float)(u_int/51.0));*/u_int = 255;}
					//if(u_int < 0 || u_int > 255) continue;
					if(u_int < 0) u_int = 0; if(u_int > 31) u_int = 31;
					v_int = (int)( (range_sum-domain_sum*u_int/32)/(int)(blocksize*blocksize) );
					if(v_int < -255 || v_int > 255) continue;
					if(v_int < 0)
						v_int = (v_int*63-128)/255;
					else
						v_int = (v_int*63+128)/255;

					//if(v_int < -255) v_int = -255; if(v_int > 255) v_int = 255;
					//printf("resulted u %d v %d\n", u_int, v_int);
				}

				for(l = 0; l < blocksize; l++) {
					for(m = 0; m < blocksize; m++) {
						int br = rotated_data_p[tr*blocksize*blocksize+l*blocksize+m];
						int bd = scaled_data_p[(s+l)*w/2+t+m];
						int mult;

						mult = (bd*u_int/32+v_int*255/63-br);
						cur_diff += mult*mult;
					}
				}
				//printf("%d < %d\n", cur_diff, best_diff);
				if(cur_diff < best_diff) {
					best_diff = cur_diff;
					block_p->dx = t;
					block_p->dy = s;
					block_p->tr = tr;
					block_p->u = u_int;
					block_p->v = v_int;
				}

				if(domain_disp == 0 || best_diff == 0)
					break;
			}
			/*if(t%10 == 0)
				printf("- %d dbs ok\n", i*max_dby*max_dbx+s*max_dbx+t);*/
			if(best_diff == 0)
				break;
		}

		if(best_diff == 0)
			break;
	}

	if(best_diff > worst_diff && blocksize > 2 && blocksize%2 == 0) {
		printf("best_diff %d/%d (=rms^2*blocksize^2), divided block size %d to %d\n", best_diff, worst_diff, blocksize, blocksize/2);

		fiFindBestDomainBlock(rx, ry, // Позиция нового блока блока
			(image_rangeblock_type *)block_p->divided_into, // Указатель на новый блок
			blocksize/2, // Уменьшаем размер блока на 2
			w, h, scaled_data_p, data_p,
			worst_diff/4); // Уменьшаем разницу на 4 (т.к. новый блок будет в 4 раза меньше)

		fiFindBestDomainBlock(rx+blocksize/2, ry, // Позиция нового блока блока
			(image_rangeblock_type *)block_p->divided_into+1, // Указатель на новый блок
			blocksize/2, // Уменьшаем размер блока на 2
			w, h, scaled_data_p, data_p,
			worst_diff/4); // Уменьшаем разницу на 4 (т.к. новый блок будет в 4 раза меньше)

		fiFindBestDomainBlock(rx, ry+blocksize/2, // Позиция нового блока блока
			(image_rangeblock_type *)block_p->divided_into+2, // Указатель на новый блок
			blocksize/2, // Уменьшаем размер блока на 2
			w, h, scaled_data_p, data_p,
			worst_diff/4); // Уменьшаем разницу на 4 (т.к. новый блок будет в 4 раза меньше)

		fiFindBestDomainBlock(rx+blocksize/2, ry+blocksize/2, // Позиция нового блока блока
			(image_rangeblock_type *)block_p->divided_into+3, // Указатель на новый блок
			blocksize/2, // Уменьшаем размер блока на 2
			w, h, scaled_data_p, data_p,
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
	unsigned int rx, ry, channel;

	unsigned char *scaled_data = ((fi_worker_arg_type *)arg)->scaled_data;
	unsigned char **data = ((fi_worker_arg_type *)arg)->data;
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

	block_p = block_start_p+worker_start;

	for(i = worker_start; i < nof_blocks_per_image; i += worker_step) {
		channel = i / nof_blocks_per_channel;
		rx = ((i % nof_blocks_per_channel) % (w/blocksize)) * blocksize;
		ry = ((i % nof_blocks_per_channel) / (w/blocksize)) * blocksize;

		if(((i-worker_start)/worker_step)%10 == 0)
			printf("worker %d %d rbs (%f) ok\n", worker_start+1, (i-worker_start)/worker_step,
				(float)( 100.0 * (double)((i-worker_start)/worker_step) / (double)((nof_blocks_per_image-worker_start-1)/worker_step+1) ));

		fiFindBestDomainBlock(rx, ry,
			block_p, blocksize,
			w, h,
			scaled_data+channel*alloc_memory/4,
			data[channel],
			worst_diff);

		block_p += worker_step;
	}

	return 0;
}
