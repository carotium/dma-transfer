/**************************************************************
* File: lines.h
* Description: Implementation of Bresenhaam's line drawing
* algorithm. Scaled to 256 lines being drawn at one moment.
*
* Author: Ahac Rafael Bela
* Created on: 03.03.2025
* Last modified: 14.04.2025
*************************************************************/
//Protection macro
#pragma once
#ifndef LINES_H
#define LINES_H

/*************************************************************
* Include section
*************************************************************/
#include "vga.h"

/*************************************************************
* Function prototype section
*************************************************************/
//Draws a line using Bresenham's line drawing algorithm.
 void drawLineB(point start, point end, u32 color);
 //Erases a line using Bresenham's line drawing algorithm.
 void eraseLineB(point start, point end);
 //Draws 256 lines using Bresenham's line drawing algorithm.
 void drawLinesB(u32 t);
 //Initializes line's moving speed and sets a random starting point of the line.
 void initializeLines(void);
 //Calculates next coordinates for the line moving at a given speed.
 void calculateLine(u32 t);

#endif

 /*************************************************************
 * End of file
 *************************************************************/
