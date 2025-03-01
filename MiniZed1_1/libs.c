/**************************************************************
* File: libs.c
* Description: MiniZed board DMA, GIC, UART configuration.
* Other miscellaneous functions.
*
* Author: Ahac Rafael Bela
* Created on: 01.03.2025
* Last modified: 01.03.2025
*************************************************************/

/**************************************************************
* Include section
*************************************************************/
#include "libs.h"

/**************************************************************
* Variable definitions
*************************************************************/
controllers *ctrls;

static volatile s32 lineIndex = 0;

/**************************************************************
* Function definitions
*************************************************************/

/**************************************************************
* initPlatform initializes the platform. It initializes UART, DMA and interrupts.
*
* @param	ctrls is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
*
* @return
* 		- XST_SUCCESS if successful,
* 		- XST_FAILURE otherwise.
*
* @note	None.
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

/**************************************************************
* initUart initializes the UART.
*
* @param	ctrls is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
*
* @return
* 		- XST_SUCCESS if successful,
* 		- XST_FAILURE otherwise.
*
* @note	None.
*************************************************************/
int initUART(controllers *ctrls) {
	int Status;

	xil_printf("\r\nInitializing Uart...\r\n");
	//Get the UART configuration
	ctrls->Cfg = XUartPs_LookupConfig(UART_DEVICE_ID);		
	if (ctrls->Cfg == NULL) return XST_FAILURE;
	//Initialize UART
	Status = XUartPs_CfgInitialize(ctrls->UartPs, ctrls->Cfg, ctrls->Cfg->BaseAddress);		
	if (Status != XST_SUCCESS) return XST_FAILURE;
	//Set the baud rate
	XUartPs_SetBaudRate(ctrls->UartPs, 115200);		
	//Set UART in normal mode
	XUartPs_SetOperMode(ctrls->UartPs , XUARTPS_OPER_MODE_NORMAL); 	

	return Status;
}

/**************************************************************
* initDMA initializes the DMA.
*
* @param	ctrls is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
*
* @return
* 		- XST_SUCCESS if successful,
* 		- XST_FAILURE otherwise.
*
* @note	None.
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

/**************************************************************
* initInterrupt initializes MM2S and HSync interrupts. (for now)
*
* @param	ctrls is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
*
* @return
* 		- XST_SUCCESS if successful,
* 		- XST_FAILURE otherwise.
*
* @note	None.
*************************************************************/
int initInterrupt(controllers *ctrls) {
	int Status;

	xil_printf("\r\nInitializing interrupts...\r\n");

	//Get the interrupt controller configuration
	ctrls->IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);		
	if(NULL == ctrls->CfgPtr) return XST_FAILURE;
	//Initialize the interrupt controller
	Status = XScuGic_CfgInitialize(ctrls->IntcInstancePtr, ctrls->IntcConfig, (ctrls->IntcConfig)->CpuBaseAddress);		
	if(Status != XST_SUCCESS) return XST_FAILURE;

	//Perform a self-test.
	Status = XScuGic_SelfTest(ctrls->IntcInstancePtr);
	if(Status != XST_SUCCESS) {xil_printf("Failed interrupt setup\r\n"); return XST_FAILURE;}

	//Set priority for connected interrupts (0 is highest, 0xF8 is highest, with 0x8 increments)
    XScuGic_SetPriorityTriggerType(ctrls->IntcInstancePtr, HSYNC_INTR_ID, 0xA0, 0x3);
    XScuGic_SetPriorityTriggerType(ctrls->IntcInstancePtr, VSYNC_INTR_ID, 0x98, 0x3);

	//Connect interrupts to their corresponding handlers
	Status = XScuGic_Connect(ctrls->IntcInstancePtr, HSYNC_INTR_ID, (Xil_InterruptHandler) HSyncIntrHandler, ctrls->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	Status = XScuGic_Connect(ctrls->IntcInstancePtr, VSYNC_INTR_ID, (Xil_InterruptHandler) VSyncIntrHandler, ctrls->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;

	//Disable all DMA interrupts before setup
	XAxiDma_IntrDisable(ctrls->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Enable IOC (interrupt on completion) for DMA to device (DMA read/MM2S)
	XAxiDma_IntrEnable(ctrls->AxiDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Enable interrupts from GIC to PS
    //Xil_ExceptionInit();	this function does nothing!
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)INTC_HANDLER,(void *)ctrls->IntcInstancePtr);
	//Enable interrupts in ARM
    Xil_ExceptionEnable();
	return Status;
}

/**************************************************************
* enableInterrupts enables interrupts
*
* @param	ctrls is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
*
* @return
* 			None.
*
* @note	None.
*************************************************************/
void enableInterrupts(controllers *ctrls) {
    XScuGic_Enable(ctrls->IntcInstancePtr, VSYNC_INTR_ID);
	XScuGic_Enable(ctrls->IntcInstancePtr, HSYNC_INTR_ID);
	XScuGic_Enable(ctrls->IntcInstancePtr, FIFO_EMPTY_INTR_ID);
	XScuGic_Enable(ctrls->IntcInstancePtr, FIFO_FULL_INTR_ID);
}

/**************************************************************
* getChar reads from UART and returns keyboard input.
*
* @param	UartPsPtr is a pointer to the XUartPs instance.
*
* @return	The character typed to the UART.
*
* @note	None.
*************************************************************/
u8 getChar(XUartPs *UartPsPtr) {
	u32 receive = 0;
	do {
		receive = XUartPs_Recv(UartPsPtr,(u8 *) (XPAR_PS7_RAM_1_S_AXI_HIGHADDR - 0xFFF), 1);

	} while (receive == 0);
	return *(u8 *) (XPAR_PS7_RAM_1_S_AXI_HIGHADDR - 0xFFF);
}

/**************************************************************
* dmaReadReg sets the appropriate DMA registers for a read operation MM2S.
*
* @param	srcAddr is the source address from where to read.
* @param	length is the number of bytes to read.
* @param	ctrls is a pointer to the controllers structure which holds necessary configuration and instance variables.
*
* @return
* 		- XST_SUCCESS if successful,
* 		- XST_FAILURE otherwise.
*
* @note	None.
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

/**************************************************************
* HsyncIntrHandler is Hsync interrupt handler.
*
* @param	Callback is a pointer to the caller, in this case the interrupt controller.
*
* @return	None.
*
* @note	None.
*************************************************************/
void HSyncIntrHandler(void *Callback) {
	//Disable the interrupt
	XScuGic_Disable(ctrls->IntcInstancePtr, HSYNC_INTR_ID);

	//Do some data transfer
	dmaReadReg(dataArray[lineIndex], SCREEN_WIDTH, ctrls);
	Xil_DCacheFlushRange((INTPTR) dataArray[(lineIndex+1)%SCREEN_HEIGHT], SCREEN_WIDTH*4);

	//Sending 640 lines, then starting over
	if(lineIndex<(SCREEN_HEIGHT - 1))lineIndex++;

	//End of data transfer, enable the interrupt
	XScuGic_Enable(ctrls->IntcInstancePtr, HSYNC_INTR_ID);
}