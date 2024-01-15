
#ifndef _FI_FILE_H
#define _FI_FILE_H

#define FI_SIGN 0xC0F1

typedef struct {
	unsigned short sign; // Сигнатура
	unsigned short blocksize; // Размер блока. if(blocksize == 0) blocksize = 8;
	unsigned int w;
	unsigned int h;
	unsigned int noc; // Количество каналов
} FI_HEADER;

#endif
