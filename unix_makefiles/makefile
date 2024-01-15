CC=gcc
CPP=g++
CFLAGS=-c -Wall -O2
CFLAGS_MPI=$(CFLAGS) -DI_USE_MPI -I/usr/include/mpich2/
SOURCES_COMPRESS=../src/arrays.c ../src/compress.c ../src/fi_save.c ../src/fi_save_blocks_compression.c ../src/fi_save_blocks_search.c ../src/image_misc.c ../src/tga_load.c ../src/cpu_driven/fi_save_blocks_search_workers.c
OBJECTS_COMPRESS=arrays.o compress.o fi_save.o fi_save_blocks_compression.o fi_save_blocks_search.o image_misc.o tga_load.o fi_save_blocks_search_workers.o
SOURCES_DECOMPRESS=../src/arrays.c ../src/decompress.c ../src/fi_load.c ../src/image_misc.c ../src/tga_save.c
OBJECTS_DECOMPRESS=arrays.o decompress.o fi_load.o image_misc.o tga_save.o

all: release release_mpi

release: compress decompress

release_mpi: compress_mpi

compress:
	$(CC) $(CFLAGS) $(SOURCES_COMPRESS)
	$(CPP) -o Release/compress -lpthread $(OBJECTS_COMPRESS) -s
	
compress_mpi:
	$(CC) $(CFLAGS_MPI) $(SOURCES_COMPRESS)
	$(CPP) -o ReleaseMPI/compress -lpthread -lmpi $(OBJECTS_COMPRESS) -s

decompress:
	$(CC) $(CFLAGS) $(SOURCES_DECOMPRESS)
	$(CPP) -o Release/decompress $(OBJECTS_DECOMPRESS) -s

clean:
	rm -rf *.o Release/compress Release/decompress ReleaseMPI/compress
