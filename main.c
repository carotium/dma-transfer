#include "libs.h"

//Test numero due
static XAxiDma AxiDma;				//Instance of XAxiDma driver
static INTC Intc;					//Interrupt controller instance
static XAxiDma_Config *CfgPtr;		//XAxiDma config pointer
static XScuGic_Config *IntcConfig;	//XScuGic config pointer
static controllers *components;

//Our type, we assign it in the main function
//static controllers *components;

int main(void) {
	int Status;

	XUartPs UartPs;
	XUartPs_Config *Cfg;

	u8 text[] = "Hello world! (on my own)";

	Cfg = XUartPs_LookupConfig(UART_DEVICE_ID);

	if (Cfg == NULL) return XST_FAILURE;
	Status = XUartPs_CfgInitialize(&UartPs, Cfg, Cfg->BaseAddress);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	/*Status = XUartPs_SetBaudRate(&UartPs, 115200);
	if (Status != XST_SUCCESS) return XST_FAILURE;*/

	XUartPs_SetOperMode(&UartPs , XUARTPS_OPER_MODE_NORMAL);

	//XUartPs_();

	printf("Hey im printf!\n");

	for(int i = 0; i < (sizeof(text)-1);) {
		i += XUartPs_Send(&UartPs, &text[i], 1);
	}
	//printf("\nnova vrstica\r\nnova vrstica IN return");


	//XUartPs_SetOperMode(&UartPs , XUARTPS_OPER_MODE_AUTO_ECHO);



	components->CfgPtr=CfgPtr;
	components->AxiDma=&AxiDma;
	components->IntcInstancePtr=&Intc;
	components->IntcConfig = IntcConfig;

	Status = initInterrupt(components);

	//char sken[10];



	u8 dataArray[4] = {0xFE, 0xFE, 0xFE, 0xFE};

	//Status = LoadTx((u8 *)TX_BUFFER_BASE, (u8 *) *dataArray, 4);

	Status = ReadRx(dataArray, 4);
	xil_printf("dataArray buffer\r\n");
	if(Status != XST_SUCCESS) return XST_FAILURE;

	Status = ReadRx((u8 *)RX_BUFFER_BASE, 4);
	xil_printf("dataArray buffer pre DMA\r\n");
	if(Status != XST_SUCCESS) return XST_FAILURE;

//	Status = XAxiDma_Send_Array(components->AxiDma, (UINTPTR) TX_BUFFER_BASE, (UINTPTR) RX_BUFFER_BASE, 3);
//	if(Status != XST_SUCCESS) return XST_FAILURE;

	//Load values to DMA base address so he doesn't send gibberish
//	Status = LoadTx((u8 *)XPAR_AXIDMA_0_BASEADDR, (u8 *) TX_BUFFER_BASE, 4);

	Status = dmaSend();

	Status = XAxiDma_SimpleTransfer(components->AxiDma, (UINTPTR) RX_BUFFER_BASE, 4, XAXIDMA_DMA_TO_DEVICE);
	if(Status != XST_SUCCESS) return XST_FAILURE;

	Status = ReadRx((u8 *)RX_BUFFER_BASE, 4);
	xil_printf("dataArray buffer after DMA transfer\r\n");
	if(Status != XST_SUCCESS) return XST_FAILURE;

	xil_printf("Successfully ran first part of AXI DMA interrupt Example\r\n");

	//Disable interrupts and return success
	DisableIntrSystem(components->IntcInstancePtr);

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;
}
