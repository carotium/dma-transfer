#include "libs.h"

static XAxiDma AxiDma;				//Instance of XAxiDma driver
static XAxiDma_Config *CfgPtr;		//XAxiDma config pointer
static INTC Intc;					//Interrupt controller instance
static XScuGic_Config *IntcConfig;	//XScuGic config pointer
static XUartPs UartPs;				//Instance of UartPs driver
static XUartPs_Config *Cfg;			//XUartPs config pointer
//static controllers *components;		//Struct that holds driver information -> defined as extern in libs.h


int main(void) {
	int Status;
	//Assembling the controllers struct for easier access to the underlying drivers

	//controllers *components = {(XAxiDma *) &AxiDma, CfgPtr,(INTC *) &Intc, IntcConfig,(XUartPs *) &UartPs, Cfg};

	components->AxiDma=&AxiDma;
	components->CfgPtr=CfgPtr;
	components->IntcInstancePtr=&Intc;
	components->IntcConfig = IntcConfig;
	components->UartPs = &UartPs;
	components->Cfg = Cfg;

	//Initialize the UART, DMA and Interrupts
	if(initUart(components) != XST_SUCCESS) {
		xil_printf("Initialization of UartPs failed :(\n\r");
		return XST_FAILURE;
	} else xil_printf("Initialization of UartPs done!\n\r");

	if(initDMA(components) != XST_SUCCESS) {
		xil_printf("Initialization of DMA failed :(\n\r");
		return XST_FAILURE;
	} 	else xil_printf("Initialization of DMA done!\n\r");

	if(initInterrupt(components) != XST_SUCCESS) {
		xil_printf("Initialization of interrupts failed.\n\r");
		return XST_FAILURE;
	} else xil_printf("Initialization of interrupts done!\r\n");

	//Disable interrupts and return success
	//DisableIntrSystem(components->IntcInstancePtr);

	u32 arrayLength = 512;

	for(u32 i = 0; i < arrayLength; i++) {
		if((i%128) < 16) data_dma_to_vga[i] = (i%16);					//RED 0-15
		else if((i%128) < 32) data_dma_to_vga[i] = (i%16) << 8;		//GREEN 0-15
		else if((i%128) < 48) data_dma_to_vga[i] = (i%16) << 16;		//BLUE 0-15
		else if((i%128) < 64) data_dma_to_vga[i] = ((i%16) << 8) + (i%16);	//RED + GREEN 0-15
		else if((i%128) < 80) data_dma_to_vga[i] = ((i%16) << 16) + ((i%16)<<8);//GREEN + BLUE 0-15
		else if((i%128) < 96) data_dma_to_vga[i] = ((i%16) << 16) + (i%16);	//BLUE + RED 0-15
		else if((i%128) < 112) data_dma_to_vga[i] = ((i%16) << 16) + ((i%16) << 8) + (i%16);	//R+G+B 0-15
		else if((i%128) < 128) data_dma_to_vga[i] = ((i%16) << 16) + ((i%16) << 8) + (i%16);	//R+G+B 0-15
	}

	//Xil_DCacheFlushRange((u8 *) dataArray, length);
	Xil_DCacheFlushRange((u32 *) (data_dma_to_vga), 512*4);

	xil_printf("Press a key to exit");
	getChar(components->UartPs);
	xil_printf("\n\rHappy DMA-ing\n\r");
	enableInterrupts(components);
	while(1)
	{
//		static u32 i = 0;
//		dmaReadReg((data_dma_to_vga), 512, components);
//		dmaRead(data_dma_to_vga+(i%256), length, components);
//		Xil_DCacheFlushRange((INTPTR) (data_dma_to_vga+(i%256)), length);
//		i++;
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