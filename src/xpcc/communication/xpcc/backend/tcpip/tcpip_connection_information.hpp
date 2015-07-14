// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_TCPIP_CONNECTION_INFORMATION_HPP
#define XPCC_TCPIP_CONNECTION_INFORMATION_HPP

#include <boost/asio.hpp>
#include <ctime>
#include <string>
#include <stdint>

namespace xpcc
{

namespace tcpip
{

/**
 * This datatype is used to keep track about all alive client components.
 *
 * @author Thorsten Lajewski
 */
class ConnectionInformation
{
public:
	ConnectionInformation();

	~ConnectionInformation();

private:
	time_t lastPing;
	std::string ip;

	uint8_t typeId;

};

}	// namespace tcpip

}	// namespace xpcc

#endif // XPCC_TCPIP_CONNECTION_INFORMATION_HPP
