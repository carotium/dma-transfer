#include "libs.h"

controllers *components;	//Struct that holds driver information -> defined as extern in libs.h
u32 data_dma_to_vga[512];
u32 dataArray[256][256];
static volatile s32 i = 0;	//line number to send

/***************************************
 * initPlatform initializes the platform. It initializes UART, DMA and interrupts.
 *
 * @param	components is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
 *
 * @return
 * 		- XST_SUCCESS if successful,
 * 		- XST_FAILURE otherwise.
 *
 * @note	None.
 ***************************************/
int initPlatform(controllers *components) {
	int Status;

	Status = initUART(components);
	if(Status != XST_SUCCESS) {
		xil_printf("Initialization of UartPs failed :(\n\r");
		return XST_FAILURE;
	} else xil_printf("Initialization of UartPs done!\n\r");

	Status = initDMA(components);
	if(Status != XST_SUCCESS) {
		xil_printf("Initialization of DMA failed :(\n\r");
		return XST_FAILURE;
	} 	else xil_printf("Initialization of DMA done!\n\r");

	Status = initInterrupt(components);
	if(Status != XST_SUCCESS) {
		xil_printf("Initialization of interrupts failed.\n\r");
		return XST_FAILURE;
	} else xil_printf("Initialization of interrupts done!\r\n");

	return Status;
}
/***************************************
 * initUart initializes the UART.
 *
 * @param	components is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
 *
 * @return
 * 		- XST_SUCCESS if successful,
 * 		- XST_FAILURE otherwise.
 *
 * @note	None.
 ***************************************/
int initUART(controllers *components) {
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
 * @param	components is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
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
 * @param	components is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
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
	if(Status != XST_SUCCESS) {xil_printf("Failed interrupt setup\r\n"); return XST_FAILURE;}

	//Set priority for connected interrupts (0 is highest, 0xF8 is highest, with 0x8 increments
	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, TX_INTR_ID, 0x90, 0x3);
	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, VSYNC_INTR_ID, 0x98, 0x3);
	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, HSYNC_INTR_ID, 0xA0, 0x3);
	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, FIFO_EMPTY_INTR_ID, 0x98, 0x3);
	XScuGic_SetPriorityTriggerType(components->IntcInstancePtr, FIFO_FULL_INTR_ID, 0x90, 0x3);

	//Connect interrupts to their corresponding handlers
	Status = XScuGic_Connect(components->IntcInstancePtr, TX_INTR_ID, (Xil_InterruptHandler) TxIntrHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	Status = XScuGic_Connect(components->IntcInstancePtr, HSYNC_INTR_ID, (Xil_InterruptHandler) HSyncIntrHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	Status = XScuGic_Connect(components->IntcInstancePtr, VSYNC_INTR_ID, (Xil_InterruptHandler) VSyncIntrHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	Status = XScuGic_Connect(components->IntcInstancePtr, FIFO_EMPTY_INTR_ID, (Xil_InterruptHandler) FifoEmptyHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;
	Status = XScuGic_Connect(components->IntcInstancePtr, FIFO_FULL_INTR_ID, (Xil_InterruptHandler) FifoFullHandler, components->IntcInstancePtr);
	if(Status != XST_SUCCESS) return XST_FAILURE;

	//Disable all DMA interrupts before setup
	XAxiDma_IntrDisable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
//	XAxiDma_IntrDisable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

	//Enable IOC (interrupt on completion) for DMA to device (DMA read/MM2S)
	XAxiDma_IntrEnable(components->AxiDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
//	XAxiDma_IntrEnable(components->AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

	//Enable interrupts from GIC to PS
//	Xil_ExceptionInit();	this function does nothing!
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)INTC_HANDLER,(void *)components->IntcInstancePtr);
	Xil_ExceptionEnable();	//Enable interrupts in ARM
	return Status;
}
/***************************************
 * enableInterrupts enables interrupts
 *
 * @param	components is a pointer to the controllers structure which holds necessary configuration and instance variables for initialization.
 *
 * @return
 * 			None.
 *
 * @note	None.
 ***************************************/
void enableInterrupts(controllers *components) {
//	XScuGic_Enable(components->IntcInstancePtr, TX_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, VSYNC_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, HSYNC_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, FIFO_EMPTY_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, FIFO_FULL_INTR_ID);
}

int dmaReadRun(controllers *components) {
	//Setting DMA MM2S run/stop bit to 1
	Xil_Out32((components->CfgPtr->BaseAddr + XAXIDMA_CR_OFFSET), 1);

	return XST_SUCCESS;
}
int dmaReadReg(u32 *srcAddr, u32 length, controllers *components) {
	//Setting DMA MM2S run/stop bit to 1
	Xil_Out32((components->CfgPtr->BaseAddr + XAXIDMA_CR_OFFSET), XAXIDMA_CR_RUNSTOP_MASK);
	//Write a valid source address to the MM2S_SA register
	Xil_Out32((UINTPTR) (components->CfgPtr->BaseAddr + XAXIDMA_SRCADDR_OFFSET), (u32) srcAddr);
	//Write the number of bytes to transfer -> this starts the DMA transaction
	Xil_Out32((UINTPTR) (components->CfgPtr->BaseAddr + XAXIDMA_BUFFLEN_OFFSET), length * 4);

	//Check if transmit is (not) done -> usleep
	//instead of waiting in this function an interrupt will be called when transfer is complete
	if(!(Xil_In32(components->CfgPtr->BaseAddr + XAXIDMA_SR_OFFSET) & 0x2)) {
		usleep(1);
	}
	return XST_SUCCESS;
}
/***************************************
 * TxIntrHandler is transmit interrupt on complete handler.
 *
 * @param	Callback is a pointer to the caller, in this case the interrupt controller.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void TxIntrHandler(void *Callback) {
	//DMA transaction completed on DMA to DEVICE channel (DMA read, MM2S)
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
	//Disable the interrupt
	XScuGic_Disable(components->IntcInstancePtr, HSYNC_INTR_ID);
	//Do some data transfer
	dmaReadReg(dataArray[i], 256, components);
	//Sending 256 lines, then starting over
	if(i<255)i++;
//	i = (i<255) ? (i+1) : 0;

	//End of data transfer, enable the interrupt
	XScuGic_Enable(components->IntcInstancePtr, HSYNC_INTR_ID);
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
	XScuGic_Disable(components->IntcInstancePtr, VSYNC_INTR_ID);
	//Reset the line number identifier
	i=-29;
	XScuGic_Enable(components->IntcInstancePtr, VSYNC_INTR_ID);
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

void drawLine() {
	int x1, x2, y1, y2;
	char c;
	x1 = rand()%256;
	y1 = rand()%256;
	x2 = rand()%256;
	y2 = rand()%256;
	c = rand()%64;

	line(x1, y1, x2, y2, c);
	usleep(30000);	//cakaj 300ms ?
}

void line(int x0, int y0, int x1, int y1, char c) {
/*
	//first implementation
	int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
  	int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
  	int err = dx + dy, e2; // error value e_xy
 
  	for (;;){  // loop
    	setPixel (x0,y0);
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0
		if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0
	}
*/
	//second implementation, maybe AI created it for this specific case
	int dx = abs(x1-x0);
	int dy = abs(y1-y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx-dy;
	int e2;

	while(1) {
		data_dma_to_vga[y0%512] = c;
		if(x0 == x1 && y0 == y1) break;
		e2 = 2*err;
		if(e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
/*
	//third implementation
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = (dx>dy ? dx : -dy)/2, e2;
	char *p;

	for(;;){
		p= (char *)XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR+x0+512*y0;
		*p = c;

		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
*/
}