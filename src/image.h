
#ifndef _IMAGE_H
#define _IMAGE_H

#include <stdbool.h>

typedef struct {
	unsigned int w;
	unsigned int h;
	unsigned int nof_channels;
	unsigned int alloc_memory; // Общий размер массива data
	unsigned char *data;
} image_pc_type; // pc == packed channels, каналы идут последовательно для одного пикселя, содержатся в одном массиве

typedef struct {
	unsigned int w;
	unsigned int h;
	unsigned int nof_channels;
	unsigned int alloc_memory; // Размер массива, выделенного под каждый канал в data
	unsigned char **data;
} image_uc_type; // uc == unpacked channels, отдельный массив под каждый канал (красный, зелёный, синий)

typedef struct {
	unsigned short dx; // Смещение по x доменного блока
	unsigned short dy; // Смещение по у доменного блока
	unsigned char tr; // Трансформация
	unsigned char u; // Контрастность
	short v; // Яркость
	void *divided_into; // Указывает на массив из 4-х дочерних блоков
	bool is_divided; // true, если блок поделён
	bool must_process;
} image_rangeblock_type;

#define IMAGE_MAXBLOCKSIZE 128

#define IMAGE_MAXDXY 1023
#define IMAGE_LHALFDXY 512
#define IMAGE_HHALFDXY 511

#define IMAGE_RB_TRANSFORMATION_NONE 0
#define IMAGE_RB_TRANSFORMATION_90RIGHT 1
#define IMAGE_RB_TRANSFORMATION_180RIGHT 2
#define IMAGE_RB_TRANSFORMATION_270RIGHT 3
#define IMAGE_RB_TRANSFORMATION_VFLIP 4
#define IMAGE_RB_TRANSFORMATION_HFLIP 5
#define IMAGE_RB_TRANSFORMATION_MAINDIAGFLIP 6
#define IMAGE_RB_TRANSFORMATION_ANTIDIAGFLIP 7

#endif
