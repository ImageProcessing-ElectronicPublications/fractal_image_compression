
#ifndef _TGA_FILE_H
#define _TGA_FILE_H

#pragma pack (push, 1)
typedef struct {
	unsigned char IdLeight;		//Длина информации после заголовка
	unsigned char ColorMap;		//Идентификатор наличия цветовой карты (0 - нет, 1 - есть)
	unsigned char DataType;		//Тип сжатия
								//   0 - No Image Data Included
								//   1 - Uncompressed, Color-mapped Image
								//   2 - Uncompressed, True-color Image
								//   3 - Uncompressed, Black-and-white Image
								//   9 - Run-length encoded, Color-mapped Image
								//   10 - Run-length encoded, True-color Image
								//   11 - Run-length encoded, Black-and-white Image
	unsigned short CmapStart;	//Начало палитры
	unsigned short CmapLength;	//Длина палитры
	unsigned char CmapDepth;	//Глубина элементов палитры (15, 16, 24, 32)
	unsigned short X_Origin;	//Начало изображения по оси X
	unsigned short Y_Origin;	//Начало изображения по оси Y
	unsigned short TGAWidth;	//Ширина изображения
	unsigned short TGAHeight;	//Высота изображения
	unsigned char BitPerPel;	//Кол-во бит на пиксель (8, 16, 24, 32)
	unsigned char Description;	//Описание
} TGAHEADER;
#pragma pack (pop)

#endif
