#include "libs.h"

//Struct that holds driver information
controllers *components;

u32 dataArray[480][640];
//Line number index to send to VGA
static volatile s32 lineIndex = 0;
//Index of the character to print on the VGA screen
static u32 vgaIndex = 0;

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

	//Initialize UART
	Status = initUART(components);
	if(Status != XST_SUCCESS) {
		xil_printf("Initialization of UartPs failed :(\n\r");
		return XST_FAILURE;
	} else xil_printf("Initialization of UartPs done!\n\r");

	//Initialize DMA
	Status = initDMA(components);
	if(Status != XST_SUCCESS) {
		xil_printf("Initialization of DMA failed :(\n\r");
		return XST_FAILURE;
	} 	else xil_printf("Initialization of DMA done!\n\r");

	//Initialize interrupts
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
	//Initialize UART
	Status = XUartPs_CfgInitialize(components->UartPs, components->Cfg, components->Cfg->BaseAddress);		
	if (Status != XST_SUCCESS) return XST_FAILURE;
	//Set the baud rate
	XUartPs_SetBaudRate(components->UartPs, 115200);		
	//Set UART in normal mode
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
	//Initialize DMA
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

	//Set priority for connected interrupts (0 is highest, 0xF8 is highest, with 0x8 increments)
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

	//Enable IOC (interrupt on completion) for DMA to device (DMA read/MM2S)
	XAxiDma_IntrEnable(components->AxiDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);

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
	XScuGic_Enable(components->IntcInstancePtr, VSYNC_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, HSYNC_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, FIFO_EMPTY_INTR_ID);
	XScuGic_Enable(components->IntcInstancePtr, FIFO_FULL_INTR_ID);
}

/***************************************
 * dmaReadReg sets the appropriate DMA registers for a read operation MM2S.
 *
 * @param	srcAddr is the source address from where to read.
 * @param	length is the number of bytes to read.
 * @param	components is a pointer to the controllers structure which holds necessary configuration and instance variables.
 *
 * @return
 * 		- XST_SUCCESS if successful,
 * 		- XST_FAILURE otherwise.
 *
 * @note	None.
 ***************************************/
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
	dmaReadReg(dataArray[lineIndex], 640, components);
	Xil_DCacheFlushRange((INTPTR) dataArray[(lineIndex+1)%480], 640*4);

	//Sending 640 lines, then starting over
	if(lineIndex<639)lineIndex++;

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

	//Reset the line index (its negative because I need to fix the interrupt in hardware)
	lineIndex=-35;

	XScuGic_Enable(components->IntcInstancePtr, VSYNC_INTR_ID);
}

//Soon to be erased
void FifoEmptyHandler(void *Callback) {
	INTC *IntcInstancePtr = (INTC *) Callback;
	XScuGic_Disable(IntcInstancePtr, FIFO_EMPTY_INTR_ID);
	//
	XScuGic_Enable(IntcInstancePtr, FIFO_EMPTY_INTR_ID);
}

//Soon to be erased
void FifoFullHandler(void *Callback) {
	INTC *IntcInstancePtr = (INTC *) Callback;
	XScuGic_Disable(IntcInstancePtr, FIFO_FULL_INTR_ID);
	//
	XScuGic_Enable(IntcInstancePtr, FIFO_FULL_INTR_ID);
}

