#include "libs.h"

/***************************************
 * Config and instance variables for DMA, Interrupt controller and UART
 ***************************************/
static XAxiDma AxiDma;				//Instance of XAxiDma driver
static XAxiDma_Config *CfgPtr;		//XAxiDma config pointer
static INTC Intc;					//Interrupt controller instance
static XScuGic_Config *IntcConfig;	//XScuGic config pointer
static XUartPs UartPs;				//Instance of UartPs driver
static XUartPs_Config *Cfg;			//XUartPs config pointer

/***************************************
 * Line drawing variables
 ***************************************/
//Array of drawn lines' coordinates (256 lines on the screen)
u32 startX[256], endX[256], startY[256], endY[256];
//Speed of line's coordinates
s32 dx0, dx1, dy0, dy1;
//Color palette, 16 colors ATM
u32 colorPalette[16] = {
		0xc3a2c3,
		0xc8634c,
		0xcd23d4,
		0xd1e45c,
		0xd6a4e5,
		0xdf3d5e,
		0xe7d5d6,
		0xf06e4f,
		0xf906c8,
		0xf8fa99,
		0xf8ee6a,
		0xf8e23b,
		0xf8d60c,
		0xcffb11,
		0xa72016,
		0x7e451a
};

//Scaling variable of the font
static u32 scale = 2;

selectorWText selectorWText1;
selectorWText selectorWText2;
selectorWText selectorWText3;
selectorWText selectorWText4;

u32 discovered = 0;

XTime timerS, timerE;
double timeUsed;

int main(void) {

	//Assembling the controllers structure for easier access to the underlying drivers
	components = &(controllers){&AxiDma, CfgPtr, &Intc, IntcConfig, &UartPs, Cfg};

	//Initialize the UART, DMA and Interrupts
	if(initPlatform(components) != XST_SUCCESS) return XST_FAILURE;

	//Flushing cache, so the DMA transmits defined data
	Xil_DCacheFlushRange((INTPTR) dataArray, 640*480*4);

//	xil_printf("Press a key to continue\n\r");
	//A stopping point to wait for user input
//	getChar(components->UartPs);
	xil_printf("\n\rHappy DMA-ing\n\r");

	//Enable the interrupts
	enableInterrupts(components);

	//Setting starting line parameters
	lineStart(0);

	drawText("MiniZed 1.0", (point){2, 2}, 4, white, d_gray);

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

	drawStraight(0, 0, 0, 479, red);
	drawStraight(0, 0, 639, 0, red);
	drawStraight(0, 479, 639, 479, red);
	drawStraight(639, 0, 639, 479, red);

	//First box is selected on startup
	static int selected = 1;
	drawSelector(selector1, white);
	drawSelector(selector2, d_gray);
	drawSelector(selector3, d_gray);
//	drawSelector(selector4, l_red);

	drawText(selectorWText1.menuText, selectorWText1.menu, 2, white, black);
	drawText(selectorWText2.menuText, selectorWText2.menu, 2, d_gray, black);
	drawText(selectorWText3.menuText, selectorWText3.menu, 2, d_gray, black);
//	drawText(selectorWText4.menuText, selectorWText4.menu, 2, l_red, black);

//	drawLineB(64, 64, 575, 415, l_blue);

//	XTime_GetTime(&tStart);
//	printVGA("B", 1, cyan);
//	XTime_GetTime(&tEnd);

//	printf("printVGA took %llu clock cycles.\n", 2*(tEnd - tStart));
//	printf("printVGA took %.2f us.\n",
//			1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000));

	nextLine(1);
	scale = 1;
	while(1)
	{
		char caughtChar = getChar(components->UartPs);

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

		if(caughtChar == 0xD) {
			enterMenu(menuText);
		}
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
