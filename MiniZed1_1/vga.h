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
 * Macros section
 *************************************************************/
 //How many bytes in vgaArray to go one pixel down
#define PITCH 		2560
//How many bytes in dataArray to next pixel
#define PIXEL_WIDTH	4
//How many pixels for a character
#define CHAR_WIDTH  8