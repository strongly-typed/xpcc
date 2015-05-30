// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_INTERFACE_UART_HPP
#define XPCC_INTERFACE_UART_HPP

#include <xpcc/architecture/interface.hpp>
#include <xpcc/architecture/driver/function.hpp>

/**
 * @ingroup		interface
 * @defgroup	uart	Universal Asynchronous Receiver/Transmitter (UART)
 */

namespace xpcc
{

/**
 * Interface of an UART Peripheral.
 *
 * Asynchronous and synchronous access to the Uart.
 *
 * @author	Niklas Hauser
 * @ingroup	uart
 */
class Uart : public ::xpcc::Peripheral
{
public:
	/**
	 * Commonly used baudrates.
	 *
	 * Most Serial-to-USB converters only support baudrates up to 115200 Baud
	 */
	enum
	Baudrate : uint32_t
	{
#ifndef B300	// termios.h defines B300 .. B38400
		    B300 =     300,
		    B600 =     600,
		   B1200 =    1200,
		   B1800 =    1800,
		   B2400 =    2400,
		   B4800 =    4800,
		   B9600 =    9600,
		  B14400 =   14400,
		  B19200 =   19200,
		  B28800 =   28800,
		  B38400 =   38400,
		  B57600 =   57600,
		  B76800 =   76800,
		 B115200 =  115200,
		 B230400 =  230400,
		 B250000 =  250000,
		 B460800 =  460800,
		 B500000 =  500000,
		 B921600 =  921600,
		B1000000 = 1000000
#endif
	};

	/// Transfer completion handler signature is very simple.
	typedef Function0<void> CompletionHandler;

#ifdef __DOXYGEN__
public:
	/**
	 * Initializes the hardware and sets the baudrate.
	 *
	 * @tparam	clockSource
	 * 		the currently active system clock
	 * @tparam	baudrate
	 *		desired baud rate in Hz
	 * @tparam	tolerance
	 * 		the allowed absolute tolerance for the resulting baudrate
	 */
	template< class clockSource, uint32_t baudrate,
			uint16_t tolerance = Tolerance::OnePercent >
	static void
	initialize();

	/**
	 * Writes a block of data and calls the completion handler when done.
	 *
	 * @param[in]	data
	 *		Pointer to a data buffer with the bytes to send
	 * @param		length
	 * 		Number of bytes to be written.
	 *
	 * @retval	true	if data buffer accepted
	 * @retval	false	if a previous transfer is ongoing
	 */
	static bool
	write(const uint8_t *data, std::size_t length);

	/// @retval	true	if write transmission finished
	/// @retval	false	if write transmission is ongoing
	static bool
	isWriteFinished();

	/// stops sending, discards the write buffer and calls completion handler.
	/// @return	the number of bytes discarded
	static std::size_t
	discardWriteBuffer();

	/// The handler gets called when the previos write operation finished.
	static void
	attachWriteCompletionHandler(CompletionHandler handler);

	/**
	 * Reads a block of data and calls the completion handler when done.
	 *
	 * @warning	If no buffer is set, or the buffer is full, received data is **discarded**!!
	 *
	 * @param[out]	data
	 *		Pointer to a buffer big enough to store `length` bytes
	 * @param		length
	 *		Number of bytes to be read
	 *
	 * @retval	true	if data buffer accepted
	 * @retval	false	if a previous transfer is ongoing
	 */
	static bool
	read(uint8_t *data, std::size_t length);

	/// @retval	true	if read transmission finished
	/// @retval	false	if read transmission is ongoing
	static bool
	isReadFinished();

	/// stops receiving, discards the read buffer and calls completion handler.
	/// @return	the number of bytes discarded
	static std::size_t
	discardReadBuffer();

	/// The handler gets called when the previously set read buffer is full.
	static void
	attachReadCompletionHandler(CompletionHandler handler);
#endif
};

}	// namespace xpcc

#endif // XPCC_INTERFACE_UART_HPP
