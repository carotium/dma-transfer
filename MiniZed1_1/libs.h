/**************************************************************
 * File: libs.h
 * Description: DMA, VGA, GIC, UART configuration. Other 
 * miscellaneous functions.
 *
 * Author: Ahac Rafael Bela
 * Created on: 01.03.2025
 * Last modified: 01.03.2025
 *************************************************************/
//Protection macro
#pragma once
#ifndef LIBS_H
#define LIBS_H

/**************************************************************
 * Include section
 *************************************************************/
//Standard xparameters library
#include "xparameters.h"
//DMA library
#include "xaxidma.h"
#include "xaxidma_hw.h"
//Exception library
#include "xil_exception.h"
//Cache library
#include "xil_cache.h"
//Debug library
#include "xdebug.h"
//Interrupt controller library
#include "xscugic.h"
//UART library
#include "xuartps.h"
//Sleep library
#include "sleep.h"
//Standard libraries
#include "stdio.h"
#include "stdlib.h"

/**************************************************************
 * Macros section
 *************************************************************/
//DMA interrupts
#define XPAR_FABRIC_HSYNC_INTROUT_VEC_ID 63U
#define XPAR_FABRIC_VSYNC_INTROUT_VEC_ID 64U

/**************************************************************
 * Interrupt section
 *************************************************************/
#define HSYNC_INTR_ID   XPAR_FABRIC_HSYNC_INTROUT_VEC_ID
#define VSYNC_INTR_ID   XPAR_FABRIC_VSYNC_INTROUT_VEC_ID

/**************************************************************
 * Device section
 *************************************************************/
#define DMA_DEV_ID      XPAR_AXIDMA_0_DEVICE_ID
#define INTC_DEV_ID     XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC            XScuGic
#define INTC_HANDLER    XScuGic_InterruptHandler
#define UART_DEV_ID     XPAR_XUARTPS_1_DEVICE_ID

#define DDR_BASE_ADDR   XPAR_PS7_DDR_0_S_AXI_BASEADDR
#define MEM_BASE_ADDR   (DDR_BASE_ADDR + 0x01000000)

#ifndef DEBUG
extern void xil_printf(const char *format, ...);
#endif

/**************************************************************
 * Struct definitions
 *************************************************************/
typedef struct controllers_t {
	XAxiDma *AxiDma;			//Pointer to Axi Dma
	XAxiDma_Config *CfgPtr;		//Pointer to the config of Axi Dma
	INTC *IntcInstancePtr;		//Pointer to interrupt controller
	XScuGic_Config *IntcConfig;	//Pointer to the config of the interrupt controller
	XUartPs *UartPs;			//Pointer to UartPs instance
	XUartPs_Config *Cfg;		//Pointer to the config of UartPs
} controllers;

/**************************************************************
 * Variable declarations
 *************************************************************/
extern controllers ctrls;
extern u32 dataArray[SCREEN_HEIGHT][SCREEN_WIDTH];

/**************************************************************
 * Function prototypes
 *************************************************************/
int initPlatform(controllers *ctrls);

int initUART(controllers *ctrls);
int initDMA(controllers *ctrls);
int initInterrupt(controllers *ctrls);

void enableInterrupts(controllers *ctrls);

u8 getChar(XUartPs *UartPs);
int dmaReadReg(XAxiDma *InstancePtr, UINTPTR BuffAddr, u32 Length, int Direction);

/**************************************************************
 * Interrupt service routines
 *************************************************************/
void HSyncIntrHandler(void *Callback);
void VSyncIntrHandler(void *Callback);

#endif /* LIBS_H */

/**************************************************************
 * End of file
 *************************************************************/