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
	UART0_RX_TX_IRQn = 16,
	UART0_ERR_IRQn = 17,
	UART1_RX_TX_IRQn = 18,
	UART1_ERR_IRQn = 19,
	UART2_RX_TX_IRQn = 20,
	UART2_ERR_IRQn = 21,


/*
	DMA0_IRQn,
	DMA1_IRQn,
	DMA2_IRQn,
	DMA3_IRQn,
	DMA4_IRQn,
	DMA5_IRQn,
	DMA6_IRQn,
	DMA7_IRQn,
	DMA8_IRQn,
	DMA9_IRQn,
	DMA10_IRQn,
	DMA11_IRQn,
	DMA12_IRQn,
	DMA13_IRQn,
	DMA14_IRQn,
	DMA15_IRQn,
	DMA_Error_IRQn,
	MCM_IRQn,
	FTFL_IRQn,
	Read_Collision_IRQn,
	LVD_LVW_IRQn,
	LLW_IRQn,
	Watchdog_IRQn,
	Reserved39_IRQn,
	I2C0_IRQn,
	I2C1_IRQn,
	SPI0_IRQn,
	SPI1_IRQn,
	SPI2_IRQn,
	CAN0_ORed_Message_buffer_IRQn,
	CAN0_Bus_Off_IRQn,
	CAN0_Error_IRQn,
	CAN0_Tx_Warning_IRQn,
	CAN0_Rx_Warning_IRQn,
	CAN0_Wake_Up_IRQn,
	I2S0_Tx_IRQHandler_IRQn,
	I2S0_Rx_IRQHandler_IRQn,
	CAN1_ORed_Message_buffer_IRQn,
	CAN1_Bus_Off_IRQn,
	CAN1_Error_IRQn,
	CAN1_Tx_Warning_IRQn,
	CAN1_Rx_Warning_IRQn,
	CAN1_Wake_Up_IRQn,
	Reserved59_IRQn,
	Reserved60_IRQn,
	UART0_RX_TX_IRQn,
	UART0_ERR_IRQn,
	UART1_RX_TX_IRQn,
	UART1_ERR_IRQn,
	UART2_RX_TX_IRQn,
	UART2_ERR_IRQn,
	UART3_RX_TX_IRQn,
	UART3_ERR_IRQn,
	UART4_RX_TX_IRQn,
	UART4_ERR_IRQn,
	UART5_RX_TX_IRQn,
	UART5_ERR_IRQn,
	ADC0_IRQn,
	ADC1_IRQn,
	CMP0_IRQn,
	CMP1_IRQn,
	CMP2_IRQn,
	FTM0_IRQn,
	FTM1_IRQn,
	FTM2_IRQn,
	CMT_IRQn,
	RTC_IRQn,
	RTC_Seconds_IRQn,
	PIT0_IRQn,
	PIT1_IRQn,
	PIT2_IRQn,
	PIT3_IRQn,
	PDB0_IRQn,
	USB0_IRQn,
	USBDCD_IRQn,
	Reserved91_IRQn,
	Reserved92_IRQn,
	Reserved93_IRQn,
	Reserved94_IRQn,
	I2S0_IRQn,
	SDHC_IRQn,
	DAC0_IRQn,
	DAC1_IRQn,
	TSI0_IRQn,
	MCG_IRQn,
	LPTimer_IRQn,
	Reserved102_IRQn,
	PORTA_IRQn,
	PORTB_IRQn,
	PORTC_IRQn,
	PORTD_IRQn,
	PORTE_IRQn,
	Reserved108_IRQn,
	Reserved109_IRQn,
	Reserved110_IRQn,
	Reserved111_IRQn,
	Reserved112_IRQn,
	Reserved113_IRQn,
	Reserved114_IRQn,
	Reserved115_IRQn,
	Reserved116_IRQn,
	Reserved117_IRQn,
	Reserved118_IRQn,
	Reserved119_IRQn
*/
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
#include "headers/MK20DZ10.h"
