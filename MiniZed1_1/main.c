/**************************************************************
* File: main.c
* Description: Main program for MiniZed board.
*
* Author: Ahac Rafael Bela
* Created on: 01.03.2025
* Last modified: 04.03.2025
*************************************************************/

/**************************************************************
* Include section
*************************************************************/
#include "libs.h"
#include "vga.h"

/**************************************************************
* Global variables
*************************************************************/
static XAxiDma AxiDma; 				/* Instance of the XAxiDma */
static XAxiDma_Config *CfgPtr; 		/* Pointer to configuration data */
static INTC Intc; 					/* Instance of the Interrupt Controller */
static XScuGic_Config *IntcConfig; 	/* Instance of the Interrupt Controller configuration */
static XUartPs UartPs; 			/* Instance of the UART */
static XUartPs_Config *Cfg; 		/* Pointer to UART configuration data */

selectorWText selectorWText1;
selectorWText selectorWText2;
selectorWText selectorWText3;
selectorWText selectorWText4;

u32 discovered = 0;

//extern u32 vgaArray[SCREEN_HEIGHT][SCREEN_WIDTH];

int main(void) {

    //Assembling the controllers structure for easier access to the underlying drivers
	ctrls = &(controllers){&AxiDma, CfgPtr, &Intc, IntcConfig, &UartPs, Cfg};

    //Initialize the UART, DMA and Interrupts
	if(initPlatform(ctrls) != XST_SUCCESS) return XST_FAILURE;

	//Flushing cache, so the DMA transmits defined data
	Xil_DCacheFlushRange((INTPTR) vgaArray, 640*480*4);

//	xil_printf("Press a key to continue\n\r");
	//A stopping point to wait for user input
//	getChar(ctrls->UartPs);
	xil_printf("\n\rHappy DMA-ing\n\r");

	//Enable the interrupts
	enableInterrupts(ctrls);

    point selector1 = {208, 200};
	point selector2 = {208, 238};
	point selector3 = {208, 276};
	point selector4 = {208, 314};

	const char *menuText1 = "Echo";
	const char *menuText2 = "Lines";
	const char *menuText3 = "Exit";
	const char *menuText4 = "Extras";


	point textPos1 = {288, 203};
	point textPos2 = {280, 241};
	point textPos3 = {288, 279};
	point textPos4 = {272, 317};

	selectorWText1 = (selectorWText){{selector1.x, selector1.y}, textPos1, menuText1};
	selectorWText2 = (selectorWText){{selector2.x, selector2.y}, textPos2, menuText2};
	selectorWText3 = (selectorWText){{selector3.x, selector3.y}, textPos3, menuText3};
	selectorWText4 = (selectorWText){{selector4.x, selector4.y}, textPos4, menuText4};

	drawStraight((point) {0, 0}, (point) {0, 479}, red);
	drawStraight((point) {0, 0}, (point) {639, 0}, red);
	drawStraight((point) {0, 479}, (point) {639, 479}, red);
	drawStraight((point) {639, 0}, (point) {639, 479}, red);

	//First box is selected on startup
	static int selected = 1;

	//Starting menu with selectors
	drawStage(0);

	XUartPs_Recv(ctrls->UartPs, (u8 *) &caughtChar, 1);

	caughtChar = '\0';
//	nextLine(1);
//	scale = 1;
	while(1) {
//		char caughtChar = getChar(ctrls->UartPs);
//		sleep(1);
		if(receivedCount == 1) {
			receivedCount--;
			if(caughtChar == 'w') {
				//Up arrow
				if(selected > 1) {
					selected--;
					discovered = 0;
				}
			}
			else if(caughtChar == 's') {
				//Down arrow
				if(selected < 3) {
					selected++;
				}
			} else if(caughtChar == 'S' && selected == 3) {
				selected++;
				discovered = 1;
			}

			selectorWText menuText;
			switch(selected) {
			case 1:
				menuText = selectorWText1;
				break;
			case 2:
				menuText = selectorWText2;
				break;
			case 3:
				menuText = selectorWText3;
				break;
			case 4:
				menuText = selectorWText4;
			}

			selectMenu(menuText);

			//Pressing enter, so entering echo, lines or exit
			if(caughtChar == 0xD) {
	//			drawStage(selected);
				enterMenu(menuText);
			}
		}

//		if(caughtChar == 0xD) {
//			enterMenu(menuText);
//		}
		//Print to VGA from UART input
//		printLetter(getLetter(getChar(components->UartPs), scale), scale, white);

/************************************
*******	Line drawing algorithm ******
*************************************
		static volatile u16 i = 0;

		static volatile u32 t = 0;

		if(i == 65534) {
			static int j = 0;
			if(j == 0) {
				drawLines(t%256);
				j += 1;

				if(t < 255) t++;
				else t = 0;
			} else j--;
		}

		if(i < 65534) i++;
		else i = 0;
************************************/
	}
	//Safety while loop
	while(1);

	return XST_SUCCESS;
}
