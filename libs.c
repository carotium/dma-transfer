#include "libs.h"

/***************************************
 * initInterrupt initializes DMA, their respective S2MM and MM2S and also VSync, HSync interrupts.
 *
 * @param	components is a pointer to the controllers struct which holds necessary information for initialization.
 *
 * @return
 * 		- XST_SUCCESS if successful,
 * 		- XST_FAILURE otherwise.
 *
 * @note	None.
 ***************************************/
int initInterrupt(controllers *components) {
	int Status;

	xil_printf("\r\nInitializing...\r\n");

	components->CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
	if(!(components->CfgPtr)) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(components->AxiDma, components->CfgPtr);
	if(Status != XST_SUCCESS) {xil_printf("Initialization failed"); return XST_FAILURE;}
	if(XAxiDma_HasSg(components->AxiDma)) {xil_printf("Device configured as SG mode \r\n"); return XST_FAILURE;}
	//Setup interrupt system
	components->IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if(NULL == components->CfgPtr) return XST_FAILURE;
	Status = XScuGic_CfgInitialize(components->IntcInstancePtr, components->IntcConfig, (components->IntcConfig)->CpuBaseAddress);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	//Enable interrupts from the hardware
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)INTC_HANDLER,(void *)components->IntcInstancePtr);
	Xil_ExceptionEnable();

	if(Status != XST_SUCCESS) {xil_printf("Failed interrupt setup\r\n"); return XST_FAILURE;}

	//Disable all interrupts before setup
	XAxiDma_IntrDisable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_IntrDisable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	//Enable all interrupts
	XAxiDma_IntrEnable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_IntrEnable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, VSYNC_INTR_ID, 0x9F, 0x3);
	//XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, HSYNC_INTR_ID, 0xA0, 0x3);

	//Status = XScuGic_Connect(components->IntcInstancePtr, HSYNC_INTR_ID, (Xil_InterruptHandler) HSyncIntrHandler, components->IntcInstancePtr);
	Status = XScuGic_Connect(components->IntcInstancePtr, VSYNC_INTR_ID, (Xil_InterruptHandler) VSyncIntrHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	//We enable the interrupts
	//XScuGic_Enable(components->IntcInstancePtr, HSYNC_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, VSYNC_INTR_ID);

	return Status;
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
	XScuGic_Enable(IntcInstancePtr, HSYNC_INTR_ID);
}

int LoadTx(u8 *Addr, u8 *ValueAddr, u8 Length) {
	for(int i = 0; i < Length; i++) {
		Addr[i] = ValueAddr[i];
		xil_printf("%02X", ValueAddr[i]);
	}
	xil_printf("loaded\r\n");
	return XST_SUCCESS;
}

int ReadRx(u8 *Addr, u8 Length) {
	for(int i = 0; i < Length; i++) {
		xil_printf("%02X", *(Addr + i));
	}
	xil_printf("read");
	return XST_SUCCESS;
}

int XAxiDma_Send_Array(XAxiDma *axiDma, UINTPTR TxBA, UINTPTR RxBA, u32 tranLen) {
	int Status;

	Xil_DCacheFlushRange((UINTPTR)TxBA, tranLen);
	Xil_DCacheFlushRange((UINTPTR)RxBA, tranLen);
	//Status = XAxiDma_SimpleTransfer(axiDma, RxBA, tranLen, XAXIDMA_DEVICE_TO_DMA);
	//if (Status != XST_SUCCESS) return XST_FAILURE;
	Status = XAxiDma_SimpleTransfer(axiDma, TxBA, tranLen, XAXIDMA_DMA_TO_DEVICE);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	while (XAxiDma_Busy(axiDma,XAXIDMA_DMA_TO_DEVICE)) {/* Wait */}

	printf("sent data over DMA\r");
	return XST_SUCCESS;
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
	XScuGic_Disconnect(IntcInstancePtr, VSYNC_INTR_ID);
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
