/* Copyright (c) 2016, Roboterclub Aachen e. V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 * ------------------------------------------------------------------------- */

#ifndef COMPONENT__SENDER_HPP
#define COMPONENT__SENDER_HPP

#include <xpcc/communication/xpcc/abstract_component.hpp>
#include <xpcc/processing/timer.hpp>

#include "communication/communication.hpp"
#include "communication/packets.hpp"

namespace component
{
	class Sender : public xpcc::AbstractComponent
	{
	public:
		Sender(uint8_t id, xpcc::Dispatcher &communication);
		
		void
		update();
		
		void
		eventRobotLocation(const xpcc::Header& header,
			const robot::packet::Location *payload);

	private:
		void
		getPositionCallback(const xpcc::Header& header,
				const robot::packet::Position *parameter);
		
		xpcc::ResponseCallback positionCallback;
		xpcc::ShortPeriodicTimer timer;

		robot::packet::Location location;
	};
}

#endif // COMPONENT__SENDER_HPP
