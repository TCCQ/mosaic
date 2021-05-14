gc=-std=gnu18 -Wall -Wextra -Werror -g -pg -DUSEJPG -DUSEPNG `libpng-config --cflags` 
gl=-std=gnu18 -lm -Wall -Wextra -Werror -g -pg -ljpeg `libpng-config --ldflags` -lz

mosaic: mosaic.o bmp.o
	gcc ${gl} mosaic.o bmp.o -o mosaic 

mosaic.o: mosaic.c mosaic.h bmp.h
	gcc -c ${gc} mosaic.c -o mosaic.o

bmp.o: bmp.c bmp.h
	gcc -c ${gc} bmp.c -o bmp.o

clean: 
	rm *.o mosaic
