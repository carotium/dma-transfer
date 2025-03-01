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

/**************************************************************
 * Variable definitions
 *************************************************************/
 u32 vgaArray[SCREEN_HEIGHT][SCREEN_WIDTH];

/**************************************************************
 * Function definitions
 *************************************************************/

/**************************************************************
 * clearVGA clears the VGA screen.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
 void clearVGA() {
	for(u32 y = 0; y < 480; y++) {
		for(u32 x = 0; x < 640; x++) {
			putPixel(x, y, black);
		}
	}
}

/**************************************************************
 * putPixel draws a pixel.
 *
 * @param	x is the x coordinate of the point.
 * @param	y is the y coordinate of the point.
 * @param	color is the color of the point.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void putPixel(u32 x, u32 y, colors color) {
	u8 *screen = (u8 *) *dataArray;
	u32 where = x * PIXEL_WIDTH + y * PITCH;

	screen[where] = color;				//RED
	screen[where + 1] = color >> 8;		//GREEN
	screen[where + 2] = color >> 16;	//BLUE
}

/**************************************************************
 * drawChar draws a character.
 *
 * @param	c is the character to be drawn.
 * @param	x is the x coordinate of the character.
 * @param	y is the y coordinate of the character.
 * @param	scale is the scale of the character.
 * @param	fgcolor is the color of the character.
 * @param	bgcolor is the background color of the character.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawChar(u8 c, u32 x, u32 y, u32 scale, colors fgcolor, colors bgcolor) {
	u32 i, j;
	u32 mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	u8 *letter = IBM_VGA_8x16 + (u32) c * 16;

	for(i = 0 ; i < (16 * scale); i++){
		for(j = 0; j < (8 * scale); j++){
			putPixel( (x + j), (y + i), (letter[i / scale] & mask[j / scale]) ? fgcolor : bgcolor);
		}
	}
}

/**************************************************************
 * drawText draws a text.
 *
 * @param	text is the text to be drawn.
 * @param	textP is the starting point of the text.
 * @param	scale is the scale of the text.
 * @param	fgcolor is the color of the text.
 * @param	bgcolor is the background color of the text.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawText(const char *text, point textP, u32 scale, colors fgcolor, colors bgcolor) {
	for(u32 i = 0; text[i]; i++) {
		drawChar(text[i], textP.x + i * CHAR_WIDTH * scale, textP.y, scale, fgcolor, bgcolor);
	}
}

/**************************************************************
 * drawStraight draws a straight line.
 *
 * @param	x0 is starting x coordinate of the line.
 * @param	y0 is starting y coordinate of the line.
 * @param	x1 is ending x coordinate of the line.
 * @param	y1 is ending y coordinate of the line.
 * @param	color is the color of the line.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawStraight(int x0, int y0, int x1, int y1, u32 color) {
	int dx = 0, dy = 0;
	//Horizontal or vertical and which direction
	if(x1 == x0) {
		//Vertical line
		if(y1 > y0) {
			//Vertical line moving down
			dy = 1;
		} else {
			//Vertical line moving up
			dy = -1;
		}
	} else if(x1 > x0) {
		//Horizontal line moving right
		dx = 1;
	} else {
		//Horizontal line moving left
		dx = -1;
	}
	if(dx != 0) {
		//Horizontal line
		while(1) {
			putPixel(x0, y0, color);
			x0 += dx;
			if(x0 == x1) {
				putPixel(x0, y0, color);
				break;
			}
		}
	} else {
		while(1) {
			putPixel(x0, y0, color);
			y0 += dy;
			if(y0 == y1) {
				putPixel(x0, y0, color);
				break;
			}
		}
	}
}

/**************************************************************
 * drawBox draws a box.
 *
 * @param	x0 is x coordinate of the top left corner of the box.
 * @param	y0 is y coordinate of the top left corner of the box.
 * @param	x1 is x coordinate of the bottom right corner of the box.
 * @param	y1 is y coordinate of the bottom right corner of the box.
 * @param	color is the color of the box.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawBox(int x0, int y0, int x1, int y1, colors color) {
	drawStraight(x0, y0, x0 + (x1 - x0), y0, color);	//top line
	drawStraight(x0, y0, x0, y0 + (y1 - y0), color);	//left line
	drawStraight(x1, y0, x1, y0 + (y1 - y0), color);	//right line
	drawStraight(x0, y1, x0 + (x1 - x0), y0, color);	//bottom line
}

/**************************************************************
 * drawSelector draws a selector box.
 *
 * @param	tL is the top left corner of the selector box.
 * @param	color is the color of the selector box.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawSelector(point tL, colors color) {
	int offset = 16;
	int height = 32;
	int width = 224;

	point tR;
	tR.x = tL.x + width;
	tR.y = tL.y;

	point bL;
	bL.x = tL.x;
	bL.y = tL.y + height;

	point bR;
	bR.x = tL.x + width;
	bR.y = tL.y + height;

	drawStraight(tL.x, tL.y, tL.x + offset, tL.y, color);	//top line_left
	drawStraight(tR.x - offset, tR.y, tR.x, tR.y, color);	//top line_right

	drawStraight(tL.x, tL.y, tL.x, tL.y + offset, color);	//left line_top
	drawStraight(bL.x, bL.y - offset, bL.x, bL.y, color);	//left line_bottom

	drawStraight(tR.x, tR.y, tR.x, tR.y + offset, color);	//right line_top
	drawStraight(bR.x, bR.y - offset, bR.x, bR.y, color);	//right line_bottom

	drawStraight(bL.x, bL.y, bL.x + offset, bL.y, color);	//bottom line_left
	drawStraight(bR.x - offset, bR.y, bR.x, bL.y, color);	//bottom line_right
}

/**************************************************************
 * selectSelector selects a selector box.
 *
 * @param	selector is the top left corner of the selector box.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void selectSelector(point selector) {
	drawSelector(selector, white);
}

/**************************************************************
 * unselectSelector unselects a selector box.
 *
 * @param	selector is the top left corner of the selector box.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void unselectSelector(point selector) {
	drawSelector(selector, d_gray);
}

/**************************************************************
 * unselectAll unselects all selector boxes.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void unselectAll() {
	unselectSelector(selectorWText1.selector);
	unselectSelector(selectorWText2.selector);
	unselectSelector(selectorWText3.selector);
	if(discovered) {
		drawSelector(selectorWText4.selector, l_red);
	} else drawSelector(selectorWText4.selector, black);

	drawText(selectorWText1.menuText, selectorWText1.menu, 2, d_gray, black);
	drawText(selectorWText2.menuText, selectorWText2.menu, 2, d_gray, black);
	drawText(selectorWText3.menuText, selectorWText3.menu, 2, d_gray, black);
	if(discovered) {
		drawText(selectorWText4.menuText, selectorWText4.menu, 2, l_red, black);
	} else drawText(selectorWText4.menuText, selectorWText4.menu, 2, black, black);
}

/**************************************************************
 * selectSelectorWText selects a selector box with text.
 *
 * @param	selectorWText is the selector box with text.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void selectSelectorWText(selectorWText selectorWText) {
	unselectAll();

	if(selectorWText.menu.y != selectorWText4.menu.y) {
		selectSelector(selectorWText.selector);
	} else drawSelector(selectorWText.selector, red);
}

/**************************************************************
 * selectMenu selects a menu.
 *
 * @param	menuText is the menu with text.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void selectMenu(selectorWText menuText) {
	unselectAll();

	selectSelector(menuText.selector);
	if(menuText.menu.y != selectorWText4.menu.y) {
		drawText(menuText.menuText, menuText.menu, 2, white, black);
	} else {
		drawSelector(menuText.selector, red);
		drawText(menuText.menuText, menuText.menu, 2, red, black);
	}
}