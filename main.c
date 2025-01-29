#include "libs.h"

static XAxiDma AxiDma;				//Instance of XAxiDma driver
static XAxiDma_Config *CfgPtr;		//XAxiDma config pointer
static INTC Intc;					//Interrupt controller instance
static XScuGic_Config *IntcConfig;	//XScuGic config pointer
static XUartPs UartPs;				//Instance of UartPs driver
static XUartPs_Config *Cfg;			//XUartPs config pointer

int main(void) {
//	int Status;

	//Assembling the controllers structure for easier access to the underlying drivers
	components = &(controllers){&AxiDma, CfgPtr, &Intc, IntcConfig, &UartPs, Cfg};

	//Initialize the UART, DMA and Interrupts
	if(initPlatform(components) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	static u32 arrayLength = 512;

	for(u32 i = 0; i < arrayLength; i++) {
		if((i) < 16) data_dma_to_vga[i] = (i%16);										//RED 0-15
		else if((i) < 32) data_dma_to_vga[i] = (i%16) << 8;								//GREEN 0-15
		else if((i) < 48) data_dma_to_vga[i] = (i%16) << 16;							//BLUE 0-15
		else if(i < 64) data_dma_to_vga[i] = ((i%16) << 8) + (i%16);					//RED + GREEN 0-15
		else if(i < 80) data_dma_to_vga[i] = ((i%16) << 16) + ((i%16)<<8);				//GREEN + BLUE 0-15
		else if(i < 96) data_dma_to_vga[i] = ((i%16) << 16) + (i%16);					//BLUE + RED 0-15
		else if(i < 112) data_dma_to_vga[i] = ((i%16) << 16) + ((i%16) << 8) + (i%16);	//R+G+B 0-15
		else if(i < 128) data_dma_to_vga[i] = ((i%16) << 16) + ((i%16) << 8) + (i%16);	//R+G+B 0-15
		else if(i < 256) data_dma_to_vga[i] = i/2;
		else data_dma_to_vga[i] = i<<8;
	}

	for(u32 y = 0; y < arrayLength/2; y++) {
		for(u32 x = 0; x < arrayLength/2; x++) {
			dataArray[y][x] = data_dma_to_vga[y%128];
//			dataArray[y][x] = data_dma_to_vga[x%128];
		}
	}

	//Flushing cache, so the DMA transmits defined data
	Xil_DCacheFlushRange((INTPTR) (data_dma_to_vga), 512*4);
	Xil_DCacheFlushRange((INTPTR) dataArray, 256*256*4);

	xil_printf("Press a key to exit");
	getChar(components->UartPs);
	xil_printf("\n\rHappy DMA-ing\n\r");
	//Enable the interrupts
	enableInterrupts(components);
	while(1)
	{
		//Something
		//Xil_WaitForEventSet();
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
