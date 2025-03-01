# DMA-Transfer
A library for initiating dma transfers. Hardware project is based on the [***Minized board***](https://www.avnet.com/wps/portal/us/products/avnet-boards/avnet-board-families/minized/).
## File descriptions
### [Main program](MiniZed1_0/main.c)
In the main program, the UART, DMA and interrupts are initialized. Then a keyboard input is expected, before enabling the interrupts. The program then enters a while loop, which scans for user input and prints corresponding characters on the screen. The used font is standard [IBM 8x16 BIOS](https://int10h.org/oldschool-pc-fonts/fontlist/font?ibm_vga_8x16#-). There is also an implementation for drawing lines using Bresenham's line algorithm with random colors for each pixel but is currently disabled. Further programming needs to be done.
### [Libraries program](MiniZed1_0/libs.c)
Defined here are library functions:
- initPlatform(controllers *components), initializes UART, DMA and interrupts:
- - initDMA(controllers *components),
- - initUART(controllers *components),
- - initInterrupt(controllers *components).
### [Libraries header](MiniZed1_0/libs.h)
Besides declaring functions, here are structs and variables that are relevant and used in the project:\
***Structs***:
+ controllers.

***Variables***:
- **components**, controllers struct
- **dataArray**, 2D array of *u32* RGB values
