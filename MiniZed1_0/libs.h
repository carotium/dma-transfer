#pragma once
#ifndef LIBS_H		//Protection macro
#define LIBS_H

/***************************************
 * Include files
 ***************************************/
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
//Standard library
#include "stdio.h"
#include "stdlib.h"

#include "xtime_l.h"

//VGA font library
#include "IBM_VGA_8x16.h"

/***************************************
 * Macros
 ***************************************/
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
//How many bytes in dataArray to go one pixel down
#define PITCH 			2560
//How many bytes in dataArray to next pixel
#define PIXEL_WIDTH		4
//How many pixels for a character
#define CHAR_WIDTH 8

//Directly after DMA MM2S and S2MM interrupt IDs (61U, 62U)
//currently only MM2S interrupt is connected
#define XPAR_FABRIC_HSYNC_INTROUT_VEC_ID 63U
#define XPAR_FABRIC_VSYNC_INTROUT_VEC_ID 64U
#define XPAR_FABRIC_FIFO_EMPTY_INTROUT_VEC_ID 65U
#define XPAR_FABRIC_FIFO_FULL_INTROUT_VEC_ID 66U

/***************************************
 * Interrupt IDs
 ***************************************/
//#define RX_INTR_ID			XPAR_FABRIC_AXIDMA_1_VEC_ID
#define TX_INTR_ID			XPAR_FABRIC_AXIDMA_0_VEC_ID
#define HSYNC_INTR_ID		XPAR_FABRIC_HSYNC_INTROUT_VEC_ID
#define VSYNC_INTR_ID		XPAR_FABRIC_VSYNC_INTROUT_VEC_ID
#define FIFO_EMPTY_INTR_ID	XPAR_FABRIC_FIFO_EMPTY_INTROUT_VEC_ID
#define FIFO_FULL_INTR_ID	XPAR_FABRIC_FIFO_FULL_INTROUT_VEC_ID

/***************************************
 * Device IDs
 ***************************************/
#define DMA_DEV_ID			XPAR_AXIDMA_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC				XScuGic
#define INTC_HANDLER		XScuGic_InterruptHandler
#define UART_DEVICE_ID      XPAR_XUARTPS_1_DEVICE_ID

//DDR Base Address, in xparameters.h
#define DDR_BASE_ADDR		XPAR_PS7_DDR_0_S_AXI_BASEADDR
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x01000000)

#ifndef DEBUG
extern void xil_printf(const char *format, ...);
#endif

/***************************************
 * Enum definitions
 ***************************************/
typedef enum colors {
	black	=	0x0,
	blue	=	0x0A0000,
	green	=	0x000A00,
	cyan	=	0x0A0A00,
	red		=	0x00000A,
	purple	=	0x0A000A,
	brown	=	0x00050A,
	gray	=	0x0A0A0A,
	d_gray	=	0x050505,
	l_blue	=	0x0F0505,
	l_green	=	0x050F05,
	l_cyan	=	0x0F0F05,
	l_red	=	0x05050F,
	l_purple=	0x0F050F,
	yellow	=	0x050F0F,
	white	=	0x0F0F0F
} colors;

/***************************************
 * Struct definitions
 ***************************************/
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

typedef struct selectorWText_t {
	point selector;
	point menu;
	const char *menuText;
} selectorWText;

/***************************************
 * Variable definitions
 ***************************************/
extern controllers *components;
extern u32 dataArray[SCREEN_HEIGHT][SCREEN_WIDTH];

extern u32 startX[256], endX[256], startY[256], endY[256];
extern s32 dx0, dx1, dy0, dy1;
extern u32 colorPalette[16];

extern selectorWText selectorWText1;
extern selectorWText selectorWText2;
extern selectorWText selectorWText3;
extern selectorWText selectorWText4;

extern u32 discovered;

/***************************************
 * Function prototypes
 ***************************************/
int initPlatform(controllers *components);			//Initialization routine
int initInterrupt(controllers *components);			//Initialization routine with interrupts
int initDMA(controllers *components);				//Initialization routine for the DMA
int initUART(controllers *components);				//Initialization routine for the UartPs
void enableInterrupts(controllers *components);		//Enabling interrupts

void TxIntrHandler(void *Callback);
void VSyncIntrHandler(void *Callback);				//ISR for VSYNC
void HSyncIntrHandler(void *Callback);				//ISR for HSYNC
void FifoEmptyHandler(void *Callback);				//ISR for fifo empty threshold = 256
void FifoFullHandler(void *Callback);				//ISR for fifo full threshold = 256

u8 getChar(XUartPs *UartPs);
int dmaSend(XAxiDma *InstancePtr, UINTPTR BuffAddr, u32 Length, int Direction);

int dmaReadReg(u32 *srcAddr, u32 length, controllers *components);

//Line drawing functions
void drawStraight(int x0, int y0, int x1, int x2, u32 color);
void drawBox(int x0, int y0, int x1, int y1, colors color);
void drawSelector(point tL, colors color);
void selectSelector(point selector);
void unselectSelector(point selector);
void unselectAll();
void selectSelectorWText(selectorWText selectorWText);
void selectMenu(selectorWText menuText);
void enterMenu(selectorWText selectorWText);
void drawSelectorWText(selectorWText selectorWText);
void clearVGA();
void enterEcho();

void drawLineB(u32 x0, u32 y0, u32 x1, u32 y1, u32 color);
void drawLines(u32 t);
void lineStart(int state);
void calculateLine(u32 t);
void eraseLineB(u32 x0, u32 y0, u32 x1, u32 y1);

//Character drawing functions
u8 *getLetter(u32 asciiNum, u32 scale);
void printLetter(u8 *character, u32 scale, colors color);
void eraseLetter(u32 scale);
void printVGA(const char* string, u32 scale, colors color);
void nextLine(u32 scale);
void nextTab(u32 scale);

void putPixel(u32 x, u32 y, colors color);
void drawChar(u8 c, u32 x, u32 y, u32 scale, colors fgcolor, colors bgcolor);
void drawText(const char* text, point textP, u32 scale, colors fgcolor, colors bgcolor);
void drawNextText(const char *text, u32 scale, colors fgcolor, colors bgcolor);
u32 getNextIndex();

//Exponentiation calculation
u32 power(u32 base, u32 power);

#endif		//End of protection macro
