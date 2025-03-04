/**************************************************************
* File: vga.c
* Description: VGA driver for MiniZed board.
*
* Author: Ahac Rafael Bela
* Created on: 01.03.2025
* Last modified: 04.03.2025
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
			point pos = {x, y};
			putPixel(pos, black);
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
void putPixel(point pos, colors color) {
	u8 *screen = (u8 *) *vgaArray;
	u32 where = pos.x * PIXEL_WIDTH + pos.y * PITCH;

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
void drawChar(u8 c, point pos, u32 scale, colors fgcolor, colors bgcolor) {
	u32 i, j;
	u32 mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	u8 *letter = IBM_VGA_8x16 + (u32) c * 16;

	for(i = 0 ; i < (16 * scale); i++){
		for(j = 0; j < (8 * scale); j++){
			point newPos = {pos.x + j, pos.y + i};
			putPixel( newPos, (letter[i / scale] & mask[j / scale]) ? fgcolor : bgcolor);
		}
	}
}

void drawCursor(point index) {
	drawChar('|', index, 1, white, black);
	usleep(500000);
	drawChar(' ', index, 1, white, black);
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
		point pos = {textP.x + i * CHAR_WIDTH * scale, textP.y};
		drawChar(text[i], pos, scale, fgcolor, bgcolor);
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
void drawStraight(point pos0, point pos1, u32 color) {
	int dx = 0, dy = 0;
	//Horizontal or vertical and which direction
	if(pos1.x == pos0.x) {
		//Vertical line
		if(pos1.y > pos0.y) {
			//Vertical line moving down
			dy = 1;
		} else {
			//Vertical line moving up
			dy = -1;
		}
	} else if(pos1.x > pos0.x) {
		//Horizontal line moving right
		dx = 1;
	} else {
		//Horizontal line moving left
		dx = -1;
	}
	if(dx != 0) {
		//Horizontal line
		while(1) {
			putPixel(pos0, color);
			pos0.x += dx;
			if(pos0.x == pos1.x) {
				putPixel(pos0, color);
				break;
			}
		}
	} else {
		while(1) {
			putPixel(pos0, color);
			pos0.y += dy;
			if(pos0.y == pos1.y) {
				putPixel(pos0, color);
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
void drawBox(point pos0, point pos1, colors color) {
	point topL = pos0;
	point topR = {pos1.x, pos0.y};
	point leftT = pos0;
	point leftB = {pos0.x, pos1.y};
	point rightT = {pos1.x, pos0.y};
	point rightB = pos1;
	point botL = {pos0.x, pos1.y};
	point botR = {pos1.x, pos0.y};

	drawStraight(topL, topR, color);	//top line
	drawStraight(leftT, leftB, color);	//left line
	drawStraight(rightT, rightB, color);	//right line
	drawStraight(botL, botR, color);	//bottom line
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

	point tLoff1 = {tL.x + offset, tL.y};
	point tLoff2 = {tL.x, tL.y + offset};

	point tR;
	tR.x = tL.x + width;
	tR.y = tL.y;
	point tRoff1 = {tR.x - offset, tR.y};
	point tRoff2 = {tR.x, tR.y + offset};

	point bL;
	bL.x = tL.x;
	bL.y = tL.y + height;
	point bLoff1 = {bL.x, bL.y - offset};
	point bLoff2 = {bL.x + offset, bL.y};

	point bR;
	bR.x = tL.x + width;
	bR.y = tL.y + height;
	point bRoff1 = {bR.x, bR.y - offset};
	point bRoff2 = {bR.x - offset, bR.y};

	drawStraight(tL, tLoff1, color);	//top line_left
	drawStraight(tRoff1, tR, color);	//top line_right

	drawStraight(tL, tLoff2, color);	//left line_top
	drawStraight(bLoff1, bL, color);	//left line_bottom

	drawStraight(tR, tRoff2, color);	//right line_top
	drawStraight(bRoff1, bR, color);	//right line_bottom

	drawStraight(bL, bLoff2, color);	//bottom line_left
	drawStraight(bRoff2, bR, color);	//bottom line_right
}

void drawMenu() {
	drawText("MiniZed 1.0", (point){0, 0}, 4, white, d_gray);

	drawSelector(selectorWText1.selector, white);
	drawSelector(selectorWText2.selector, d_gray);
	drawSelector(selectorWText3.selector, d_gray);

	drawText(selectorWText1.menuText, selectorWText1.menu, 2, white, black);
	drawText(selectorWText2.menuText, selectorWText2.menu, 2, d_gray, black);
	drawText(selectorWText3.menuText, selectorWText3.menu, 2, d_gray, black);
}

void drawStage(int stage) {
	clearVGA();
	switch(stage) {
	case 0:
		drawMenu();
		break;
	case 1:
		drawEcho();
		break;
	case 2:
		drawLines();
		break;
	case 3:
		drawExit();
		break;
	case 4:
		drawExtras();
		break;
	}
}

void drawEcho() {
	clearVGA();
	drawText("MiniZed 1.0: Echo          (ESC to exit)", (point) {0, 0}, 2, white, d_gray);
//	enterEcho();
}

void drawLines() {

}

void drawExit() {

}

void drawExtras() {

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
void selectMenu(selectorWText selectorWText) {
	unselectAll();

	selectSelector(selectorWText.selector);
	if(selectorWText.menu.y != selectorWText4.menu.y) {
		drawText(selectorWText.menuText, selectorWText.menu, 2, white, black);
	} else {
		drawSelector(selectorWText.selector, red);
		drawText(selectorWText.menuText, selectorWText.menu, 2, red, black);
	}
}

void enterMenu(selectorWText selectorWText) {
	if(selectorWText.menu.y == selectorWText1.menu.y) {
		enterEcho();
	} else if(selectorWText.menu.y == selectorWText2.menu.y) {
		enterLines();
	} else if(selectorWText.menu.y == selectorWText3.menu.y) {
		enterExit();
	} else if(selectorWText.menu.y == selectorWText4.menu.y) {
		enterExtras();
	}
}

void enterEcho() {
	drawEcho();
	static point index = {0, 32};
	int doPrint = 0;
	do {
		if(receivedCount == 1) {
			receivedCount--;
			char caughtWord[] = {caughtChar, '\0'};
			//Carriage return key
			if(caughtChar == 0xD) {
				index.x = (index.x%(CHAR_WIDTH * CHARS_PER_LINE)) ?
						index.x%(CHAR_WIDTH * CHARS_PER_LINE) + CHAR_WIDTH*CHARS_PER_LINE :
						index.x - CHAR_WIDTH;
				doPrint = 0;
			} else if(caughtChar != 0x9) doPrint = 1;
			//Tabulator key
			if(caughtChar == 0x9) {
				index.x = (index.x%(CHAR_WIDTH * CHARS_PER_LINE)) ?
						index.x%(CHAR_WIDTH * CHARS_PER_LINE) + CHAR_WIDTH*3 :
						index.x + CHAR_WIDTH*3;
				doPrint = 0;
			} else if(caughtChar != 0xD) doPrint = 1;

			if(doPrint) drawText(caughtWord, index, 1, white, black);

			if(index.x < (SCREEN_WIDTH - CHAR_WIDTH))index.x += CHAR_WIDTH;
			else if(index.y < (SCREEN_HEIGHT - 16)) {
				index.y += 16;
				index.x = 0;
			} else {
				index = (point) {0, 32};
			}
		}
	} while (caughtChar != 0x1B);
	index = (point) {0, 32};
	clearVGA();
	drawStage(0);
}

void enterLines() {

}

void enterExit() {

}

void enterExtras() {

}
