Репозиторий Fractal Image Compression
=====================================

Описание
--------

Простой кодек для фрактального сжатия изображений. Основывается на описании из первой главы книги Yuval Fisher, Fractal Image compression: Theory and Application.

Кодирование реализовано на CPU (compress.exe) и GPU (cuda_compress.exe - cuda 7.5, версии спецификации >=2.0, и cudaold_compress.exe - cuda 6.5, версии спецификации 1.1, 2.0). Имеется версия с поддержкой mpi.

Ссылки
------

[Репозиторий](https://github.com/ImageProcessing-ElectronicPublications/fractal_image_compression)

[Скачать](https://github.com/ImageProcessing-ElectronicPublications/fractal_image_compression/releases)

Для запуска под Windows необходим Windows XP и выше и Visual Studio 2013 runtime; для использования с mpi необходим mpich2 v1.4.1p1.

Использование
-------------

Как сжимать:

>compress.exe input.tga output.fi 32 4 5

где 32 - размер рангового блока (ширина и высота изображения должны делиться на 32; чем больше размер рангового блока, тем лучше сжатие), 4 - количество потоков ЦПУ, 5 - максимальное значение погрешности для пикселя (чем больше число, тем меньше файл, но хуже качество получившегося изображения).

Как разжимать:

>decompress.exe output.fi decompressed.tga

или

>decompress.exe output.fi decompressed.tga 3

если вы хотите увеличить ширину и высоту изображения в 3 раза (можно использовать любое целое число).

Сборка
------

Под Windows просто используйте Visual Studio 2013 и выше.

Под unix просто войдите в unix_makefiles и введите в консоли:

>make all

Для сборки под юниксами также необходим gcc и mpi (тестировал под Debian 7 с mpich2 версии 1.4.1p1).

Лицензия
--------

Нет её). Общественное достояние.
