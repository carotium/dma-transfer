#include "libs.h"

controllers *components;
u32 data_dma_to_vga[512];

/***************************************
 * initUart initializes the Uart.
 *
 * @param	components is a pointer to the controllers struct which holds necessary config and instance variables for initialization.
 *
 * @return
 * 		- XST_SUCCESS if successful,
 * 		- XST_FAILURE otherwise.
 *
 * @note	None.
 ***************************************/
int initUart(controllers *components) {
	int Status;

	xil_printf("\r\nInitializing Uart...\r\n");
	//Get the UART configuration
	components->Cfg = XUartPs_LookupConfig(UART_DEVICE_ID);		
	if (components->Cfg == NULL) return XST_FAILURE;
	//Initialize the UART
	Status = XUartPs_CfgInitialize(components->UartPs, components->Cfg, components->Cfg->BaseAddress);		
	if (Status != XST_SUCCESS) return XST_FAILURE;
	//Set the baud rate
	XUartPs_SetBaudRate(components->UartPs, 115200);		
	//Set the UART in normal mode
	XUartPs_SetOperMode(components->UartPs , XUARTPS_OPER_MODE_NORMAL); 	

	return Status;
}
/***************************************
 * initDMA initializes the DMA.
 *
 * @param	components is a pointer to the controllers struct which holds necessary config and instance variables for initialization.
 *
 * @return
 * 		- XST_SUCCESS if successful,
 * 		- XST_FAILURE otherwise.
 *
 * @note	None.
 ***************************************/
int initDMA(controllers *components) {
	int Status;

	xil_printf("\r\nInitializing DMA...\r\n");
	//Get the DMA configuration
	components->CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
	if(!(components->CfgPtr)) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);
		return XST_FAILURE;
	}
	//Initialize the DMA
	Status = XAxiDma_CfgInitialize(components->AxiDma, components->CfgPtr);
	if(Status != XST_SUCCESS) {xil_printf("Initialization failed"); return XST_FAILURE;}
	if(XAxiDma_HasSg(components->AxiDma)) {xil_printf("Device configured as SG mode \r\n"); return XST_FAILURE;}

	return Status;
}
int enableInterrupts(controllers *components) {
	XScuGic_Enable(components->IntcInstancePtr, HSYNC_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, FIFO_EMPTY_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, FIFO_FULL_INTR_ID);
}
/***************************************
 * initInterrupt initializes MM2S and HSync interrupts. (for now)
 *
 * @param	components is a pointer to the controllers struct which holds necessary config and instance variables for initialization.
 *
 * @return
 * 		- XST_SUCCESS if successful,
 * 		- XST_FAILURE otherwise.
 *
 * @note	None.
 ***************************************/
int initInterrupt(controllers *components) {

	int Status;

	xil_printf("\r\nInitializing interrupts...\r\n");

	//Get the interrupt controller configuration
	components->IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);		
	if(NULL == components->CfgPtr) return XST_FAILURE;
	//Initialize the interrupt controller
	Status = XScuGic_CfgInitialize(components->IntcInstancePtr, components->IntcConfig, (components->IntcConfig)->CpuBaseAddress);		
	if(Status != XST_SUCCESS) return XST_FAILURE;

	//Perform a self-test.
	Status = XScuGic_SelfTest(components->IntcInstancePtr);
	if (Status != XST_SUCCESS)return XST_FAILURE;
	if(Status != XST_SUCCESS) {xil_printf("Failed interrupt setup\r\n"); return XST_FAILURE;}


//	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, VSYNC_INTR_ID, 0x9F, 0x3);
	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, HSYNC_INTR_ID, 0xA0, 0x3);
	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, FIFO_EMPTY_INTR_ID, 0x98, 0x3);
	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, FIFO_FULL_INTR_ID, 0x90, 0x3);

//	Status = XScuGic_Connect(components->IntcInstancePtr, VSYNC_INTR_ID, (Xil_InterruptHandler) VSyncIntrHandler, components->IntcInstancePtr);
	Status = XScuGic_Connect(components->IntcInstancePtr, HSYNC_INTR_ID, (Xil_InterruptHandler) HSyncIntrHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	Status = XScuGic_Connect(components->IntcInstancePtr, FIFO_EMPTY_INTR_ID, (Xil_InterruptHandler) FifoEmptyHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	Status = XScuGic_Connect(components->IntcInstancePtr, FIFO_FULL_INTR_ID, (Xil_InterruptHandler) FifoFullHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	//We enable the interrupts



//	XScuGic_Enable(components->IntcInstancePtr, VSYNC_INTR_ID);

	//Disable all interrupts before setup
	XAxiDma_IntrDisable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
//	XAxiDma_IntrDisable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	//Enable all interrupts
//	XAxiDma_IntrEnable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
//	XAxiDma_IntrEnable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

	//Enable interrupts from GIC to PS
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)INTC_HANDLER,(void *)components->IntcInstancePtr);
	Xil_ExceptionEnable();	//Enable interrupts in ARM
	return Status;
}


