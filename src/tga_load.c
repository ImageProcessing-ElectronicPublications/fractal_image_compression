
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tga_file.h"
#include "tga_load.h"

static size_t GetFileLength(FILE *f)
{
	size_t cur_pos, file_len;

	cur_pos = ftell(f);
	fseek(f, 0, SEEK_END);
	file_len = ftell(f);
	fseek(f, cur_pos, SEEK_SET);

	return file_len;
}

int tgaLoad(char *fname, image_pc_type *image)
{
	FILE *f;
	TGAHEADER head;
	unsigned int bpp;
	unsigned char *pal = 0;
	unsigned char *databuf = 0, *pdb = 0;
	unsigned char *temp, *p, *p2, *p3;
	unsigned char b;
	unsigned int i, j, k;
	size_t file_len;

	f = fopen(fname, "rb");

	if(!f)
		return TGALOAD_CANTOPENFILE;

	file_len = GetFileLength(f);

	if(fread(&head, sizeof(TGAHEADER), 1, f) != 1)
		return TGALOAD_DAMAGEDFILE;

	switch(head.DataType) { // Проверка правильности\поддержки файла
		case 1: // Проверяю изображения с палитрой
		case 9:
			if((head.ColorMap != 1) || (head.BitPerPel != 8)) {
				fclose(f);
				return TGALOAD_DAMAGEDFILE;
			}
			if(!((head.CmapDepth == 24) || (head.CmapDepth == 32))) {
				fclose(f);
				return TGALOAD_UNSUPPORTEDFILETYPE;
			}
			break;
		case 2: // Проверяю изображения без палитры (24,32 bpp)
		case 10:
			if(head.ColorMap != 0) {
				fclose(f);
				return TGALOAD_DAMAGEDFILE;
			}
			if(!((head.BitPerPel == 24) || (head.BitPerPel == 32))) {
				fclose(f);
				return TGALOAD_UNSUPPORTEDFILETYPE;
			}
			break;
		case 3: // Проверяю изображения без палитры (8 bpp, greyscale)
		case 11:
			if(head.ColorMap != 0) {
				fclose(f);
				return TGALOAD_DAMAGEDFILE;
			}
			if(head.BitPerPel != 8) {
				fclose(f);
				return TGALOAD_UNSUPPORTEDFILETYPE;
			}
			break;
		default:
			fclose(f);
			return TGALOAD_UNSUPPORTEDFILETYPE;
	}

	if((head.BitPerPel == 8) && (head.ColorMap == 0)) { // Настройка для greyscale-изображений
		bpp = 1;
	} else if((head.BitPerPel == 32) || ((head.BitPerPel == 8) && (head.CmapDepth == 32))) { // Настройка 32bpp изображений (и палитрой или без)
		bpp = 4;
	} else { // Настройка 24bpp изображений (и палитрой или без)
		bpp = 3;
	}

	image->nof_channels = bpp;
	image->w = head.TGAWidth;
	image->h = head.TGAHeight;

	image->alloc_memory = image->w*image->h*bpp;
	image->data = malloc(image->alloc_memory);
	if(!image->data) {
		fclose(f);
		return TGALOAD_MEMORYALLOCERROR;
	}
	if(head.ColorMap == 1) {
		pal = malloc(head.CmapLength*bpp);
		if(!pal) {
			free(image->data);
			fclose(f);
			return TGALOAD_MEMORYALLOCERROR;
		}
		fread(pal, 1, head.CmapLength*bpp, f);
	}

	// Пропускаю идентификатор
	fseek(f, head.IdLeight, SEEK_CUR);

	switch(head.DataType) { // Чтение изображения
		case 1: // Чтение изображения с палитрой
			temp = malloc(image->w*image->h);
			if(!temp) {
				free(image->data);
				free(pal);
				return TGALOAD_MEMORYALLOCERROR;
			}
			fread(temp, 1, image->w*image->h, f);
			p = image->data;
			p3 = temp;
			for(i = 0;i < image->w*image->h;i++) {
				p2 = pal+((head.CmapStart+*p3)*bpp);
				*p = *p2; p++; p2++;
				*p = *p2; p++; p2++;
				*p = *p2; p++;
				if(bpp == 4) {  p2++; *p = *p2; p++; }
				p3++;
			}
			free(temp);
			break;
		case 2: // Чтение 8(greyscale),24,32bit изображений
		case 3:
			fread(image->data, 1, image->alloc_memory, f);
			break;
		case 9: // Декодирование изображения с палитрой
			databuf = malloc(file_len-ftell(f));
			if(!databuf) {
				free(image->data);
				free(pal);
				fclose(f);
				return TGALOAD_MEMORYALLOCERROR;
			}
			fread(databuf, 1, file_len-ftell(f), f);

			pdb = databuf;
			i = 0;
			p = image->data;
			while(i < image->w*image->h) {
				b = *pdb; pdb++;
				if((i+(b&0x7F)+1) > image->w*image->h) {
					free(image->data);
					break; // Часть файла не удалось прочитать
				}
				if(b & 0x80) { // the packet is a Run-length Packet
					for(j = 0;j < (unsigned int)((b&0x7F)+1);j++) {
						p2 = pal+((head.CmapStart+(*pdb))*bpp);
						*p = *p2; p++; p2++;
						*p = *p2; p++; p2++;
						*p = *p2; p++; p2++;
						if(bpp == 4) { *p = *p2; p++; p2++; }
					}
					pdb++;
				} else { // the packet is a Raw Packet
					for(j = 0;j < (unsigned int)((b&0x7F)+1);j++) {
						p2 = pal+((head.CmapStart+pdb[j])*bpp);
						*p = *p2; p++; p2++;
						*p = *p2; p++; p2++;
						*p = *p2; p++; p2++;
						if(bpp == 4) { *p = *p2; p++; p2++; }
					}
					pdb += (b&0x7F)+1;
				}
				i += (b&0x7F)+1;
			}
			free(databuf);
			break;
		case 10: // Чтение 8(greyscale),24,32bit изображений
		case 11:
			databuf = malloc(file_len-ftell(f));
			if(!databuf) {
				free(image->data);
				free(pal);
				fclose(f);
				return TGALOAD_MEMORYALLOCERROR;
			}
			fread(databuf, 1, file_len-ftell(f), f);

			pdb = databuf;
			i = 0;
			p = image->data;
			while(i < image->w*image->h) {
				b = *pdb; pdb++;
				if((i+(b&0x7F)+1) > image->w*image->h) {
					free(databuf); 
					break; // Часть файла не удалось прочитать
				}
				if(b & 0x80) { // the packet is a Run-length Packet
					for(j = 0;j < (unsigned int)((b&0x7F)+1);j++) {
						p2 = pdb;
						for(k = 0;k < bpp;k++) {
							*p = *p2; p++; p2++; }
					}
					pdb += bpp;
				} else { // the packet is a Raw Packet
					memcpy(p, pdb, bpp*((b&0x7F)+1)); pdb += bpp*((b&0x7F)+1); p += bpp*((b&0x7F)+1);
				}
				i += (b&0x7F)+1;
			}
			free(databuf);
			break;
	}

	// Переворот по оси y (если необходимо, гимп может сохранять с этим флагом)
	if(head.Description & 0x20) {
		p = image->data;
		temp = malloc(image->w*bpp);
		if(temp) { // Можно здесь завершить работу функции, если !temp -^_^-
			p2 = &image->data[image->w*bpp*(image->h-1)];
			for(i = 0;i < image->h/2;i++) {
				memcpy(temp,p,image->w*bpp);
				memcpy(p,p2,image->w*bpp);
				memcpy(p2,temp,image->w*bpp);
				p += image->w*bpp;
				p2 -= image->w*bpp;
			}
			free(temp);
		}
	}

	if(head.ColorMap == 1) free(pal);
	fclose(f);

	return TGALOAD_OKAY;
}
