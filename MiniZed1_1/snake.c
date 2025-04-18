/**************************************************************
 * File: snake.c
 * Description: Snake game implementation.
 *
 * Author: Ahac Rafael Bela
 * Created on: 08.04.2025
 * Last modified: 14.04.2025
 *************************************************************/

/*************************************************************
* Include section
*************************************************************/
#include "snake.h"

/*************************************************************
* Globar variable section
*************************************************************/
fullSnakeBody fullSnake;
point snakeBody[1225];

snakeFood foodStack[4];
static int foodCount = 0;
point snakeGrid[35*35];

/*************************************************************
* Function prototype section
*************************************************************/

/*************************************************************
* initializeGrid initializes a 2D array with all possible
* 			locations of the snake and food.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void initializeGrid(void) {
	for(int y = 0; y < 35; y++) {
		for(int x = 0; x < 35; x++) {
			snakeGrid[35*y + x] = (point) {120 + 16*(x), 36 + 16*(y)};
		}
	}
}

/*************************************************************
* getGridPos extracts the location out of the 2D array with
* 			all possible locations for the snake and food.
*
* @param	pos is the point of which to get the index in the
* 			2D array.
*
* @return
* 		- value that ranges [0 - 1225], which is an index in the
* 		2D array of all possible locations for the snake and food.
* 		- -1 otherwise
*
* @note		None.
*************************************************************/
int getGridPos(point pos) {
	for(int i = 0; i < 35*35; i++) {
		if(pos.x == snakeGrid[i].x && pos.y == snakeGrid[i].y) return i;
	}
	return -1;
}

/*************************************************************
* drawInstructions draws the controls for the snake.
*
* @param	textColor is the color of the text.
*
* @return	None.
*
* @note		None.
*************************************************************/
void drawInstructions(colors textColor) {
	point wBoxTL = {380, 260};
	point wBoxBR = {420, 300};

	point sBoxTL = {380, 300};
	point sBoxBR = {420, 340};

	point aBoxTL = {340, 300};
	point aBoxBR = {380, 340};

	point dBoxTL = {420, 300};
	point dBoxBR = {460, 340};

	//Box containers for letters
	drawBox(wBoxTL, wBoxBR, textColor);
	drawBox(sBoxTL, sBoxBR, textColor);
	drawBox(aBoxTL, aBoxBR, textColor);
	drawBox(dBoxTL, dBoxBR, textColor);

	//Letters
	drawText("w", (point) {wBoxTL.x+13, wBoxTL.y+4}, 2, textColor, black);
	drawText("s", (point) {sBoxTL.x+13, sBoxTL.y+4}, 2, textColor, black);
	drawText("a", (point) {aBoxTL.x+13, aBoxTL.y+4}, 2, textColor, black);
	drawText("d", (point) {dBoxTL.x+13, dBoxTL.y+4}, 2, textColor, black);
}

/*************************************************************
* clearInstructions clears the controls for the snake.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void clearInstructions(void) {
	drawInstructions(black);
}

/*************************************************************
* drawBody draws the body of the snake, which is smaller than
* 			the head.
*
* @param	pos is the position for where to draw the body.
* @param	color is the color of the body.
*
* @return	None.
*
* @note		4x4 pixels.
*************************************************************/
void drawBody(point pos, colors color) {
	point posTL = {pos.x+6, pos.y+6};
	point posBR = {pos.x+10, pos.y+10};
	drawBoxFull(posTL, posBR, color);
}

/*************************************************************
* drawHead draws the head of the snake, which is larger
* 			than the body.
*
* @param	None.
*
* @return	None.
*
* @note		12x12 pixels.
*************************************************************/
void drawHead(point pos, colors color) {
	point posTL = {pos.x+2, pos.y+2};
	point posBR = {pos.x+14, pos.y+14};
	drawBoxFull(posTL, posBR, color);
}

/*************************************************************
* initializeSnake sets a random starting point for the head
* 			of the snake. It sets the lenght to 1 and also
* 			an indeterminate direction, so the player specifies
* 			the direction for the snake to start moving.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void initializeSnake(void) {
	//Random starting point for the head of the snake.
	fullSnake.parts[0].pos = (point) {(rand()%560)/16*16 + 120, (rand()%560)/16*16 + 36};

	//Declare the length of the snake equal to 1.
	fullSnake.length = 1;
	//Initialize the direction of the head to 0, so the player chooses the direction.
	fullSnake.parts[0].direction = 0;
	//Set the food count to 0.
	foodCount = 0;

	for(int i = 1224; i >= 0; i--) {
		//Declare all non-head parts not grown
		if(i != 0) fullSnake.parts[i].grown = 0;
		//Declare the head part grown
		else fullSnake.parts[i].grown = 1;
	}
}

/*************************************************************
* growSnake lengthens the snake by one upon eating food.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void growSnake(void) {
	//Set the next part of snake to grown
	fullSnake.nextGrowablePart->grown = 1;
	//Increment the snake length
	fullSnake.length++;
	drawBody(fullSnake.parts[1].pos, green);
	//Decrement food count so the next food spawns
	foodCount--;
}

/*************************************************************
* moveSnake sets the position and direction of parts from tail
* 			to head and checks if the snake head's position is
* 			the same as the food's position.
*
* @param	dX is the change in the x coordinate for the snake.
* @param	dY is the change in the y coordinate for the snake.
*
* @return	None.
*
* @note		None.
*************************************************************/
void moveSnake(int dX, int dY) {

	for(int i = fullSnake.length; i >= 0; i--) {
		if(i != 0) {
			//Set the next part's direction to previous part's direction
			fullSnake.parts[i].direction = fullSnake.parts[i-1].direction;
			//Set the next part's position to previous part's position
			fullSnake.parts[i].pos = fullSnake.parts[i-1].pos;
			//Set the next growable part to the first not drawn body part
			if(i == fullSnake.length) fullSnake.nextGrowablePart = fullSnake.parts + i;
		} else {
			//Calculate the new position of the head part
			fullSnake.parts[i].pos = snakeGrid[getGridPos(fullSnake.parts[i].pos) + dX + dY*35];
			//Check if the head part is on food's position -> grow snake
			if(fullSnake.parts[i].pos.x == foodStack[0].pos.x && fullSnake.parts[i].pos.y == foodStack[0].pos.y) growSnake();
		}
	}
}

