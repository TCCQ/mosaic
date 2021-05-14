#include "mosaic.h"
#include "bmp.h"

#define max(a,b) (a>b)? a:b

//global variables, seednum sets "resolution" of stained glass look 
const int seednum = 100;
struct point* seeds;

//for looping over surrounding 8 points, unit vectors in 8 directions, see spread 
const signed char order[8][2] = { {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1}};

//print small board to console, use for debugging 
void printb() {
  printf("\n");
  for (int a = 0; a < boardSize.y; a++) {
    for (int b = 0; b < boardSize.x; b++) {
      struct point p = {b,a};
      char v = get(p);
      if (v) {
        printf("%1i", (int)v);
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }
}

//given target and source points, check if target's seed is farther than source's from target 
//if so, update target's seed, returns 1 if this was the first time that target had been touched 
char update(struct point t, struct point s ) {
  int sidx = get(s); //seed indexes 
  int tidx = get(t);
  struct point sval = seeds[sidx];
  struct point tval = seeds[tidx];
  if (tidx == 0) {
    set(t,sidx);
    return 1;
  } else if (distance(t,sval) < distance(t,tval)) set(t, sidx);
  return 0;
}

//takes array of points (size of board), the index of the first unused point, and the size of the step 
//iterates over existing points steps them in 8 directions, updating said locations, see above 
int spread(struct point* parr, int nump, int stepl) {
  struct point cp;
  struct point target;
  int out = nump;
  for (int q = 0; q < nump; q++){
    cp = parr[q];
    for (int i = 0; i < 8; i++) {
        target = qp(cp.x + order[i][0]*stepl, cp.y + order[i][1]*stepl);
        if (target.x >= boardSize.x || target.x < 0 || target.y >= boardSize.y || target.y < 0) continue;
        if (update(target, cp)) {
          parr[out++] = target;
        }
    }
  }
  return out;
}

//return random 0 through (max-1), int
int grand(int max) {
  return (int)(((float)(rand())/RAND_MAX) * max);
}

//randomly place seeds on board. seeds[0] is special for marking untouched pixels 
void initseeds() {
  seeds[0] = qp(-1,-1);
  int max = boardSize.x * boardSize.y;
  int u;
  for (int s = 1; s < seednum; s++) {
    u = grand(max);
    seeds[s] = qp(u/boardSize.x, u%boardSize.x);
    set(seeds[s],s);
  }
}

//with generation done, gather color of seed groups, average, and copy to framebuffer 
//not happy with having to use the framebuffer, as this prevents applications in 95% of contexts (when X is running)
//tried to use bmp.h to write to file, simply does not work, and I am not sifting through 6000 lines to find 
//  an error based on differences in which C standard I am using. 
//I may come back to improve this, we will see 
//(ps. you wouldn't think that finding bitmap libraries for C would be that hard, its been around for like 
//  50 years, and bitmaps are not exactly high tech, but /shrug 
void outputImage(Bitmap* input) {
  //get average color of each seed group (dumb, average r,g,b seperately) 
  int* sred = malloc(sizeof(int)*seednum);
  memset(sred, 0x00, sizeof(int)*seednum);
  int* sgreen = malloc(sizeof(int)*seednum);
  memset(sgreen, 0x00, sizeof(int)*seednum);
  int* sblue = malloc(sizeof(int)*seednum);
  memset(sblue, 0x00, sizeof(int)*seednum);
  int* scounter = malloc(sizeof(int)*seednum);
  memset(scounter, 0x00, sizeof(int)*seednum);
  //initalize all the counters 

  for (int y = 0 ; y < boardSize.y; y++) {
    for (int x = 0; x < boardSize.x; x++) {
      int c = bm_get(input, x, y);
      struct point p = {x,y};
      int sidx = (int)get(p);
      sred[sidx] += c & 0xFF;
      sgreen[sidx] += (c >> 8) & 0xFF;
      sblue[sidx] += (c >> 8*2) & 0xFF;
      scounter[sidx]++;
    }
  }
  //fill data 
  
  for (int i = 1; i < seednum; i++) {
    sred[i] = (int)((float)sred[i]/scounter[i]);
    sgreen[i] = (int)((float)sgreen[i]/scounter[i]);
    sblue[i] = (int)((float)sblue[i]/scounter[i]);
  }
  //average rgb over each seeds area 

  //screen information 
  const int w = 1920;
  const int h = 1080;
  const int bbp = 32;
  char* fbptr = initfb(w,h,bbp);
  if (fbptr == NULL) {
    printf("error\n"); //couldn't map properly, see mosaic.h 
  } else {
    for (int y = 0; y < boardSize.y; y++) {
      for (int x = 0; x < boardSize.x; x++) { //iterate over input pixels 
        char blue, green, red, alpha;
        int sidx = (int)get(qp(x,y)); //get seed group 

        blue = sblue[sidx];
        red = sred[sidx];
        green = sgreen[sidx];
        alpha = 0xFF; //alpha seems to have no effect at all?

        //write to output fb 
        fbptr[(x+(y*w))*(bbp/8)] = blue;
        fbptr[(x+(y*w))*(bbp/8) + 1 ] = green;
        fbptr[(x+(y*w))*(bbp/8) + 2] = red;
        fbptr[(x+(y*w))*(bbp/8) + 3] = alpha;
      }
    }
  }
  
  //release memory 
  free(sred);
  free(sgreen);
  free(sblue);
  free(scounter);
  munmap(fbptr, w*h*bbp/8);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Need at least 1 argument\n");
    return 1;
  }

  //initialize random 
  time_t t;
  srand((unsigned) time(&t));
  
  //copy pixel data from file to memory and set global variables 
  struct bitmap* input = bm_load(argv[1]);
  boardSize = qp(bm_width(input),bm_height(input));
  board = malloc(boardSize.x*boardSize.y);
  memset(board, 0x00, boardSize.x*boardSize.y);
  seeds = malloc(sizeof(struct point*) * seednum);

  //place seeds, see above 
  initseeds();
  
  //make point array for marking which points have been touched and need to be iterated over 
  struct point* allp = malloc(sizeof(struct point) * boardSize.x * boardSize.y);
  int pnum = seednum-1;
  for (int i = 1; i < seednum; i++) {
    allp[i-1] = seeds[i]; //copy starting seeds into spread array 
  }

  //main seedgroup finding loop. most of the time spent in the program is this loop 
  int stepl = max(boardSize.x, boardSize.y)/2;
  do {
    pnum = spread(allp, pnum, stepl);
    //printf("%i\n", pnum); //give general sense of time of each 'generation' 
    stepl = stepl/2;
  } while (stepl > 0);
  
  free(allp); //no longer needed, and nontrival amount of memory for large images 

  //one addition round removes most errors around edges, but is by far the larges time sink. 
  //every pixel does a full check on the 8 around it. very slow.
//  stepl = 1;
//  for (int i = 0; i < seednum; i++) {
//      pnum = spread(allp, pnum, stepl);
//  }

  //generate and print the finalized image to the framebuffer 
  outputImage(input);

  free(seeds);
  free(board);
  return 0;
}
