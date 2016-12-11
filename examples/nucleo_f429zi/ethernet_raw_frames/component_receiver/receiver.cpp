/* Copyright (c) 2016, Roboterclub Aachen e. V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 * ------------------------------------------------------------------------- */

#include <xpcc/debug/logger.hpp>

// set new log level
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

#include "receiver.hpp"

// ----------------------------------------------------------------------------
component::Receiver::Receiver(uint8_t id, xpcc::Dispatcher &communication) :
	xpcc::AbstractComponent(id, communication)
{
}

// ----------------------------------------------------------------------------
void
component::Receiver::actionSetPosition(const xpcc::ResponseHandle&,
		const robot::packet::Position *parameter)
{
	XPCC_LOG_INFO << XPCC_FILE_INFO
			<< "action set position: x=" << parameter->x
			<< ", y=" << parameter->y << xpcc::endl;
	
	position = *parameter;
}

// ----------------------------------------------------------------------------
void
component::Receiver::actionGetPosition(const xpcc::ResponseHandle& handle)
{
	XPCC_LOG_INFO << XPCC_FILE_INFO << "action get position" << xpcc::endl;
	
	this->sendResponse(handle, position);
}

// ----------------------------------------------------------------------------
void
component::Receiver::update()
{
}
