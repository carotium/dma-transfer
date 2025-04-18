/**************************************************************
 * File: snake.h
 * Description: Snake game implementation.
 *
 * Author: Ahac Rafael Bela
 * Created on: 08.04.2025
 * Last modified: 14.04.2025
 *************************************************************/
//Protection macro
#pragma once
#ifndef SNAKE_H
#define SNAKE_H

/*************************************************************
* Include section
*************************************************************/
#include "vga.h"

/*************************************************************
* Global variable section
*************************************************************/
extern point snakeGrid[35*35];
extern point snakeBody[1225];

/*************************************************************
* Struct section
*************************************************************/
typedef struct snakeBodyPart {
	point pos;			//Position of the snake part
	int grown;			//If the body part is grown
	int direction;		//Moving direction of the snake's part
} snakeBodyPart;

typedef struct fullSnakeBody {
	snakeBodyPart parts[1225];
	snakeBodyPart *nextGrowablePart;
	int length;
} fullSnakeBody;

typedef struct snakeFood {
	point pos;
	int consumed;
} snakeFood;

/*************************************************************
* Function definition section
*************************************************************/
//Initializes a 2D array with possible all locations of the snake and food.
void initializeGrid(void);
//Extracts the location out of the 2D array with all possible locations for the snake and food.
int getGridPos(point pos);
//Draws the controls for the snake.
void drawInstructions(colors textColor);
//Clears the controls for the snake.
void clearInstructions(void);
//Controls the snake game
void enterSnake(void);
//Initializes the snake
void initializeSnake(void);
//Draws the body of the snake
void drawBody(point pos, colors color);
//Draws the head of the snake
void drawHead(point pos, colors color);
//Decides in which direction to move the snake
void updateSnake(void);
//Draws parts of the snake.
void drawSnake(void);
//Erases snake's head and tail.
void eraseSnake(void);
//
int checkBoundaries(void);
//
void gameOver(void);
//
void spawnFood(void);
//Lengthens the snake by one upon eating food.
void growSnake(void);
//Updates snake's body part's positions.
void moveSnake(int dX, int dY);

#endif

/*************************************************************
* End of file
*************************************************************/
