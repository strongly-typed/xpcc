// coding: utf-8
/* Copyright (c) 2016, Roboterclub Aachen e. V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef ETHERNET_CONNECTOR_TEST_HPP
#define ETHERNET_CONNECTOR_TEST_HPP

#include <unittest/testsuite.hpp>

class EthernetConnectorTest : public unittest::TestSuite
{
public:
	void
	testConversionToEthernetFrame();
	
	void
	testConversionToXpccPacket();
};

#endif // ETHERNET_CONNECTOR_TEST_HPP
