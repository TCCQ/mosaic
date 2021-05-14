#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>

//basic structs and global variables required in both files 
//not sure if writing implimentation in a header file is good practice, but I am doing it anyway 
//frankly this is an excuse just to offload some of the trivial QoL functions to a seperate file 
//makes debugging less of a pain imo 
struct point {
  int x,y;
};

char* board;
struct point boardSize;

struct point qp(int x, int y) {
  struct point out = {x,y};
  return out;
}

//euclidian distance between two pixels, commented is manhatten street distance 
//frankly this is the biggest time lose in the program. it is simply *too slow* to be called on every pixel so many times 
//M. distance works, but leads to some funky shapes. So much faster though 
float distance (struct point a, struct point b) {
  //return abs(a.x - b.x) + abs(a.y - b.y);
  return sqrt( pow(fabsf((float)a.x-b.x), 2) +  pow(fabsf((float)a.y-b.y), 2) );
}

//QoL, set board to value at x,y
void set(struct point p, char value) {
  board[p.y*boardSize.x + p.x] = value;
}

//opposite of above, pull value from board 
char get(struct point p) {
  return board[p.y*boardSize.x + p.x];
}

//open the framebuffer and map it onto an array that is returned. 
//you have to pass expected size and bitdepth to prevent some hard to debug problems. 
//(frankly this is an excuse to figure out how mmap works) 
char* initfb(int w, int h, int bbp) {
  //confim the bbp is correct 
  FILE* fbbp = fopen("/sys/class/graphics/fb0/bits_per_pixel", "r");
  if (!fbbp) return NULL;
  char localstring[64]; 
  localstring[fread(localstring, 1, 64, fbbp)] = 0x00; //null ternimate
  fclose(fbbp);
  if (atoi(localstring) != bbp) return NULL; //not the right bitdepth

  //confirm that the size will fit
  FILE* fWidthAndHeight = fopen("/sys/class/graphics/fb0/virtual_size", "r");
  if (!fbbp) return NULL;
  localstring[fread(localstring, 1, 64, fWidthAndHeight)] = 0x00; //null terminate 
  int idx = 0; 
  while (localstring[idx] != ',') {
    idx++;
  }
  localstring[idx] = 0x00; //middle terminator 
  int fbw = atoi(localstring); //get first number 
  int fbh = atoi(localstring + idx+1);
  if ( w != fbw || h != fbh) return NULL; //desired size is too large 
  fclose(fWidthAndHeight);

  int fb = open("/dev/fb0", O_RDWR); //not sure what mode to use
  //if (!fb) return NULL;
  printf("%i\n", w*h*bbp/8);
  void* out = mmap(NULL, w*h*bbp/8, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
  if (out == MAP_FAILED) {
    printf("mapp failed\n");
    return NULL;
  }
  return (char*)out;
}
