/*************************************************************
* File: libs.c
* Description: MiniZed board DMA, GIC, UART configuration.
* Other interrupt functions for DMA, UART receive, VGA synchronization signals.
*
* Author: Ahac Rafael Bela
* Created on: 01.03.2025
* Last modified: 14.04.2025
*************************************************************/

/*************************************************************
* Include section
*************************************************************/
#include "libs.h"

/*************************************************************
* Global variable section
*************************************************************/
controllers *ctrls;
volatile u8 caughtChar;
volatile u8 receivedCount = 0;

static volatile s32 lineIndex = 0;

/*************************************************************
* Function definition section
*************************************************************/

/*************************************************************
* initPlatform initializes the platform. It initializes UART, DMA and interrupts.
*
* @param	ctrls is a pointer to the controllers structure which
* 			holds necessary configuration and instance variables
* 			for initialization.
*
* @return
* 			- XST_SUCCESS if successful,
* 			- XST_FAILURE otherwise.
*
* @note		None.
*************************************************************/
int initPlatform(controllers *ctrls) {
	int Status;

	//Initialize UART
	Status = initUART(ctrls);
	if(Status != XST_SUCCESS) {
		xil_printf("Initialization of UartPs failed :(\n\r");
		return XST_FAILURE;
	} else xil_printf("Initialization of UartPs done!\n\r");

	//Initialize DMA
	Status = initDMA(ctrls);
	if(Status != XST_SUCCESS) {
		xil_printf("Initialization of DMA failed :(\n\r");
		return XST_FAILURE;
	} 	else xil_printf("Initialization of DMA done!\n\r");

	//Initialize interrupts
	Status = initInterrupt(ctrls);
	if(Status != XST_SUCCESS) {
		xil_printf("Initialization of interrupts failed.\n\r");
		return XST_FAILURE;
	} else xil_printf("Initialization of interrupts done!\r\n");

	return Status;
}

/*************************************************************
* initUart initializes the UART.
*
* @param	ctrls is a pointer to the controllers structure which
* 			holds necessary configuration and instance variables
* 			for initialization.
*
* @return
* 			- XST_SUCCESS if successful,
* 			- XST_FAILURE otherwise.
*
* @note		None.
*************************************************************/
int initUART(controllers *ctrls) {
	int Status;

	xil_printf("\r\nInitializing Uart...\r\n");
	//Get the UART configuration
	ctrls->Cfg = XUartPs_LookupConfig(UART_DEV_ID);
	if (ctrls->Cfg == NULL) return XST_FAILURE;
	//Initialize UART
	Status = XUartPs_CfgInitialize(ctrls->UartPs, ctrls->Cfg, ctrls->Cfg->BaseAddress);		
	if (Status != XST_SUCCESS) return XST_FAILURE;
	//Set the baud rate
	XUartPs_SetBaudRate(ctrls->UartPs, 115200);		
	//Set UART in normal mode
	XUartPs_SetOperMode(ctrls->UartPs , XUARTPS_OPER_MODE_NORMAL); 	
	//Set the interrupt mask to RX Trigger
    XUartPs_SetInterruptMask(ctrls->UartPs, XUARTPS_IXR_RXOVR | XUARTPS_IXR_MASK);
    //Set the custom handler for the interrupt
    XUartPs_SetHandler(ctrls->UartPs, (XUartPs_Handler) UartPsIntrHandler, ctrls->UartPs);
    //Set the threshold for the interrupt to 1 byte aka 1 character
    XUartPs_SetFifoThreshold(ctrls->UartPs, 1);

	return Status;
}

