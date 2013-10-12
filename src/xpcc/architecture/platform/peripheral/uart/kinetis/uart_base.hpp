// coding: utf-8
/*
* Copyright (c) 2013, Roboterclub Aachen e.V.
* All Rights Reserved.
*
* The file is part for the xpcc library and is released under the
* 3-clause BSD license. See the file `LICENSE` for the full license
* governing this code.
*/
// ----------------------------------------------------------------------------

#ifndef XPCC_KINETIS_UART_BASE_HPP
#define XPCC_KINETIS_UART_BASE_HPP

#include <stdint.h>
#include "../../../device.hpp"

namespace xpcc
{
namespace kinetis
{
/**
 * Base class for the UART classes
 *
 * Provides some common enum that do not depend on the specific UART.
 *
 * @author Kevin Laeufer
 * @internal
 * @ingroup		kinetis
 */
class UartBase
{

public:
	enum class
	Interrupt : uint8_t
	{
		/// Call interrupt when transmit register is empty
		TxEmpty		= UART_C2_TIE_MASK,		// bit7
		/// Called when the byte was completely transmitted
		TxComplete	= UART_C2_TCIE_MASK,	// bit6
		/// Call interrupt when receiver is full
		RxFull		= UART_C2_RIE_MASK,		// bit5
		/// Call interrupt if a parity error was detected.
		ParityError 	= UART_C3_PEIE_MASK,// bit0
		/// Call interrupt if a framing error was detected.
		FramingError	= UART_C3_FEIE_MASK,// bit1
		/// Call interrupt if a noise error was detected.
		NoiseError 		= UART_C3_NEIE_MASK,// bit2
		/// Call interrupt if an overrun error was detected.
		OverrunError	= UART_C3_ORIE_MASK,// bit3
	};

protected:
	static constexpr uint8_t InterruptC2Mask = 
		UART_C2_TIE_MASK | UART_C2_TCIE_MASK | UART_C2_RIE_MASK;
	static constexpr uint8_t InterruptC3Mask = 
		UART_C3_PEIE_MASK | UART_C3_FEIE_MASK | UART_C3_NEIE_MASK | UART_C3_ORIE_MASK;

public:
	enum class
	InterruptFlag : uint8_t
	{
		/// Set if the transmit data register is empty.
		TxEmpty			= UART_S1_TDRE_MASK,
		/// Set if the transmission is complete.
		TxComplete		= UART_S1_TC_MASK,
		/// Set if the number of dataword in the rx buffer is >= RWFIFO
		/// thus as long as you are not using the hw FIFO you can use this
		/// just like a RxNotEmpty interrupt on other devices.
		RxFull			= UART_S1_RDRF_MASK,
		/// Set if a parity error was detected.
		ParityError 	= UART_S1_PF_MASK,
		/// Set if a framing error was detected.
		FramingError	= UART_S1_FE_MASK,
		/// Set if a noise error was detected.
		NoiseError 		= UART_S1_NF_MASK,
		/// Set if an overrun error was detected.
		OverrunError	= UART_S1_OR_MASK,
	};

	enum class
	Parity : uint8_t
	{
		Disabled 	= 0,
		Even 		= UART_C1_PE_MASK,
		Odd  		= UART_C1_PE_MASK | UART_C1_PT_MASK,
	};

	/// @name FIFO
	///@{

	#define UART_CFIFO_RXOFE_MASK	0x4u

	enum class
	FifoInterrupt : uint8_t
	{
		ReceiveOverflow		= UART_CFIFO_RXOFE_MASK,
		TransmitOverflow	= UART_CFIFO_TXOFE_MASK,
		ReceiveUnderflow	= UART_CFIFO_RXUFE_MASK,
	};

	#define UART_SFIFO_RXOF_MASK	0x4u

	enum class
	FifoInterruptFlag : uint8_t
	{
		ReceiveOverflow		= UART_SFIFO_RXOF_MASK,
		TransmitOverflow	= UART_SFIFO_TXOF_MASK,
		ReceiveUnderflow	= UART_SFIFO_RXUF_MASK,
	};

	enum class
	FifoStatusFlag : uint8_t
	{
		TransmitEmpty		= UART_SFIFO_TXEMPT_MASK,
		ReceiveEmpty		= UART_SFIFO_RXEMPT_MASK,
	};

	enum class
	TransmitFifoDepth : uint8_t
	{
		DataWord1   = 0b000,
		DataWord4   = 0b001 << UART_PFIFO_TXFIFOSIZE_SHIFT,
		DataWord8   = 0b010 << UART_PFIFO_TXFIFOSIZE_SHIFT,
		DataWord16  = 0b011 << UART_PFIFO_TXFIFOSIZE_SHIFT,
		DataWord32  = 0b100 << UART_PFIFO_TXFIFOSIZE_SHIFT,
		DataWord64  = 0b101 << UART_PFIFO_TXFIFOSIZE_SHIFT,
		DataWord128 = 0b110 << UART_PFIFO_TXFIFOSIZE_SHIFT,
	};

	enum class
	ReceiveFifoDepth : uint8_t
	{
		DataWord1   = 0b000,
		DataWord4   = 0b001 << UART_PFIFO_RXFIFOSIZE_SHIFT,
		DataWord8   = 0b010 << UART_PFIFO_RXFIFOSIZE_SHIFT,
		DataWord16  = 0b011 << UART_PFIFO_RXFIFOSIZE_SHIFT,
		DataWord32  = 0b100 << UART_PFIFO_RXFIFOSIZE_SHIFT,
		DataWord64  = 0b101 << UART_PFIFO_RXFIFOSIZE_SHIFT,
		DataWord128 = 0b110 << UART_PFIFO_RXFIFOSIZE_SHIFT,
	};

	///@}


};
ENUM_CLASS_FLAG(UartBase::Interrupt)
ENUM_CLASS_FLAG(UartBase::InterruptFlag)
}
}

#endif // XPCC_KINETIS_UART_BASE_HPP
