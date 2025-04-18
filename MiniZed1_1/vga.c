/*************************************************************
* File: vga.c
* Description: VGA driver for MiniZed board.
*
* Author: Ahac Rafael Bela
* Created on: 01.03.2025
* Last modified: 14.04.2025
*************************************************************/

/*************************************************************
* Include section
*************************************************************/
#include "vga.h"

/*************************************************************
* Global variable section
*************************************************************/
u32 vgaArray[SCREEN_HEIGHT][SCREEN_WIDTH];

point startPoints[256];
point endPoints[256];

/*************************************************************
* Function definition section
*************************************************************/

/*************************************************************
* clearVGA clears the VGA screen.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void clearVGA(void) {
	for(u32 y = 0; y < SCREEN_HEIGHT; y++) {
		for(u32 x = 0; x < SCREEN_WIDTH; x++) {
			point pos = {x, y};
			putPixel(pos, black);
		}
	}
}

/*************************************************************
* putPixel draws a pixel.
*
* @param	pos is the location of the pixel (x, y).
* @param	color is the color of the pixel.
*
* @return	None.
*
* @note		None.
*************************************************************/
void putPixel(point pos, colors color) {
	u8 *screen = (u8 *) *vgaArray;
	u32 where = pos.x * PIXEL_WIDTH + pos.y * PITCH;

	screen[where] = color;				//RED
	screen[where + 1] = color >> 8;		//GREEN
	screen[where + 2] = color >> 16;	//BLUE
}

/*************************************************************
* drawChar draws a character.
*
* @param	c is the character to be drawn.
* @param	pos is the location of the character (x, y).
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

/*************************************************************
* drawText draws a text.
*
* @param	text is the text to be drawn.
* @param	textP is the top left location of the text.
* @param	scale is the scale of the text.
* @param	fgcolor is the color of the text.
* @param	bgcolor is the background color of the text.
*
* @return	None.
*
* @note		None.
*************************************************************/
void drawText(const char *text, point textP, u32 scale, colors fgcolor, colors bgcolor) {
	for(u32 i = 0; text[i]; i++) {
		point pos = {textP.x + i * CHAR_WIDTH * scale, textP.y};
		drawChar(text[i], pos, scale, fgcolor, bgcolor);
	}
}

/*************************************************************
* drawStraight draws a straight line.
*
* @param	pos0 is starting location of the line.
* @param	pos1 is ending location of the line.
* @param	color is the color of the line.
*
* @return	None.
*
* @note		None.
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

/*************************************************************
* drawBox draws a box.
*
* @param	pos0 is the top left point of the box.
* @param	pos1 is the bottom right point of the box.
* @param	color is the color of the box.
*
* @return	None.
*
* @note		None.
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

/*************************************************************
* drawBoxFull draws a filled box.
*
* @param	pos0 is the top left point of the box.
* @param	pos1 is the bottom right point of the box.
* @param	color is the color of the box.
*
* @return	None.
*
* @note		None.
*************************************************************/
void drawBoxFull(point pos0, point pos1, colors color) {
	point tmp = (point) {pos0.x, pos0.y};
	while(tmp.x != pos1.x || tmp.y != pos1.y) {
		putPixel(tmp, color);
		if(tmp.x != pos1.x) {
			tmp.x++;
		} else {
			tmp.x = pos0.x;
			tmp.y++;
		}
	}
	putPixel(tmp, color);
}

