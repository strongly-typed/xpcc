// coding: utf-8
/* Copyright (c) 2016, Roboterclub Aachen e. V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

// #include "../connector.hpp"
#include "../xpcc_over_ethernet.hpp"
#include "ethernet_connector_test.hpp"

// ----------------------------------------------------------------------------
void
EthernetConnectorTest::testConversionToEthernetFrame()
{
	// xpcc::EthernetConnector connector;
	xpcc::Header header;
	xpcc::SmartPointer sp;

	xpcc::EthernetFrame ethFrame;

	TEST_ASSERT_TRUE(true);

	// type(Type::REQUEST),	isAcknowledge(false), destination(0), source(0), packetIdentifier(0)
	header = xpcc::Header(xpcc::Header::Type::REQUEST, /* ack */ false, /* dst */ 0, /* src */ 0, /* id */ 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(
		/* header */ header, /* payload */sp, /* EthernetFrame */ ethFrame);

	xpcc::EthernetFrame expectedEthernetFrame;

	// clear
	memset(expectedEthernetFrame.buffer, 0, 64);

	expectedEthernetFrame.buffer[ 0] = 0x8e;
	expectedEthernetFrame.buffer[ 1] = 'R';
	expectedEthernetFrame.buffer[ 2] = 'C';
	expectedEthernetFrame.buffer[ 3] = 'A';
	expectedEthernetFrame.buffer[ 4] = 0x00;
	expectedEthernetFrame.buffer[ 5] = 0x00; // dst
	
	expectedEthernetFrame.buffer[ 6] = 0x8e;
	expectedEthernetFrame.buffer[ 7] = 'R';
	expectedEthernetFrame.buffer[ 8] = 'C';
	expectedEthernetFrame.buffer[ 9] = 'A';
	expectedEthernetFrame.buffer[10] = 0x00;
	expectedEthernetFrame.buffer[11] = 0x00; // src
	
	expectedEthernetFrame.buffer[12] = 0x82;
	expectedEthernetFrame.buffer[13] = 0x11;

	expectedEthernetFrame.buffer[14] = 0; // type
	expectedEthernetFrame.buffer[15] = 0; // ack
	expectedEthernetFrame.buffer[16] = 0; // id
	expectedEthernetFrame.buffer[17] = 0; // size

	// No payload
	//  expectedEthernetFrame.buffer[18];

	//  TEST_ASSERT_EQUALS_ARRAY(array1, array2, count, start)
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::RESPONSE, false, 0, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[14] = 1; // type = RESPONSE
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::NEGATIVE_RESPONSE, false, 0, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[14] = 2; // type = NEGATIVE_RESPONSE
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[14] = 0; // type = REQUEST
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, true, 0, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[15] = 1; // ack
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0xff, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[ 5] = 0xff; // dst
	expectedEthernetFrame.buffer[15] = 0; // not ack
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0xcc, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[ 5] = 0xcc; // dst
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0x0a, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[ 5] = 0x0a; // dst
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0xff, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[ 5] = 0x0; // dst
	expectedEthernetFrame.buffer[11] = 0xff; // src
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0xcc, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[11] = 0xcc; // src
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0x0a, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[11] = 0x0a; // src
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0xff);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[11] = 0x0; // src
	expectedEthernetFrame.buffer[16] = 0xff; // packet identifier
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0xcc);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[16] = 0xcc; // packet identifier
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0x0a);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, ethFrame);
	expectedEthernetFrame.buffer[16] = 0x0a; // packet identifier
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame.buffer, ethFrame.buffer, 64, 0);
}

void EthernetConnectorTest::testConversionToXpccPacket()
{

}