/***************************************
 * getChar reads from UART and returns keyboard input.
 *
 * @param	UartPsPtr is a pointer to the XUartPs instance.
 *
 * @return	The character typed to the UART.
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

/***************************************
 * drawLines draws multiple lines to the dataArray using Bresenham's line algorithm.
 * It also erases the last line drawn and calculates the next line's coordinates.
 *
 * @param	t is an index for which line to draw.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void drawLines(u32 t) {

	drawLine(startX[t], startY[t], endX[t], endY[t], colorPalette[t%16]);

	//Check for screen borders and reverse direction
	if(startX[t] < abs(dx0) || startX[t] >= (SCREEN_WIDTH-abs(dx0))) dx0 = -dx0;
	if(startY[t] < (abs(dy0)+1) || startY[t] >= (SCREEN_HEIGHT-abs(dy0))) dy0 = -dy0;
	if(endX[t] < abs(dx1) || endX[t] >= (SCREEN_WIDTH-abs(dx1))) dx1 = -dx1;
	if(endY[t] < abs(dy1) || endY[t] >= (SCREEN_HEIGHT-abs(dy1))) dy1 = -dy1;

	static int full = 0;
	if(t == 255) full = 1;

	u32 indexLast = (t == 255) ? 0 : (t + 1);

	if(full) {
		//Start deleting lines
		

		eraseLineB(startX[indexLast],
				  startY[indexLast],
				  endX[indexLast],
				  endY[indexLast]);
	}
	calculateLine(indexLast);
}

/***************************************
 * calculateLine calculates line coordinates according to speed of each point of the line.
 *
 * @param	t is an index for which line's coordinates to calculate.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void calculateLine(u32 t) {
	//Coordinates for next line
	u32 indexNext = (t == 0) ? 255 : (t-1)%256;
	startX[t%256] = startX[indexNext] + dx0;
	startY[t%256] = startY[indexNext] + dy0;
	endX[t%256] = endX[indexNext] + dx1;
	endY[t%256] = endY[indexNext] + dy1;
}

/***************************************
 * drawLine draws a line to the dataArray using Bresenham's line algorithm.
 *
 * @param	x0 is the starting x coordinate of the line.
 * @param	y0 is the starting y coordinate of the line.
 * @param	x1 is the ending x coordinate of the line.
 * @param	y1 is the ending y coordinate of the line.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void drawLine(int x0, int y0, int x1, int y1, int color) {

	//Bresenham's line algorithm implementation
	int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
  	int dy = abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
  	int err = dx - dy, e2; // error value e_xy
 
  	for (;;){  // loop
    	*(*(dataArray + y0)+x0) = color;
    	if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= -dy) {
			if(x0 == x1) break;
			err -= dy;
			x0 += sx;
		} // e_xy+e_x > 0
		if (e2 <= dx) {
			if(y0 == y1) break;
			err += dx;
			y0 += sy;
		} // e_xy+e_y < 0
	}
}

/***************************************
 * eraseLineB erases a line in the dataArray using Bresenham's line algorithm.
 *
 * @param	x0 is the starting coordinate of the line to erase.
 * @param	y0 is the starting coordinate of the line to erase.
 * @param	x1 is the ending coordinate of the line to erase.
 * @param	y1 is the ending coordinate of the line to erase.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void eraseLineB(u32 x0, u32 y0, u32 x1, u32 y1) {
	//Bresenham's line algorithm implementation
	int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx - dy, e2; // error value e_xy
	for (;;){  // loop
	    	*(*(dataArray + y0)+x0) = 0;
	    	if(x0 == x1 && y0 == y1) break;
			e2 = 2 * err;
			if (e2 >= -dy) {
				if(x0 == x1) break;
				err -= dy;
				x0 += sx;
			} // e_xy+e_x > 0
			if (e2 <= dx) {
				if(y0 == y1) break;
				err += dx;
				y0 += sy;
			} // e_xy+e_y < 0
		}
}

/***************************************
 * lineStart initializes the starting coordinates of the first line.
 *
 * @param	
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void lineStart(int state) {
	//Random starting coordinates for the first line
	startX[0] = rand()%SCREEN_WIDTH;
	startY[0] = rand()%SCREEN_HEIGHT;
	endX[0] = rand()%SCREEN_WIDTH;
	endY[0] = rand()%SCREEN_HEIGHT;

	//(Random) speed for the first line
	dx0 = rand()%6 + 1;
	dx1 = rand()%6 + 1;
	dy0 = rand()%6 + 1;
	dy1 = rand()%6 + 1;
}

/***************************************
 * getLetter looks up the IBM VGA 8x16 font array for a specific character.
 *
 * @param	asciiNum is the ASCII number of the character.
 *
 * @return	Pointer to a character in the IBM VGA 8x16 font array.
 *
 * @note	None.
 ***************************************/
u8 *getLetter(u32 asciiNum) {
	u8 *character;
	u32 ascii = asciiNum * 16;
	character = (IBM_VGA_8x16 + ascii);

	//Line feed return key or '\n'
	if(asciiNum == 0xA || asciiNum == 0xD) {	
		nextLine();
		return getLetter(dataArray[vgaIndex/80*16][(vgaIndex*8)%640]);
		vgaIndex--;

	//Horizontal tab key or '\r'
	} else if(asciiNum == 0x9) {
		nextTab();
		return getLetter(dataArray[vgaIndex/80*16][(vgaIndex*8)%640]);
		vgaIndex--;
	}

	return character;
}

/***************************************
 * eraseLetter erases a letter at the vgaIndex position.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void eraseLetter(void) {
	for(int i = 0; i < 16; i++) {
		for(int j = 0; j < 8; j++) {
			dataArray[i+vgaIndex/80*16][j+(vgaIndex*8)%640] = 0;
			}
		}
}

/***************************************
 * printLetter prints a letter at the vgaIndex position.
 *
 * @param	character is a pointer to the character in the IBM VGA 8x16 font array.
 * @param	color is the color of the character defined in main.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void printLetter(u8 *character, colors color) {
	//Erase character at this index prior to printing it
	eraseLetter();

	for(int i = 0; i < 16; i++) {

		u8 row = *(character + i);

		for(int j = 0; j < 8; j++) {
			//extract single pixel
			u32 selector = power(2, (7-j));
			u32 pixel = row & selector;
			u8 pixelValue = (pixel != 0) ? 1 : 0;

//			dataArray[i+y*16][j+x*8] = pixelValue * color;
			dataArray[i+vgaIndex/80*16][j+(vgaIndex*8)%640] = pixelValue * color;
		}
	}
	if(vgaIndex < 2399) vgaIndex++;	//640/8=80, 480/16=30; 80*30=2400
	else vgaIndex = 0;
}

/***************************************
 * printVga prints to the VGA screen display.
 *
 * @param	string is the string to print.
 * @param	color is the color of the string.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void printVGA(const char* string, colors color) {
	u32 i = 0;		//Index for the string
	while(string[i] != 0) {
		printLetter(getLetter(string[i]), color);
		i++;
	}
}

/***************************************
 * nextLine increments the vgaIndex so it points one line ahead.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void nextLine() {
	vgaIndex = (vgaIndex/80)*80 + SCREEN_WIDTH/8-1;
//	eraseLine();
}

/***************************************
 * nextTab increments the vgaIndex so it points one tab (4 spaces) ahead.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************/
void nextTab() {
	vgaIndex += 3;
}

/***************************************
 * power calculates a value of a base raised to a power.
 *
 * @param	base is the base of potentiation.
 * @param	power is the power of potentiation.
 *
 * @return	Value of a base raised to a power.
 *
 * @note	None.
 ***************************************/
u32 power(u32 base, u32 power) {
	if (power == 0) return 1;
	u32 number = 1;
	for(int i = 0; i < power; i++) {
		number *= base;
	}
	return number;
}
