/**************************************************************
* File: vga.h
* Description: VGA driver for MiniZed board.
*
* Author: Ahac Rafael Bela
* Created on: 01.03.2025
* Last modified: 01.03.2025
*************************************************************/
//Protection macro
#pragma once
#ifndef VGA_H
#define VGA_H

/**************************************************************
* Include section
*************************************************************/
#include "libs.h"
#include "IBM_VGA_8x16.h"

/**************************************************************
* Macro section
*************************************************************/
//How many bytes in vgaArray to go one pixel down
#define PITCH 		2560
//How many bytes in dataArray to next pixel
#define PIXEL_WIDTH	4
//How many pixels for a character
#define CHAR_WIDTH  8

/**************************************************************
* Enum section
*************************************************************/
typedef enum colors {
	black	=	0x0,
	blue	=	0x0A0000,
	green	=	0x000A00,
	cyan	=	0x0A0A00,
	red		=	0x00000A,
	purple	=	0x0A000A,
	brown	=	0x00050A,
	gray	=	0x0A0A0A,
	d_gray	=	0x050505,
	l_blue	=	0x0F0505,
	l_green	=	0x050F05,
	l_cyan	=	0x0F0F05,
	l_red	=	0x05050F,
	l_purple=	0x0F050F,
	yellow	=	0x050F0F,
	white	=	0x0F0F0F
} colors;

/**************************************************************
* Struct section
*************************************************************/
typedef struct point_t {
	int x;
	int y;
} point;

typedef struct selectorWText_t {
	point selector;
	point menu;
	const char *menuText;
} selectorWText;

/**************************************************************
* Variable declaration section
*************************************************************/
extern selectorWText selectorWText1;
extern selectorWText selectorWText2;
extern selectorWText selectorWText3;
extern selectorWText selectorWText4;

extern u32 discovered;

/**************************************************************
* Function prototype section
*************************************************************/
void clearVGA();
void putPixel(u32 x, u32 y, colors color);
void drawChar(u8 c, u32 x, u32 y, u32 scale, colors fgcolor, colors bgcolor);
void drawText(const char* text, point textP, u32 scale, colors fgcolor, colors bgcolor);
void drawStraight(int x0, int y0, int x1, int x2, u32 color);
void drawBox(int x0, int y0, int x1, int y1, colors color);
void drawSelector(point tL, colors color);
void selectSelector(point selector);
void unselectSelector(point selector);
void selectSelectorWText(selectorWText selectorWText);
void selectMenu(selectorWText menuText);
 
#endif /* VGA_H */

/**************************************************************
* End of file
*************************************************************/