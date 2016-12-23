/* Copyright (c) 2016, Roboterclub Aachen e. V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 * ------------------------------------------------------------------------- */

#ifndef COMPONENT__RECEIVER_HPP
#define COMPONENT__RECEIVER_HPP

#include <xpcc/communication/xpcc/abstract_component.hpp>
#include <xpcc/processing/timer.hpp>

#include "communication/packets.hpp"

namespace component
{
	class Receiver : public xpcc::AbstractComponent
	{
	public:
		Receiver(uint8_t id, xpcc::Dispatcher &communication);
		
		void
		actionSetPosition(const xpcc::ResponseHandle& handle,
				const robot::packet::Position *parameter);
		
		void
		actionGetPosition(const xpcc::ResponseHandle& handle);
		
		void
		update();
		
	private:
		robot::packet::Position position;
		xpcc::ShortPeriodicTimer timer;
	};
}

#endif // COMPONENT__RECEIVER_HPP
