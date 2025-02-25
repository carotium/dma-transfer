#include "libs.h"

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



int main(void) {

	//Assembling the controllers structure for easier access to the underlying drivers
	components = &(controllers){&AxiDma, CfgPtr, &Intc, IntcConfig, &UartPs, Cfg};

	//Initialize the UART, DMA and Interrupts
	if(initPlatform(components) != XST_SUCCESS) return XST_FAILURE;

	//Flushing cache, so the DMA transmits defined data
	Xil_DCacheFlushRange((INTPTR) dataArray, 640*480*4);

	static colors color;

//	for(int i = 0; i < 256; i++) {
//
//		switch(i%15) {
//		case 0:  color = blue; break;
//		case 1:  color = green; break;
//		case 2:  color = cyan; break;
//		case 3:  color = red; break;
//		case 4:  color = purple; break;
//		case 5:  color = brown; break;
//		case 6:  color = gray; break;
//		case 7:  color = d_gray; break;
//		case 8:  color = l_blue; break;
//		case 9:  color = l_green; break;
//		case 10: color = l_cyan; break;
//		case 11: color = l_red; break;
//		case 12: color = l_purple; break;
//		case 13: color = yellow; break;
//		case 14: color = white; break;
//		default: break;
//		}
//		color = white;
//
//		printLetter(getLetter(i), color);
//	}

	xil_printf("Press a key to continue\n\r");
	getChar(components->UartPs);
	xil_printf("\n\rHappy DMA-ing\n\r");

	//Enable the interrupts
	enableInterrupts(components);

	//Setting starting line parameters
	lineStart(0);

	printVGA("\nWhat can you say, I'm a decent programmer\t", white);

	while(1)
	{
		//Print to VGA from UART input
		printLetter(getLetter(getChar(components->UartPs)), white);

/******	Line drawing algorithm ******
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
	while(1);

/*
	// Set the RS bit in CR register
	XAxiDma_WriteReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_CR_OFFSET, XAXIDMA_CR_RUNSTOP_MASK);
	// Reset the DMA
	XAxiDma_WriteReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_CR_OFFSET, XAXIDMA_CR_RESET_MASK);

	// Wait for the reset to complete
	while (XAxiDma_ReadReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_CR_OFFSET) & XAXIDMA_CR_RESET_MASK);

	// Set the source address
	XAxiDma_WriteReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_SRCADDR_OFFSET, &srcAddr);

	// Setting the transfer length starts the transaction
	XAxiDma_WriteReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_BUFFLEN_OFFSET, length);
*/
	return XST_SUCCESS;
}