/*************************************************************
 * drawSelector draws a selector box.
 *
 * @param	tL is the top left point of the selector box.
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

	//Top left offset points of the selector
	point tLoff1 = {tL.x + offset, tL.y};
	point tLoff2 = {tL.x, tL.y + offset};

	//Top right point of the selector
	point tR;
	tR.x = tL.x + width;
	tR.y = tL.y;
	point tRoff1 = {tR.x - offset, tR.y};
	point tRoff2 = {tR.x, tR.y + offset};

	//Bottom left point of the selector
	point bL;
	bL.x = tL.x;
	bL.y = tL.y + height;
	point bLoff1 = {bL.x, bL.y - offset};
	point bLoff2 = {bL.x + offset, bL.y};

	//Bottom right point of the selector
	point bR;
	bR.x = tL.x + width;
	bR.y = tL.y + height;
	point bRoff1 = {bR.x, bR.y - offset};
	point bRoff2 = {bR.x - offset, bR.y};

	//Draws left segment of the top line of the selector
	drawStraight(tL, tLoff1, color);
	//Draws right segment of the top line of the selector
	drawStraight(tRoff1, tR, color);
	//Draws top segment of the left line of the selector
	drawStraight(tL, tLoff2, color);
	//Draws bottom segment of the left line of the selector
	drawStraight(bLoff1, bL, color);
	//Draws top segment of the right line of the selector
	drawStraight(tR, tRoff2, color);
	//Draws bottom segment of the right line of the selector
	drawStraight(bRoff1, bR, color);
	//Draws left segment of the bottom line of the selector
	drawStraight(bL, bLoff2, color);
	//Draws right segment of the bottom line of the selector
	drawStraight(bRoff2, bR, color);
}

/**************************************************************
 * drawMenu draws the selection menu.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawMenu(void) {
	drawText("MiniZed 1.0", (point){0, 0}, 4, white, d_gray);

	drawSelector(selectorWText1.selector, white);
	drawSelector(selectorWText2.selector, d_gray);
	drawSelector(selectorWText3.selector, d_gray);

	drawText(selectorWText1.menuText, selectorWText1.menu, 2, white, black);
	drawText(selectorWText2.menuText, selectorWText2.menu, 2, d_gray, black);
	drawText(selectorWText3.menuText, selectorWText3.menu, 2, d_gray, black);
}

/**************************************************************
 * drawStage first clears the screen and then draws the starting
 * 			selection menu.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawStage(void) {
	clearVGA();
	drawMenu();
}

/**************************************************************
 * drawEcho draws the echo sub-program text at the top.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawEcho(void) {
	clearVGA();
	drawText("MiniZed 1.0: Echo                    (ESC to exit)", (point) {0, 0}, 2, white, d_gray);
}

/**************************************************************
 * drawLine draws the lines sub-program text at the top.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawLines(void) {
	clearVGA();
	drawText("MiniZed 1.0: Lines                   (ESC to exit)", (point) {0, 0}, 2, white, d_gray);
}

/**************************************************************
 * drawExit draws the exit sub-program text.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawExit(void) {
	clearVGA();
	drawText("Goodbye!", (point) {336, 268}, 2, white, black);
	usleep(10000);
}

/**************************************************************
 * drawExtras draws the extras sub-program text at the top.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void drawExtras(void) {
	clearVGA();
	drawText("MiniZed 1.0: Snake                   (ESC to exit)", (point) {0, 0}, 2, white, d_gray);
}

/*************************************************************
* selectSelector colors a selector box white (selected).
*
* @param	selector is the top left point of the selector box.
*
* @return	None.
*
* @note		None.
*************************************************************/
void selectSelector(point selector) {
	drawSelector(selector, white);
}

/*************************************************************
* unselectSelector colors a selector box dark gray (unselected).
*
* @param	selector is the top left point of the selector box.
*
* @return	None.
*
* @note		None.
*************************************************************/
void unselectSelector(point selector) {
	drawSelector(selector, d_gray);
}

/*************************************************************
* unselectAll colors all selector boxes dark gray.
*
* @param	None.
*
* @return	None.
*
* @note		None.
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

/*************************************************************
* selectSelectorWText selects a selector box with text.
*
* @param	selectorWText is the selector box with text.
*
* @return	None.
*
* @note		None.
*************************************************************/
void selectSelectorWText(selectorWText selectorWText) {
	unselectAll();

	if(selectorWText.menu.y != selectorWText4.menu.y) {
		selectSelector(selectorWText.selector);
	} else drawSelector(selectorWText.selector, red);
}

/*************************************************************
* selectMenu selects a menu.
*
* @param	menuText is the menu with text.
*
* @return	None.
*
* @note		None.
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

/*************************************************************
 * enterMenu enters the selected sub-program.
 *
 * @param	selectorWText is the selector for the sub-program
 * 			to enter.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void enterMenu(selectorWText selectorWText) {
	switch(selectorWText.selector.y) {
		case SELECTOR_Y(1):
			enterEcho();
			break;
		case SELECTOR_Y(2):
			enterLines();
			break;
		case SELECTOR_Y(3):
			enterExit();
			break;
		case SELECTOR_Y(4):
			enterExtras();
			break;
	}
}

/*************************************************************
 * enterEcho enters the echo sub-program.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void enterEcho(void) {
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
			else if((index.y + 15) < (SCREEN_HEIGHT - CHAR_HEIGHT)) {
				index.y += CHAR_HEIGHT;
				index.x = 0;
			} else {
				index = (point) {0, 32};
			}
		}
	} while (caughtChar != 0x1B);
	index = (point) {0, 32};
	clearVGA();
	drawStage();
}

/*************************************************************
 * enterEcho enters the lines sub-program.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void enterLines(void) {
	drawLines();

	do {
		static u32 t = 0;
		drawLinesB(t);
		if(t < 255) t++;
		else t = 0;
	} while(caughtChar != 0x1B);

	clearVGA();
	drawStage();
}

/*************************************************************
 * enterEcho enters the exit sub-program.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void enterExit(void) {
	drawExit();
	exit(0);
}

/*************************************************************
 * enterEcho enters the extras sub-program.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *************************************************************/
void enterExtras(void) {
	drawExtras();
	enterSnake();
}

/*************************************************************
* End of file
*************************************************************/