int dmaReadReg(u32 *srcAddr, u32 length, controllers *components) {
//	Xil_Out32(addr, value);
	//Setting DMA MM2S run/stop bit to 1
	Xil_Out32((components->CfgPtr->BaseAddr + XAXIDMA_CR_OFFSET), 1);
	//Write a valid source address to the MM2S_SA register
	Xil_Out32((UINTPTR) (components->CfgPtr->BaseAddr + XAXIDMA_SRCADDR_OFFSET), (u32) srcAddr);
	//Write the number of bytes to transfer
	Xil_Out32((UINTPTR) (components->CfgPtr->BaseAddr + XAXIDMA_BUFFLEN_OFFSET), length*4);
	//Check if transmit is (not) done -> usleep
	if(!(Xil_In32(components->CfgPtr->BaseAddr + XAXIDMA_SR_OFFSET) & 0x2)) {
		usleep(1);
	}
	return XST_SUCCESS;
}
/***************************************
 * dmaRead reads data from DDR to AXIS.
 *
 * @param	components is a pointer to the controllers struct which holds necessary config and instance variables for initialization.
 *
 * @return
 * 		- XST_SUCCESS if successful,
 * 		- XST_FAILURE otherwise.
 *
 * @note	None.
 ***************************************/
int dmaRead(u32 *srcAddr, u32 length, controllers *components) {

	u32 Status;
//	u32 data_dma_to_device[8];	//dma-read from ddr to AXIS
//	u32 data_device_to_dma[8];
				     //rRgGbBxx
//	u32 data_send =  0x0F0F0F00;
//	u32 data_send2 = 0x09090900;

//	Status = XAxiDma_SimpleTransfer(components->AxiDma,(UINTPTR) data_dma_to_vga, length*4, XAXIDMA_DMA_TO_DEVICE);
	Status = XAxiDma_SimpleTransfer(components->AxiDma, (UINTPTR) srcAddr, length, XAXIDMA_DMA_TO_DEVICE);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	usleep(1);
	if(XAxiDma_Busy(components->AxiDma, XAXIDMA_DMA_TO_DEVICE)) {
		return XST_FAILURE;
	}
/*
	Status = XAxiDma_SimpleTransfer(components->AxiDma, (UINTPTR) data_send2, 2, XAXIDMA_DMA_TO_DEVICE);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	usleep(1);
	if(XAxiDma_Busy(components->AxiDma, XAXIDMA_DMA_TO_DEVICE)) {
		return XST_FAILURE;
	}*/


	return XST_SUCCESS;
}
/***************************************
 * HsyncIntrHandler is Hsync interrupt handler.
 *
 * @param	Callback is a pointer to the caller, in this case the interrupt controller.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void HSyncIntrHandler(void *Callback) {
	static u32 i = 0;
	INTC *IntcInstancePtr = (INTC *) Callback;
	XScuGic_Disable(IntcInstancePtr, HSYNC_INTR_ID);
	static u8 st = 0;	//prva polovica ali druga polovica fifo-ta
	u32 offset = (st) ? 0 : 255;
	//Do some data transfer
//	Xil_DCacheFlushRange((u32 *) (data_dma_to_vga), 512);
	dmaReadReg((data_dma_to_vga), 256, components);
	st = !st;
	offset = (st) ? 0 : 255;
//	dmaReadReg((data_dma_to_vga + offset), 256, components);
//	Xil_DCacheInvalidate();
//	u32 *data = 0x0FF0;
//	dmaRead(data, 4, components);
	i++;
	//End of data transfer

	//Xil_DCacheFlushRange((UINTPTR) )
	XScuGic_Enable(IntcInstancePtr, HSYNC_INTR_ID);
}
void FifoEmptyHandler(void *Callback) {
	INTC *IntcInstancePtr = (INTC *) Callback;
	XScuGic_Disable(IntcInstancePtr, FIFO_EMPTY_INTR_ID);
	//
	XScuGic_Enable(IntcInstancePtr, FIFO_EMPTY_INTR_ID);
}
void FifoFullHandler(void *Callback) {
	INTC *IntcInstancePtr = (INTC *) Callback;
	XScuGic_Disable(IntcInstancePtr, FIFO_FULL_INTR_ID);
	//
	XScuGic_Enable(IntcInstancePtr, FIFO_FULL_INTR_ID);
}
/***************************************
 * VsyncIntrHandler is Vsync interrupt handler.
 *
 * @param	Callback is a pointer to the caller, in this case the interrupt controller.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void VSyncIntrHandler(void *Callback) {
	INTC *IntcInstancePtr = (INTC *) Callback;
	XScuGic_Disable(IntcInstancePtr, VSYNC_INTR_ID);
	//Do some data transfer
	XScuGic_Enable(IntcInstancePtr, VSYNC_INTR_ID);
}
/***************************************
 * DisableIntrSystem disables connected interrupts.
 *
 * @param	IntcInstancePtr is pointer to interrupt controller instance.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void DisableIntrSystem(INTC *IntcInstancePtr) {
	//XScuGic_Disconnect(IntcInstancePtr, HSYNC_INTR_ID);
//	XScuGic_Disconnect(IntcInstancePtr, VSYNC_INTR_ID);
}

/***************************************
 * getChar reads from UART and returns keyboard input.
 *
 * @param	Callback is a pointer to the caller, in this case the interrupt controller.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
u8 getChar(XUartPs *UartPsPtr) {
	u32 receive = 0;
	do {
		receive = XUartPs_Recv(UartPsPtr,(u8 *) (XPAR_PS7_RAM_1_S_AXI_HIGHADDR - 0xFFF), 1);

	} while (receive == 0);
	return *(u8 *) (XPAR_PS7_RAM_1_S_AXI_HIGHADDR - 0xFFF);
}
