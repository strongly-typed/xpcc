// coding: utf-8
/* Copyright (c) 2015, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_BYTE_ARRAY_HPP
#define XPCC_BYTE_ARRAY_HPP

#include "accessor/flash.hpp"
#include <xpcc/architecture/utils.hpp>

namespace xpcc
{

// 4B on AVR, 8B on ARM
/// @author Niklas Hauser
class ATTRIBUTE_PACKED
ByteArray
{
public:
	ByteArray() :
		size(0), data(nullptr) {}

	ByteArray(const uint8_t *data, const std::size_t size) :
		size(size), data(const_cast<uint8_t*>(data)) {}

	ByteArray(uint8_t *data, const std::size_t size) :
		size(size), data(data) {}

	ByteArray(accessor::Flash<uint8_t> data, const std::size_t size) :
#ifdef XPCC__CPU_AVR
		size(size | 0x8000),
#else
		size(size),
#endif
		data(const_cast<uint8_t*>(data.getPointer())) {}

	inline void
	setSize(const std::size_t size)
	{
		this->size = size;
	}

	inline std::size_t
	getSize() const
	{
#ifdef XPCC__CPU_AVR
		return (size & 0x7FFF);
#else
		return size;
#endif
	}

	inline void
	setData(uint8_t *data, const std::size_t size)
	{
		this->data = data;
		this->size = size;
	}

	inline void
	setData(const uint8_t *data, const std::size_t size)
	{
		this->data = const_cast<uint8_t*>(data);
		this->size = size;
	}


	inline void
	setData(accessor::Flash<uint8_t> data, const std::size_t size)
	{
		this->data = const_cast<uint8_t*>(data.getPointer());
#ifdef XPCC__CPU_AVR
		this->size = size | 0x8000;
#else
		this->size = size;
#endif
	}

	inline uint8_t *
	getData() const
	{
		return data;
	}

	inline bool
	isFlash() const
	{
#ifdef XPCC__CPU_AVR
		return (size & 0x8000);
#else
		return false;
#endif
	}

private:
	std::size_t size;
	uint8_t *data;
};

}	// namespace xpcc

#endif	// XPCC_BYTE_ARRAY_HPP
