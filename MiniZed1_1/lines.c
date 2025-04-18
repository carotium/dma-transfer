/**************************************************************
* File: lines.c
* Description: Implementation of Bresenham's line drawing
* algorithm. Scaled to 256 lines being drawn at one moment.
*
* Author: Ahac Rafael Bela
* Created on: 08.04.2025
* Last modified: 10.04.2025
*************************************************************/

/*************************************************************
* Include section
*************************************************************/
#include "lines.h"

/*************************************************************
* Global variable section
*************************************************************/
//Moving speeds of two points of a line for each coordinate
int dx0, dx1, dy0, dy1;

/*************************************************************
* Function definition section
*************************************************************/

/*************************************************************
* drawLineB draws a line using Bresenham's line drawing algorithm.
*
* @param	start is the first point that defines the line to draw.
* @param	end is the second point that defines the line to draw.
* @param	color is the color of the line to draw.
*
* @return	None.
*
* @note		None.
*************************************************************/
void drawLineB(point start, point end, u32 color) {

	//Bresenham's line algorithm implementation
	int dx =  abs (end.x - start.x), sx = start.x < end.x ? 1 : -1;
  	int dy = abs (end.y - start.y), sy = start.y < end.y ? 1 : -1;
  	int err = dx - dy, e2; // error value e_xy

  	for (;;){  // loop
    	putPixel(start, color);
    	if(start.x == end.x && start.y == end.y) break;
		e2 = 2 * err;
		if (e2 >= -dy) {
			if(start.x == end.x) break;
			err -= dy;
			start.x += sx;
		} // e_xy+e_x > 0
		if (e2 <= dx) {
			if(start.y == end.y) break;
			err += dx;
			start.y += sy;
		} // e_xy+e_y < 0
	}
}

/*************************************************************
* eraseLineB erases the line using Bresenham's line drawing algorithm.
*
* @param	start is the first point that defines the line to erase.
* @param	end is the second point that defines the line to erase.
*
* @return	None.
*
* @note		None.
*************************************************************/
void eraseLineB(point start, point end) {
	//Bresenham's line algorithm implementation
	int dx =  abs (end.x - start.x), sx = start.x < end.x ? 1 : -1;
	int dy = abs (end.y - start.y), sy = start.y < end.y ? 1 : -1;
	int err = dx - dy, e2; // error value e_xy
	for (;;){  // loop
	    	putPixel(start, black);
	    	if(start.x == end.x && start.y == end.y) break;
			e2 = 2 * err;
			if (e2 >= -dy) {
				if(start.x == end.x) break;
				err -= dy;
				start.x += sx;
			} // e_xy+e_x > 0
			if (e2 <= dx) {
				if(start.y == end.y) break;
				err += dx;
				start.y += sy;
			} // e_xy+e_y < 0
		}
}

/*************************************************************
* drawLinesB draws 256 lines at a given time erasing old lines
* 			 using Bresenham's line drawing algorithm.
*
* @param	t is the index parameter for which line to draw.
*
* @return	None.
*
* @note		None.
*************************************************************/
void drawLinesB(u32 t) {
	drawLineB(startPoints[t], endPoints[t], rand()%16777215);
	usleep(50000);

	//Check for screen borders and reverse direction
	if(startPoints[t].x < abs(dx0) || startPoints[t].x >= (SCREEN_WIDTH-abs(dx0))) dx0 = -dx0;
	if(startPoints[t].y < (abs(dy0)+1) || startPoints[t].y >= (SCREEN_HEIGHT-abs(dy0)) || (startPoints[t].y-abs(dy0)) <= 32) dy0 = -dy0;
	if(endPoints[t].x < abs(dx1) || endPoints[t].x >= (SCREEN_WIDTH-abs(dx1))) dx1 = -dx1;
	if(endPoints[t].y < abs(dy1) || endPoints[t].y >= (SCREEN_HEIGHT-abs(dy1)) || (endPoints[t].y-abs(dy1)) <= 32) dy1 = -dy1;

	static int full = 0;
	if(t == 255) full = 1;

	u32 indexLast = (t == 255) ? 0 : (t + 1);

	if(full) {
		//Start deleting lines
		eraseLineB(startPoints[indexLast],
					endPoints[indexLast]);
		}
		calculateLine(indexLast);
}

/*************************************************************
* initializeLines initializes line's moving speed and sets a
* 			random starting point of the line.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void initializeLines(void) {
	dx0 = rand()%6 + 1;
	dx1 = rand()%6 + 1;
	dy0 = rand()%6 + 1;
	dy1 = rand()%6 + 1;

	startPoints[0] = (point) {rand()%640, rand()%448 + 32};
	endPoints[0] = (point) {rand()%640, rand()%448 + 32};
}

/*************************************************************
* calculateLine calculates next coordinates for the line moving
* 			at a given speed.
*
* @param	t is the index parameter for which line to draw.
*
* @return	None.
*
* @note	None.
*************************************************************/
void calculateLine(u32 t) {
	//Coordinates for next line
	u32 indexNext = (t == 0) ? 255 : (t-1)%256;
	startPoints[t%256].x = startPoints[indexNext].x + dx0;
	startPoints[t%256].y = startPoints[indexNext].y + dy0;
	endPoints[t%256].x = endPoints[indexNext].x + dx1;
	endPoints[t%256].y = endPoints[indexNext].y + dy1;
}

/*************************************************************
* End of file
*************************************************************/
