#include "libs.h"

static XAxiDma AxiDma;				//Instance of XAxiDma driver
static INTC Intc;					//Interrupt controller instance
static XAxiDma_Config *CfgPtr;		//XAxiDma config pointer
static XScuGic_Config *IntcConfig;	//XScuGic config pointer
static controllers *components;

void dmaWrite(u8 srcAddr, u8 length) {
	// Reset the DMA
	XAxiDma_WriteReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_CR_OFFSET, XAXIDMA_CR_RESET_MASK);
	
	// Wait for the reset to complete
    while (XAxiDma_ReadReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_CR_OFFSET) & XAXIDMA_CR_RESET_MASK);

    // Set the source address
    XAxiDma_WriteReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_SRCADDR_OFFSET, src_addr);

    // Set the transfer length
    XAxiDma_WriteReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_BUFFLEN_OFFSET, length);

    // Start the DMA transfer
    XAxiDma_WriteReg(XPAR_AXI_DMA_0_BASEADDR, XAXIDMA_TX_OFFSET + XAXIDMA_CR_OFFSET, XAXIDMA_CR_RUNSTOP_MASK);
}

int main(void) {
	int Status;

	XUartPs UartPs;
	XUartPs_Config *Cfg;

	u8 text[] = "Hello world! (on my own)";

	Cfg = XUartPs_LookupConfig(UART_DEVICE_ID);

	if (Cfg == NULL) return XST_FAILURE;
	Status = XUartPs_CfgInitialize(&UartPs, Cfg, Cfg->BaseAddress);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	XUartPs_SetOperMode(&UartPs , XUARTPS_OPER_MODE_NORMAL);

	printf("Hey im printf!\n");

	for(int i = 0; i < (sizeof(text)-1);) {
		i += XUartPs_Send(&UartPs, &text[i], 1);
	}

	components->CfgPtr=CfgPtr;
	components->AxiDma=&AxiDma;
	components->IntcInstancePtr=&Intc;
	components->IntcConfig = IntcConfig;

	//Status = initInterrupt(components);

	u8 dataArray[4] = {0xFE, 0xFE, 0xFE, 0xFE};

	//Disable interrupts and return success
	//DisableIntrSystem(components->IntcInstancePtr);

	u8 srcAddr = dataArray;
	u8 length = 4;

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;
}
