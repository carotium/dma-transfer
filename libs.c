#include "libs.h"

controllers *components;

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
	//Enable interrupts from the hardware
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)INTC_HANDLER,(void *)components->IntcInstancePtr);
	Xil_ExceptionEnable();

	if(Status != XST_SUCCESS) {xil_printf("Failed interrupt setup\r\n"); return XST_FAILURE;}

	//Disable all interrupts before setup
	XAxiDma_IntrDisable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
//	XAxiDma_IntrDisable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	//Enable all interrupts
	XAxiDma_IntrEnable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
//	XAxiDma_IntrEnable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

//	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, VSYNC_INTR_ID, 0x9F, 0x3);
	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, HSYNC_INTR_ID, 0x20, 0x3);

//	Status = XScuGic_Connect(components->IntcInstancePtr, VSYNC_INTR_ID, (Xil_InterruptHandler) VSyncIntrHandler, components->IntcInstancePtr);
	Status = XScuGic_Connect(components->IntcInstancePtr, HSYNC_INTR_ID, (Xil_InterruptHandler) HSyncIntrHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	//We enable the interrupts
	XScuGic_Enable(components->IntcInstancePtr, HSYNC_INTR_ID);
//	XScuGic_Enable(components->IntcInstancePtr, VSYNC_INTR_ID);

	return Status;
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
int dmaRead(u32 srcAddr, u32 length, controllers *components) {

	u32 Status;
	u32 data_dma_to_device[8];	//dma-read from ddr to AXIS
	u32 data_dma_to_vga[300];	//dma-read of half a line of 3byte data 400*3byte=1200=4byte*300 -> u32 * 300 = u24 * 400
	u32 data_device_to_dma[8];
				     //rRgGbBxx
	u32 data_send =  0x0F0F0F00;
	u32 data_send2 = 0x09090900;

	Xil_DCacheDisable();

	for(u32 i = 0; i<1200; i+=4) {
		data_dma_to_vga[i] = i;
	}

//	Status = XAxiDma_SimpleTransfer(components->AxiDma,(UINTPTR) data_dma_to_vga, length*4, XAXIDMA_DMA_TO_DEVICE);
	Status = XAxiDma_SimpleTransfer(components->AxiDma, (UINTPTR) data_send, 2, XAXIDMA_DMA_TO_DEVICE);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	usleep(1);
	if(XAxiDma_Busy(components->AxiDma, XAXIDMA_DMA_TO_DEVICE)) {
		return XST_FAILURE;
	}

	Status = XAxiDma_SimpleTransfer(components->AxiDma, (UINTPTR) data_send2, 2, XAXIDMA_DMA_TO_DEVICE);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	usleep(1);
	if(XAxiDma_Busy(components->AxiDma, XAXIDMA_DMA_TO_DEVICE)) {
		return XST_FAILURE;
	}


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
	INTC *IntcInstancePtr = (INTC *) Callback;
	XScuGic_Disable(IntcInstancePtr, HSYNC_INTR_ID);
	//Do some data transfer
	//xil_printf("Hsync\r\n");
	dmaRead(0, 0, components);
	//End of data transfer
	XScuGic_Enable(IntcInstancePtr, HSYNC_INTR_ID);
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
