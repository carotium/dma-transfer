/*************************************************************
* File: libs.h
* Description: MiniZed board DMA, GIC, UART configuration.
* Other interrupt functions for DMA, UART receive, VGA synchronization signals.
*
* Author: Ahac Rafael Bela
* Created on: 01.03.2025
* Last modified: 10.04.2025
*************************************************************/
//Protection macro
#pragma once
#ifndef LIBS_H
#define LIBS_H

/*************************************************************
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
#include "string.h"

/*************************************************************
* Macro section
*************************************************************/
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
//DMA interrupts
#define XPAR_FABRIC_HSYNC_INTROUT_VEC_ID 63U
#define XPAR_FABRIC_VSYNC_INTROUT_VEC_ID 64U

/*************************************************************
* Interrupt section
*************************************************************/
#define HSYNC_INTR_ID   XPAR_FABRIC_HSYNC_INTROUT_VEC_ID
#define VSYNC_INTR_ID   XPAR_FABRIC_VSYNC_INTROUT_VEC_ID
#define UART_INTR_ID	XPAR_XUARTPS_1_INTR

/*************************************************************
* Device section
*************************************************************/
#define DMA_DEV_ID      XPAR_AXIDMA_0_DEVICE_ID
#define INTC_DEV_ID     XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC            XScuGic
#define INTC_HANDLER    XScuGic_InterruptHandler
#define UART_DEV_ID     XPAR_XUARTPS_1_DEVICE_ID

#define DDR_BASE_ADDR   XPAR_PS7_DDR_0_S_AXI_BASEADDR
#define MEM_BASE_ADDR   (DDR_BASE_ADDR + 0x01000000)

#define UART_RX_BUFFER	(XPAR_PS7_DDR_0_S_AXI_HIGHADDR - 0xFFF)

#ifndef DEBUG
extern void xil_printf(const char *format, ...);
#endif

/*************************************************************
* Struct section
*************************************************************/
typedef struct controllers_t {
	XAxiDma *AxiDma;			//Pointer to Axi Dma
	XAxiDma_Config *CfgPtr;		//Pointer to the config of Axi Dma
	INTC *IntcInstancePtr;		//Pointer to interrupt controller
	XScuGic_Config *IntcConfig;	//Pointer to the config of the interrupt controller
	XUartPs *UartPs;			//Pointer to UartPs instance
	XUartPs_Config *Cfg;		//Pointer to the config of UartPs
} controllers;

typedef struct point_t {
	int x;
	int y;
} point;

/*************************************************************
* Variable declaration section
*************************************************************/
extern controllers *ctrls;
extern u32 vgaArray[SCREEN_HEIGHT][SCREEN_WIDTH];
extern volatile u8 caughtChar;
extern volatile u8 receivedCount;

/*************************************************************
* Function prototype section
*************************************************************/
//Encompasses all initializations.
int initPlatform(controllers *ctrls);
//Initializes UART.
int initUART(controllers *ctrls);
//Initializes the DMA controller.
int initDMA(controllers *ctrls);
//Initializes interrupts.
int initInterrupt(controllers *ctrls);
//Enables interrupts.
void enableInterrupts(controllers *ctrls);
//Starts a DMA read operation using corresponding registers.
int dmaReadReg(u32 *srcAddr, u32 length, controllers *ctrls);

/*************************************************************
* Interrupt service routine section
*************************************************************/
//Horizontal synchronization interrupt service routine.
void HSyncIntrHandler(void *Callback);
//Vertical synchronization interrupt service routine.
void VSyncIntrHandler(void *Callback);
//UART Interrupt service routine.
void UartPsIntrHandler(void *Callback, u32 Event, u32 EventData);

#endif /* LIBS_H */

/*************************************************************
* End of file
*************************************************************/
