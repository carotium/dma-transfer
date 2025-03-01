/**************************************************************
 * File: vga.c
 * Description: VGA driver for MiniZed board.
 *
 * Author: Ahac Rafael Bela
 * Created on: 01.03.2025
 * Last modified: 01.03.2025
 *************************************************************/

 /**************************************************************
 * Include section
 *************************************************************/
#include "vga.h"

u32 vgaArray[SCREEN_HEIGHT][SCREEN_WIDTH];

void clearVGA();
void putPixel(u32 x, u32 y, colors color);
void drawChar(u8 c, u32 x, u32 y, u32 scale, colors fgcolor, colors bgcolor);
void drawText(const char* text, point textP, u32 scale, colors fgcolor, colors bgcolor);
void drawStraight(int x0, int y0, int x1, int x2, u32 color);
void drawBox(int x0, int y0, int x1, int y1, colors color);
void drawSelector(point tL, colors color);