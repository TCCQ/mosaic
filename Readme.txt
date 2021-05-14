This is a project to make sort of mosaic/fractured/stained glass images out of a passed image file. It is written in C.

CREDIT to Wernsey on github for his repository, "bitmap" (bmp.c and bmp.h), it is under the MIT license, which is why it is included in this project. 

Coming back to this readme, I want to provide some hesitancy about the above library. Could not get JPEG to work, nor could I get saving to work. 
The JPEG problem at least returns a proper error message, the saving one is a full on runtime error, SIGABRT and all. Gross.

This project is somewhat of a mess, it currently only works on pngs, is slow, and writes directly to the framebuffer. Maybe I will return if I feel like fixing those.
But frankly this was an excuse to get back used to C, and an attempted implementation of a cool closest-point-to-seed algorithm I found (Jump Flooding Algorithm, National University of Singapore), so it has fufilled its purpose. 

As always feel free to yank any or all of this, I tried to document at least a bit, so some of it may be salvageable. Good Luck! 
