// coding: utf-8
/* Copyright (c) 2009, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_IODEVICE_WRAPPER_HPP
#define XPCC_IODEVICE_WRAPPER_HPP

#include <stdint.h>
#include <xpcc/architecture/driver/atomic/queue.hpp>
#include <xpcc/architecture/driver/byte_array.hpp>
#include "iodevice.hpp"
#include <cstring>


namespace xpcc
{

/// The preferred behavior when the IODevice buffer is full
/// @ingroup	io
enum class
IOBuffer
{
	DiscardIfFull,
	BlockIfFull
};

/**
 * Wrapper to use any peripheral device that supports static
 * write() and read() as an IODevice.
 *
 * You have to decide what happens when the device buffer is full
 * and you cannot write to it at the moment.
 * There are two options:
 *  1. busy wait until the buffer is free, or
 *  2. discard the bytes that cannot be written.
 *
 * Option 1 has the advantage, that none of your data will be lost,
 * however, busy-waiting can take a long time and can mess up your
 * program timings.
 * There is also a **high risk of deadlock**, when writing to a
 * IODevice inside of an interrupt and then busy-waiting forever
 * because the IODevice requires interrupts itself to send out
 * the data.
 *
 * It is therefore highly recommended to use option 2, where surplus
 * data will be discarded.
 * You should increase the IODevice buffer size, if you experience
 * missing data from your connection.
 * This behavior is also deadlock safe when called from inside another
 * interrupt, and your program timing is minimally affected (essentially
 * only coping data into the buffer).
 *
 * There is no default template argument, so that you hopefully make
 * a concious decision and be aware of this behavior.
 *
 * Example:
 * @code
 * // configure a UART
 * Uart0 uart;
 *
 * // wrap it into an IODevice
 * xpcc::IODeviceWrapper<Uart0, xpcc::IOBuffer::DiscardIfFull> device;
 *
 * // use this device to print a message
 * device.write("Hello");
 *
 * // or create a IOStream and use the stream to print something
 * xpcc::IOStream stream(device);
 * stream << " World!";
 * @endcode
 *
 * @ingroup		io
 * @tparam		Device		Peripheral which should be wrapped
 * @tparam		behavior	preferred behavior when the Device buffer is full
 */
template< class Device, IOBuffer behavior, std::size_t TxSize = 16, std::size_t RxSize = 1 >
class IODeviceWrapper : public IODevice
{
	typedef typename xpcc::tmp::Select< (TxSize >= 255),
			uint16_t,
			uint8_t >::Result Index;

public:
	/**
	 * Constructor
	 *
	 * @param	device	configured object
	 */
	IODeviceWrapper() :
		head(0), size(0), writingSize(0), scratchPointer(scratchBuffer, 0), status(0)
	{
		Device::attachWriteCompletionHandler( MakeFunction(this, &IODeviceWrapper::writeFinished) );
		Device::attachReadCompletionHandler(  MakeFunction(this, &IODeviceWrapper::readFinished)  );
	}
	IODeviceWrapper(const Device& /*device*/) :
		IODeviceWrapper()
	{
	}

	virtual void
	write(char c)
	{
		// this branch will be optimized away, since `behavior` is a template argument
		if (behavior == IOBuffer::DiscardIfFull)
		{
			push(c);
		}
		else
		{
			while( not push(c) )
				;
		}
	}

	virtual void
	write(const char *s)
	{
		write(s, std::strlen(s));
	}

	virtual void
	write(const char *s, std::size_t length)
	{
		if (length == 0) return;

		// this branch will be optimized away, since `behavior` is a template argument
		if (behavior == IOBuffer::DiscardIfFull)
		{
			if (status & IS_SCRATCH_BUFFER_WRITING)
				push(scratchPointer);
			push(ByteArray(reinterpret_cast<const uint8_t*>(s), length));
		}
		else
		{
			if (status & IS_SCRATCH_BUFFER_WRITING)
				while(not push(scratchPointer))
					;
			while( not push(ByteArray(reinterpret_cast<const uint8_t*>(s), length)) )
				;
		}
		status &= ~IS_SCRATCH_BUFFER_WRITING;
	}

	virtual void
	flush()
	{
		if (status & IS_SCRATCH_BUFFER_WRITING)
			push(scratchPointer);

		status &= ~IS_SCRATCH_BUFFER_WRITING;
	}

	virtual bool
	read(char& )
	{
		return false;
//		return Device::read(reinterpret_cast<uint8_t&>(c));
	}

private:
	void
	writeFinished()	// executed inside interrupt context!!!
	{
		if (status & WAS_PREVIOUSLY_READING_SCRATCH)
		{
			status &= ~WAS_PREVIOUSLY_READING_SCRATCH;
			size -= writingSize;
		}

		if (not txBuffer.isEmpty())
		{
			write(txBuffer.get());
			txBuffer.pop();
		}
		else status &= ~IS_BUSY_WRITING;
	}

	inline void
	write(ByteArray next)
	{
		const uint8_t *data = next.getData();
		std::size_t dataSize = next.getSize();

		// check if pointer is within scratch buffer
		if (scratchBuffer <= data and data < (scratchBuffer + scratchBufferSize))
		{
			writingSize = dataSize;
			status |= WAS_PREVIOUSLY_READING_SCRATCH;
		}

		Device::write(data, dataSize);
	}

	bool
	push(ByteArray next)
	{
		if (status & IS_BUSY_WRITING)
			return txBuffer.push(next);

		write(next);
		status |= IS_BUSY_WRITING;
		return true;
	}

	bool
	push(char c)
	{
		if (head >= scratchBufferSize)
		{
			if (behavior == IOBuffer::DiscardIfFull) {
				push(scratchPointer);
			} else {
				if (not push(scratchPointer))
					return false;
			}
			head = 0;
			status &= ~IS_SCRATCH_BUFFER_WRITING;
		}

		if (size >= scratchBufferSize)
			return false;

		if (not (status & IS_SCRATCH_BUFFER_WRITING))
		{
			scratchPointer.setData(scratchBuffer + head, 0);
			status |= IS_SCRATCH_BUFFER_WRITING;
		}

		scratchBuffer[head++] = uint8_t(c);
		size++;
		scratchPointer.setSize(scratchPointer.getSize() + 1);

		return true;
	}

	void
	readFinished()
	{

	}

private:
	xpcc::atomic::Queue<ByteArray, TxSize / 4> txBuffer;

	static constexpr std::size_t scratchBufferSize = TxSize;

	Index head;
	Index size;
	Index writingSize;

	ByteArray scratchPointer;

	enum
	{
		IS_BUSY_WRITING = (1 << 0),
		WAS_PREVIOUSLY_READING_SCRATCH = (1 << 1),
		IS_SCRATCH_BUFFER_WRITING = (1 << 2),
	};
	volatile uint8_t status;

	uint8_t scratchBuffer[scratchBufferSize];
};

}

#endif // XPCC_IODEVICE_WRAPPER_HPP
