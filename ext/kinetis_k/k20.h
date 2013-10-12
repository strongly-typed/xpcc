/*
 * Because a complete CMSIS lib for the Freescale K20.
 * This is copied together from different CMSIS libs and
 * source cpde provided by Freescale.
 */

// From CMSIS
typedef enum IRQn
{
/******  Cortex-M# Processor Exceptions Numbers ******************************/

	NonMaskableInt_IRQn		= -14,		///<  2 Non Maskable Interrupt
	MemoryManagement_IRQn	= -12,		///<  4 Memory Management Interrupt
	BusFault_IRQn			= -11,		///<  5 Bus Fault Interrupt
	UsageFault_IRQn			= -10,		///<  6 Usage Fault Interrupt
	SVCall_IRQn				=  -5,		///< 11 SV Call Interrupt
	DebugMonitor_IRQn		=  -4,		///< 12 Debug Monitor Interrupt
	PendSV_IRQn				=  -2,		///< 14 Pend SV Interrupt
	SysTick_IRQn			=  -1,		///< 15 System Tick Interrupt

/******  Device Specific Interrupt Numbers ***********************************/
	DMA0_IRQn			=  0,
	DMA1_IRQn			=  1,
	DMA2_IRQn			=  2,
	DMA3_IRQn			=  3,
	DMA_Error_IRQn		=  4,
	FTFL_IRQn			=  6,
	Read_Collision_IRQn	=  7,
	LVD_LVW_IRQn		=  8,
	LLW_IRQn			=  9,
	Watchdog_IRQn		= 10,
	I2C0_IRQn			= 11,
	SPI0_IRQn			= 12,
	I2S0_Tx_IRQn		= 13,
	I2S0_Rx_IRQn		= 14,
	UART0_LON_IRQn		= 15,
	UART0_RX_TX_IRQn	= 16,
	UART0_ERR_IRQn		= 17,
	UART1_RX_TX_IRQn	= 18,
	UART1_ERR_IRQn		= 19,
	UART2_RX_TX_IRQn	= 20,
	UART2_ERR_IRQn		= 21,
	ADC0_IRQn			= 22,
	COMP0_IRQn			= 23,
	COMP1_IRQn			= 24,
	FTM0_IRQn			= 25,
	FTM1_IRQn			= 26,
	CMT_IRQn			= 27,
	RTC_IRQn			= 28,
	RTC_Seconds_IRQn	= 29,
	PIT0_IRQn			= 30,
	PIT1_IRQn			= 31,
	PIT2_IRQn			= 32,
	PIT3_IRQn			= 33,
	PDB0_IRQn			= 34,
	USB0_IRQn			= 35,
	USBDCD_IRQn			= 36,
	TSI0_IRQn			= 37,
	MCG_IRQn			= 38,
	LPTimer_IRQn		= 39,
	PORTA_IRQn			= 40,
	PORTB_IRQn			= 41,
	PORTC_IRQn			= 42,
	PORTD_IRQn			= 43,
	PORTE_IRQn			= 44,
	SWI_IRQn			= 45,
} IRQn_Type;

/**
 * @brief Configuration of the Cortex-M4 Processor and Core Peripherals 
 */
#define __CM4_REV                 0x0001  ///< Core revision r0p1 ??
#define __MPU_PRESENT             0       ///< no MPU??
#define __NVIC_PRIO_BITS          4       ///< K20  uses 4 Bits for the Priority Levels
#define __Vendor_SysTickConfig    0       ///< Set to 1 if different SysTick Config is used
#define __FPU_PRESENT             0       ///< FPU present: no

#include "core_cm4.h"	// Cortex-M4 processor and core peripherals
#include "headers/MK20D5.h"
