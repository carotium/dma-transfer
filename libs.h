#ifndef LIBS_H		//Protection macro
#define LIBS_H

#include "xaxidma.h"
#include "xaxidma_hw.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xdebug.h"
#include "xscugic.h"
#include "xuartps.h"
#include "sleep.h"

#include "stdio.h"

/***************************************
 * Macros
 ***************************************/
//Directly after DMA MM2S and S2MM interrupt IDs (61U, 62U)

//only MM2S is enabled at XPAR_FABRIC_AXIDMA_0_VEC_ID 61U
#define XPAR_FABRIC_HSYNC_INTROUT_VEC_ID 62U
#define XPAR_FABRIC_VSYNC_INTROUT_VEC_ID 63U

//Used interrupt IDs

//#define RX_INTR_ID			XPAR_FABRIC_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID			XPAR_FABRIC_AXIDMA_0_VEC_ID
#define HSYNC_INTR_ID		XPAR_FABRIC_HSYNC_INTROUT_VEC_ID
#define VSYNC_INTR_ID		XPAR_FABRIC_VSYNC_INTROUT_VEC_ID

//DMA device ID
#define DMA_DEV_ID			XPAR_AXIDMA_0_DEVICE_ID
//DDR Base Address, in xparameters.h
#define DDR_BASE_ADDR		XPAR_PS7_DDR_0_S_AXI_BASEADDR
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x01000000)
//Transmit and receive memory addresses
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)
//Interrupt controller device ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC				XScuGic
#define INTC_HANDLER		XScuGic_InterruptHandler
//Timeout loop counter for reset
#define RESET_TIMEOUT_COUNTER	10000

//Uartps driver ID
#define UART_DEVICE_ID      XPAR_XUARTPS_1_DEVICE_ID

#ifndef DEBUG
extern void xil_printf(const char *format, ...);
#endif

/***************************************
 * Type definitions
 ***************************************/
typedef struct controllers_t {
	XAxiDma_Config *CfgPtr;		//Pointer to the config of Axi Dma
	XAxiDma *AxiDma;			//Pointer to Axi Dma
	INTC *IntcInstancePtr;		//Pointer to interrupt controller
	XScuGic_Config *IntcConfig;	//Pointer to the config of the interrupt controller
	XUartPs_Config *Cfg;		//Pointer to the config of UartPs
	XUartPs *UartPs;			//Pointer to UartPs instance
} controllers;

/***************************************
 * Variable definitions
 ***************************************/
extern controllers *components;
/***************************************
 * Function prototypes
 ***************************************/
int init_platform(controllers *components);			//Initialization routine
int initInterrupt(controllers *components);			//Initialization routine with interrupts
int initDMA(controllers *components);				//Initialization routine for the DMA
int initUart(controllers *components);				//Initialization routine for the UartPs

void VSyncIntrHandler(void *Callback);				//ISR for VSYNC
void HSyncIntrHandler(void *Callback);				//ISR for HSYNC
int ReadRx(u8 *Addr, u8 Length);					//Function that reads values from Addr of specified Length
int LoadTx(u8 *Addr, u8 *ValueAddr, u8 Length);		//Function that writes Value to Addr of specified Length
int XAxiDma_Send_Array(XAxiDma *myDma, UINTPTR TxBA, UINTPTR RxBA, u32 tranLen);
void DisableIntrSystem(INTC *IntcInstancePtr);
u8 getChar(XUartPs *UartPs);
int dmaSend(XAxiDma *InstancePtr, UINTPTR BuffAddr, u32 Length, int Direction);
int dmaRead(u32 srcAddr, u32 length, controllers *components);

#endif		//End of protection macro