/*************************************************************
* initDMA initializes the DMA.
*
* @param	ctrls is a pointer to the controllers structure which
* 			holds necessary configuration and instance variables
* 			for initialization.
*
* @return
* 			- XST_SUCCESS if successful,
* 			- XST_FAILURE otherwise.
*
* @note		None.
*************************************************************/
int initDMA(controllers *ctrls) {
	int Status;

	xil_printf("\r\nInitializing DMA...\r\n");
	//Get the DMA configuration
	ctrls->CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
	if(!(ctrls->CfgPtr)) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);
		return XST_FAILURE;
	}
	//Initialize DMA
	Status = XAxiDma_CfgInitialize(ctrls->AxiDma, ctrls->CfgPtr);
	if(Status != XST_SUCCESS) {xil_printf("Initialization failed"); return XST_FAILURE;}
	if(XAxiDma_HasSg(ctrls->AxiDma)) {xil_printf("Device configured as SG mode \r\n"); return XST_FAILURE;}

	return Status;
}

/*************************************************************
* initInterrupt initializes MM2S, HSync and UART interrupts.
*
* @param	ctrls is a pointer to the controllers structure which
* 			holds necessary configuration and instance variables
* 			for initialization.
*
* @return
* 			- XST_SUCCESS if successful,
* 			- XST_FAILURE otherwise.
*
* @note		None.
*************************************************************/
int initInterrupt(controllers *ctrls) {
	int Status;

	xil_printf("\r\nInitializing interrupts...\r\n");

	//Get the interrupt controller configuration
	ctrls->IntcConfig = XScuGic_LookupConfig(INTC_DEV_ID);
	if(NULL == ctrls->CfgPtr) return XST_FAILURE;
	//Initialize the interrupt controller
	Status = XScuGic_CfgInitialize(ctrls->IntcInstancePtr, ctrls->IntcConfig, (ctrls->IntcConfig)->CpuBaseAddress);		
	if(Status != XST_SUCCESS) return XST_FAILURE;

	//Perform a self-test.
	Status = XScuGic_SelfTest(ctrls->IntcInstancePtr);
	if(Status != XST_SUCCESS) {xil_printf("Failed interrupt setup\r\n"); return XST_FAILURE;}

	//Set priority for connected interrupts (0 is highest, 0xF8 is highest, with 0x8 increments)
    XScuGic_SetPriorityTriggerType(ctrls->IntcInstancePtr, HSYNC_INTR_ID, 0xA0, 0x3);
    XScuGic_SetPriorityTriggerType(ctrls->IntcInstancePtr, UART_INTR_ID, 0xA8, 0x3);
    XScuGic_SetPriorityTriggerType(ctrls->IntcInstancePtr, VSYNC_INTR_ID, 0x98, 0x3);

	//Connect interrupts to their corresponding handlers
	Status = XScuGic_Connect(ctrls->IntcInstancePtr, HSYNC_INTR_ID, (Xil_InterruptHandler) HSyncIntrHandler, ctrls->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	Status = XScuGic_Connect(ctrls->IntcInstancePtr, VSYNC_INTR_ID, (Xil_InterruptHandler) VSyncIntrHandler, ctrls->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	Status = XScuGic_Connect(ctrls->IntcInstancePtr, UART_INTR_ID, (Xil_InterruptHandler) XUartPs_InterruptHandler, ctrls->UartPs);
	if(Status != XST_SUCCESS) return XST_FAILURE;

	//Disable all DMA interrupts before setup
	XAxiDma_IntrDisable(ctrls->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Enable IOC (interrupt on completion) for DMA to device (DMA read/MM2S)
	XAxiDma_IntrEnable(ctrls->AxiDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);


	//Enable interrupts from GIC to PS
    //Xil_ExceptionInit();	this function does nothing!
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) INTC_HANDLER,(void *)ctrls->IntcInstancePtr);
	//Enable interrupts in ARM
    Xil_ExceptionEnable();

	return Status;
}

/*************************************************************
* enableInterrupts enables interrupts.
*
* @param	ctrls is a pointer to the controllers structure which
* 			holds necessary configuration and instance variables
* 			for initialization.
*
* @return	None.
*
* @note		None.
*************************************************************/
void enableInterrupts(controllers *ctrls) {
    XScuGic_Enable(ctrls->IntcInstancePtr, VSYNC_INTR_ID);
	XScuGic_Enable(ctrls->IntcInstancePtr, HSYNC_INTR_ID);
	XScuGic_Enable(ctrls->IntcInstancePtr, UART_INTR_ID);
}

/*************************************************************
* dmaReadReg sets the appropriate DMA registers for a read operation MM2S.
*
* @param	srcAddr is the source address from where to read.
* @param	length is the number of bytes to read.
* @param	ctrls is a pointer to the controllers structure which
* 			holds necessary configuration and instance variables
* 			for initialization.
*
* @return
* 			- XST_SUCCESS if successful,
* 			- XST_FAILURE otherwise.
*
* @note		None.
*************************************************************/
int dmaReadReg(u32 *srcAddr, u32 length, controllers *ctrls) {
	//Setting DMA MM2S run/stop bit to 1
	Xil_Out32((ctrls->CfgPtr->BaseAddr + XAXIDMA_CR_OFFSET), XAXIDMA_CR_RUNSTOP_MASK);
	//Write a valid source address to the MM2S_SA register
	Xil_Out32((UINTPTR) (ctrls->CfgPtr->BaseAddr + XAXIDMA_SRCADDR_OFFSET), (u32) srcAddr);
	//Write the number of bytes to transfer -> this starts the DMA transaction
	Xil_Out32((UINTPTR) (ctrls->CfgPtr->BaseAddr + XAXIDMA_BUFFLEN_OFFSET), length * 4);

	//Check if transmit is (not) done -> usleep
	//instead of waiting in this function an interrupt will be called when transfer is complete
	if(!(Xil_In32(ctrls->CfgPtr->BaseAddr + XAXIDMA_SR_OFFSET) & 0x2)) {
		usleep(1);
	}
	return XST_SUCCESS;
}

/*************************************************************
* HsyncIntrHandler is Hsync interrupt handler.
*
* @param	Callback is a pointer to the caller, in this case
* 			to the interrupt controller.
*
* @return	None.
*
* @note		None.
*************************************************************/
void HSyncIntrHandler(void *Callback) {
	//Disable the interrupt
	XScuGic_Disable(ctrls->IntcInstancePtr, HSYNC_INTR_ID);

	//Do some data transfer
	dmaReadReg(vgaArray[lineIndex], SCREEN_WIDTH, ctrls);
	Xil_DCacheFlushRange((INTPTR) vgaArray[(lineIndex+1)%SCREEN_HEIGHT], SCREEN_WIDTH*4);

	//Sending 600 lines, then starting over
	if(lineIndex<(SCREEN_HEIGHT - 1))lineIndex++;

	//End of data transfer, enable the interrupt
	XScuGic_Enable(ctrls->IntcInstancePtr, HSYNC_INTR_ID);
}

/*************************************************************
* VsyncIntrHandler is Vsync interrupt handler.
*
* @param	Callback is a pointer to the caller, in this case
* 			to the interrupt controller.
*
* @return	None.
*
* @note		None.
*************************************************************/
void VSyncIntrHandler(void *Callback) {

	XScuGic_Disable(ctrls->IntcInstancePtr, VSYNC_INTR_ID);

 	//Reset the line index
 	lineIndex = -28;

 	XScuGic_Enable(ctrls->IntcInstancePtr, VSYNC_INTR_ID);
}

/*************************************************************
* UartPsIntrHandler is UART on receive interrupt handler.
*
* @param	Callback is a pointer to the caller, in this case
* 			to the UART controller.
*
* @return	None.
*
* @note		None.
*************************************************************/
void UartPsIntrHandler(void *CallBackRef, u32 Event, u32 EventData) {

	XUartPs *UartInstPtr = (XUartPs *)CallBackRef;

	if(Event == XUARTPS_EVENT_RECV_DATA) {
		receivedCount = EventData;
		//Run the XUartPs_Recv so it waits for the next byte to be sent over UART
		XUartPs_Recv(UartInstPtr, (u8 *) &caughtChar, 1);
	}
}

/*************************************************************
* End of file
*************************************************************/