/*************************************************************
* updateSnake decides in which direction to move the snake and
* 			passes arguments accordingly.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void updateSnake(void) {
	//Choose the direction for the snake to move
	switch(fullSnake.parts[0].direction) {
	case 1:
		moveSnake(0, -1);
		break;
	case 2:
		moveSnake(-1, 0);
		break;
	case 3:
		moveSnake(0, 1);
		break;
	case 4:
		moveSnake(1, 0);
		break;
	}
}

/*************************************************************
* drawSnake draws parts of the snake.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void drawSnake(void) {
	for(int i = fullSnake.length-1; i >= 0; i--) {
		if(fullSnake.parts[i].grown == 1) {
			if(i != 0) drawBody(fullSnake.parts[i].pos, green);
			else drawHead(fullSnake.parts[i].pos, green);
		}
	}
}

/*************************************************************
* eraseSnake erases the snake's head part and the tail.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void eraseSnake(void) {
	drawHead(fullSnake.parts[0].pos, black);
	drawBody(fullSnake.parts[fullSnake.length-1].pos, black);
}

/*************************************************************
* checkBoundaries.
*
* @param	None.
*
* @return
* 		- 1 if over border,
* 		- 0 otherwise.
*
* @note		None.
*************************************************************/
int checkBoundaries(void) {
	//gameOver if going over edge
	if(fullSnake.parts[0].pos.x <= 120 && fullSnake.parts[0].direction == 2) return 1;
	if(fullSnake.parts[0].pos.x >= 664 && fullSnake.parts[0].direction == 4) return 1;
	if(fullSnake.parts[0].pos.y <= 36 && fullSnake.parts[0].direction == 1) return 1;
	if(fullSnake.parts[0].pos.y >= 580 && fullSnake.parts[0].direction == 3) return 1;

	return 0;
}

/*************************************************************
* gameOver.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void gameOver(void) {
	drawExtras();
	drawText("Game Over!", (point) {320, 284}, 2, white, red);
}

/*************************************************************
* spawnFood spawns food if the spawned one has been eaten, so
* 			there is always one food item on the screen.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void spawnFood(void) {
	struct snakeFood Food;
	point food = {(rand()%560)/16*16 + 120, (rand()%560)/16*16 + 36};
	Food.pos = food;
	Food.consumed = 0;

	while(fullSnake.parts[0].pos.x == Food.pos.x && fullSnake.parts[0].pos.y == Food.pos.y) {
		Food.pos = (point) {(rand()%560)/16*16 + 120, (rand()%560)/16*16 + 36};
	}

	if(foodCount == 0) {
		drawBoxFull((point) {Food.pos.x+7, Food.pos.y+7}, (point) {Food.pos.x+9, Food.pos.y+9}, red);
		foodStack[foodCount].pos = Food.pos;
		foodStack[foodCount].consumed = 0;
		foodCount++;
	}
}

/*************************************************************
* enterSnake first draws the header of the snake sub-program,
* 			draws instructions and deletes them after
* 			2 seconds. Then it draws the world borders for the
* 			snake game. Upon drawing everything it can begin by
* 			initializing the grid, the snake and spawning food.
* 			After that it follows a cycle of spawning food if
* 			eaten, getting input for the direction of the snake,
* 			drawing the snake, erasing the snake, updating snake
* 			location and it's body parts, and sleeping 125ms
* 			in-between cycles to control the snake's speed.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*************************************************************/
void enterSnake(void) {
	//Draw playing square
	point leftTop = {119, 35};
	point rightTop = {681, 35};
	point leftBot = {119, 597};
	point rightBot = {681, 597};

	drawStraight(leftTop, rightTop, red);
	drawStraight(leftTop, leftBot, red);
	drawStraight(leftBot, rightBot, red);
	drawStraight(rightTop, rightBot, red);

	drawInstructions(white);
	sleep(2);
	clearInstructions();

	initializeGrid();

	//Initialize snake head and stuff
	initializeSnake();

	//Draw the head
	drawHead(fullSnake.parts[0].pos, green);

	do {
		//Spawn the food
		spawnFood();

		switch(caughtChar) {
		case 'w':
			if(fullSnake.parts[0].direction != 3) fullSnake.parts[0].direction = 1;
			break;
		case 'a':
			if(fullSnake.parts[0].direction != 4) fullSnake.parts[0].direction = 2;
			break;
		case 's':
			if(fullSnake.parts[0].direction != 1) fullSnake.parts[0].direction = 3;
			break;
		case 'd':
			if(fullSnake.parts[0].direction != 2) fullSnake.parts[0].direction = 4;
			break;
		}

		//Draw the snake
		drawSnake();

		//Add delay between snake's movements
		usleep(125000);

		//Erase snake old body parts
		eraseSnake();

		//Check if snake has gone out of the playing field
		if(checkBoundaries() == 1) {
			gameOver();
			sleep(2);
			break;
		}

		//Update the coordinates of all the parts in the snake
		updateSnake();

	} while (caughtChar != 0x1B);
	clearVGA();
	drawStage();
}

/*************************************************************
* End of file
*************************************************************/
