/**************************************************************
* File: lines.h
* Description: Implementation of Bresenhaam's line drawing
* algorithm. Scaled to 256 lines being drawn at one moment.
*
* Author: Ahac Rafael Bela
* Created on: 03.03.2025
* Last modified: 03.03.2025
*************************************************************/
//Protection macro
#pragma once
#ifndef LINE_H
#define LINES_H

/*************************************************************
* Include section
*************************************************************/
#include "vga.h"

/*************************************************************
* Function prototype section
*************************************************************/
void calculateLine(u32 t);
void drawLineB(u32 x0, u32 y0, u32 x1, u32 y1, u32 color);
void eraseLineB(u32 x0, u32 y0, u32 x1, u32 y1);
void drawLinesB(u32 t);
void lineInitialize(int state);