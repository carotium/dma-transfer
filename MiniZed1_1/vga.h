/**************************************************************
 * File: vga.h
 * Description: VGA driver for MiniZed board.
 *
 * Author: Ahac Rafael Bela
 * Created on: 01.03.2025
 * Last modified: 14.04.2025
 *************************************************************/
//Protection macro
#pragma once
#ifndef VGA_H
#define VGA_H

/**************************************************************
* Include section
*************************************************************/
#include "libs.h"
#include "lines.h"
//IBM VGA 8 by 16 pixels font
#include "IBM_VGA_8x16.h"

/**************************************************************
* Macro section
*************************************************************/
//How many bytes in vgaArray to go one pixel down
#define PITCH 		3200
//How many bytes in dataArray to next pixel
#define PIXEL_WIDTH	4
//How many pixels for a character
#define CHAR_WIDTH  8
#define CHAR_HEIGHT 16
//How many characters per line
#define CHARS_PER_LINE 100
//Selector padding
#define SELECTOR_PADDING 3
//Selector is x-centered and has maximum space for 14 characters of 2x scale, so 224 pixels.
#define SELECTOR_X	(SCREEN_WIDTH / 2 - 7 * (CHAR_WIDTH * 2))
/*Selector's y coordinate is chosen so the selector is y-centered.
 * It's height is of a character with 2x scale with addition of 3 pixels high padding
 * at the top and bottom, so 38 pixels. */
#define SELECTOR_Y(s) \
	(SCREEN_HEIGHT / 2 - (CHAR_HEIGHT * 2 + SELECTOR_PADDING * 2) + 38 * (s-1))

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
typedef struct selectorWText_t {
	point selector;
	point menu;
	const char *menuText;
} selectorWText;

/**************************************************************
* Global variable section
*************************************************************/
extern selectorWText selectorWText1;
extern selectorWText selectorWText2;
extern selectorWText selectorWText3;
extern selectorWText selectorWText4;

extern u32 discovered;

extern point startPoints[256], endPoints[256];
extern int dx0, dx1, dy0, dy1;

/**************************************************************
* Function prototype section
*************************************************************/
//Clears the VGA screen.
void clearVGA(void);
//Draws a pixel.
void putPixel(point pos, colors color);
//Draws a character.
void drawChar(u8 c, point pos, u32 scale, colors fgcolor, colors bgcolor);
//Draws a text.
void drawText(const char* text, point textP, u32 scale, colors fgcolor, colors bgcolor);
//Draws a straight line.
void drawStraight(point pos0, point pos1, u32 color);
//Draws a box.
void drawBox(point pos0, point pos1, colors color);
//Draws a filled box.
void drawBoxFull(point pos0, point pos1, colors color);
//Draws a selector box.
void drawSelector(point tL, colors color);
//Draws the selection menu.
void drawMenu(void);
//Draws the starting selection menu.
void drawStage(void);
//Draws the echo sub-program header.
void drawEcho(void);
//Draws the line sub-program header.
void drawLines(void);
//Draws the exit sub-program.
void drawExit(void);
//Draws the extras sub-program.
void drawExtras(void);
//Colors a selector box white (selected).
void selectSelector(point selector);
//Colors a selector box dark gray (unselected).
void unselectSelector(point selector);
//Colors only the selected selector white and others dark gray.
void selectSelectorWText(selectorWText selectorWText);
//Selects and colors a menu selector appropriately.
void selectMenu(selectorWText selectorWText);
//Enters selected sub-program.
void enterMenu(selectorWText selectorWText);
 //Enters extras' snake game.
void enterSnake(void);
//Enters echo sub-program.
void enterEcho(void);
//Enters lines sub-program.
void enterLines(void);
//Enters exit sub-program.
void enterExit(void);
//Enters extras sub-program.
void enterExtras(void);

#endif /* VGA_H */

/**************************************************************
* End of file
*************************************************************/
