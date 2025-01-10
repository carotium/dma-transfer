#include "libs.h"

static XAxiDma AxiDma;				//Instance of XAxiDma driver
static INTC Intc;					//Interrupt controller instance
static XAxiDma_Config *CfgPtr;		//XAxiDma config pointer
static XScuGic_Config *IntcConfig;	//XScuGic config pointer
static XUartPs UartPs;				//Instance of UartPs driver
static XUartPs_Config Cfg;			//XUartPs config pointer
//static controllers *components;		//Struct that holds driver information


int main(void) {
	int Status;

	u8 text[] = "Hello world! (on my own)";

	printf("Hey im printf!\n");

//	for(int i = 0; i < (sizeof(text)-1);) {
//		i += XUartPs_Send(components->UartPs, &text[i], 1);
//	}

	components->CfgPtr=CfgPtr;
	components->AxiDma=&AxiDma;
	components->IntcInstancePtr=&Intc;
	components->IntcConfig = IntcConfig;
	components->Cfg = &Cfg;
	components->UartPs = &UartPs;

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
	u32 dataArray2 = 0xABCDEFFE;
	u8 dataArray[8] = {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE};
	u32 srcAddr2 = (u32) dataArray;
	u32 srcAddr = (u32) dataArray2;
	u8 length = 8;

	//Xil_DCacheFlushRange((u8 *) dataArray, length);

	//dmaRead(srcAddr2, length, components);

	xil_printf("Press a key to exit");
	getChar(components->UartPs);
	xil_printf("--- Exiting main() --- \r\n");

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
