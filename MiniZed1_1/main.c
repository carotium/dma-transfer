/*************************************************************
* File: main.c
* Description: Main program for MiniZed board.
*
* Author: Ahac Rafael Bela
* Created on: 01.03.2025
* Last modified: 10.04.2025
*************************************************************/

/*************************************************************
* Include section
*************************************************************/
#include "libs.h"
#include "vga.h"
#include "lines.h"

/*************************************************************
* Global variable section
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

/*************************************************************
* Main function section
*************************************************************/
int main(void) {

    //Assembling the controllers structure for easier access to the underlying drivers
	ctrls = &(controllers){&AxiDma, CfgPtr, &Intc, IntcConfig, &UartPs, Cfg};

    //Initialize the UART, DMA and Interrupts
	if(initPlatform(ctrls) != XST_SUCCESS) return XST_FAILURE;

	//Flushing cache, so the DMA transmits defined data
	Xil_DCacheFlushRange((INTPTR) vgaArray, SCREEN_HEIGHT*SCREEN_WIDTH*sizeof(u32));

	xil_printf("\n\rHappy DMA-ing\n\r");

	//Enable the interrupts
	enableInterrupts(ctrls);

	//Setting selectorWText selector' and text' coordinates
	selectorWText1 = (selectorWText){{SELECTOR_X, SELECTOR_Y(1)},
					{SCREEN_WIDTH / 2 - strlen("Echo") * CHAR_WIDTH, (SELECTOR_Y(1) + SELECTOR_PADDING)},
					"Echo"};
	selectorWText2 = (selectorWText){{SELECTOR_X, SELECTOR_Y(2)},
					{SCREEN_WIDTH / 2 - strlen("Lines") * CHAR_WIDTH, (SELECTOR_Y(2) + SELECTOR_PADDING)},
					"Lines"};
	selectorWText3 = (selectorWText){{SELECTOR_X, SELECTOR_Y(3)},
					{SCREEN_WIDTH / 2 - strlen("Exit") * CHAR_WIDTH, (SELECTOR_Y(3) + SELECTOR_PADDING)},
					"Exit"};
	selectorWText4 = (selectorWText){{SELECTOR_X, SELECTOR_Y(4)},
					{SCREEN_WIDTH / 2 - strlen("Extras") * CHAR_WIDTH, (SELECTOR_Y(4) + SELECTOR_PADDING)},
					"Extras"};

	//First box is selected on startup
	static int selected = 1;

	//Starting menu with selectors
	drawStage();

	//First call to the receive function of UART
	XUartPs_Recv(ctrls->UartPs, (u8 *) &caughtChar, 1);

	//Initializing the value to a null character
	caughtChar = '\0';

	//Sets the speed of moving lines to a random number (1 - 6)
	initializeLines();

	//Main while loop
	while(1) {
		//If an interrupt function has caught a character from keyboard input
		if(receivedCount == 1) {
			receivedCount--;
			//Equivalent to the up arrow
			if(caughtChar == 'w') {
				if(selected > 1) {
					selected--;
					discovered = 0;
				}
			}
			//Equivalent to the down arrow
			else if(caughtChar == 's') {
				if(selected < 3) {
					selected++;
				}
			}
			//An extras menu hidden behind a capital S, instead of small s
			else if(caughtChar == 'S' && selected == 3) {
				selected++;
				discovered = 1;
			}

			//Selecting of different menus, so they light up when going through them
			selectorWText selectedMenu;
			switch(selected) {
			case 1:
				selectedMenu = selectorWText1;
				break;
			case 2:
				selectedMenu = selectorWText2;
				break;
			case 3:
				selectedMenu = selectorWText3;
				break;
			case 4:
				selectedMenu = selectorWText4;
			}

			//Highlights the selected navigation menu
			selectMenu(selectedMenu);

			//Enter pressed, entering echo, lines or exit (+ extras)
			if(caughtChar == 0xD) {
				enterMenu(selectedMenu);
			}
		}
	}

	//Safety while loop
	while(1);

	return XST_SUCCESS;
}

/*************************************************************
* End of file
*************************************************************/
