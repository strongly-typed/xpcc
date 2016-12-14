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
	uint8_t container = 0x22;

	xpcc::EthernetFrame ethFrame;

	// type(Type::REQUEST),	isAcknowledge(false), destination(0), source(0), packetIdentifier(0)
	header = xpcc::Header(xpcc::Header::Type::REQUEST, /* ack */ false, /* dst */ 0, /* src */ 0, /* id */ 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(
		/* header */ header, /* container */ container, 
		/* payload */sp, /* EthernetFrame */ ethFrame);

	xpcc::EthernetFrame expectedEthernetFrame;

	// clear
	memset(expectedEthernetFrame, 0, 64);

	expectedEthernetFrame[ 0] = 0x8e;
	expectedEthernetFrame[ 1] = 'R';
	expectedEthernetFrame[ 2] = 'C';
	expectedEthernetFrame[ 3] = 'A';
	expectedEthernetFrame[ 4] = 0x22; // dst container
	expectedEthernetFrame[ 5] = 0x00; // dst component
	
	expectedEthernetFrame[ 6] = 0x8e;
	expectedEthernetFrame[ 7] = 'R';
	expectedEthernetFrame[ 8] = 'C';
	expectedEthernetFrame[ 9] = 'A';
	expectedEthernetFrame[10] = 0x00; // src container
	expectedEthernetFrame[11] = 0x00; // src component
	
	expectedEthernetFrame[12] = 0x82;
	expectedEthernetFrame[13] = 0x11;

	expectedEthernetFrame[14] = 0; // type
	expectedEthernetFrame[15] = 0; // ack
	expectedEthernetFrame[16] = 0; // id
	expectedEthernetFrame[17] = 0; // size

	// No payload
	// expectedEthernetFrame.buffer[18];

	//  TEST_ASSERT_EQUALS_ARRAY(array1, array2, count, start)
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::RESPONSE, false, 0, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[14] = 1; // type = RESPONSE
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::NEGATIVE_RESPONSE, false, 0, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[14] = 2; // type = NEGATIVE_RESPONSE
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[14] = 0; // type = REQUEST
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, true, 0, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[15] = 1; // ack
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0xff, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[ 5] = 0xff; // dst
	expectedEthernetFrame[15] = 0; // not ack
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0xcc, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[ 5] = 0xcc; // dst
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0x0a, 0, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[ 5] = 0x0a; // dst
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0xff, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[ 5] = 0x0; // dst
	expectedEthernetFrame[11] = 0xff; // src
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0xcc, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[11] = 0xcc; // src
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0x0a, 0);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[11] = 0x0a; // src
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0xff);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[11] = 0x0; // src
	expectedEthernetFrame[16] = 0xff; // packet identifier
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0xcc);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[16] = 0xcc; // packet identifier
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0x0a);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, container, sp, ethFrame);
	expectedEthernetFrame[16] = 0x0a; // packet identifier
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);
}

void EthernetConnectorTest::testConversionToXpccPacket()
{
	uint8_t ethernetFrame[64];

	// clear
	memset(ethernetFrame, 0, 64);

	ethernetFrame[ 0] = 0x8e;
	ethernetFrame[ 1] = 'R';
	ethernetFrame[ 2] = 'C';
	ethernetFrame[ 3] = 'A';
	ethernetFrame[ 4] = 0x00;
	ethernetFrame[ 5] = 0x00; // dst
	
	ethernetFrame[ 6] = 0x8e;
	ethernetFrame[ 7] = 'R';
	ethernetFrame[ 8] = 'C';
	ethernetFrame[ 9] = 'A';
	ethernetFrame[10] = 0x00;
	ethernetFrame[11] = 0x00; // src
	
	ethernetFrame[12] = 0x82;
	ethernetFrame[13] = 0x11;

	ethernetFrame[14] = 0; // type
	ethernetFrame[15] = 0; // ack
	ethernetFrame[16] = 0; // id
	ethernetFrame[17] = 0; // size




	xpcc::Header header;
	// xpcc::SmartPointer payload;
	uint8_t size = xpcc::XpccOverEthernet::xpccPacketHeaderFromEthernetFrame(ethernetFrame, header);
	TEST_ASSERT_EQUALS(size, 0);
	TEST_ASSERT_EQUALS(header, xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0));

	ethernetFrame[17] = 3;
	ethernetFrame[18] = 0x82;
	ethernetFrame[19] = 0x11;
	ethernetFrame[20] = 0x22;

	// xpcc::SmartPointer payload = xpcc::XpccOverEthernet::xpccPacketFromEthernetFrame(ethernetFrame, header);
	size = xpcc::XpccOverEthernet::xpccPacketHeaderFromEthernetFrame(ethernetFrame, header);
	TEST_ASSERT_EQUALS(header, xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0));
	TEST_ASSERT_EQUALS(size, 3);
	// uint8_t expected_payload[] = {0x82, 0x11, 0x22};
	// TEST_ASSERT_EQUALS_ARRAY(payload.getPointer(), expected_payload, 3, 0);

}
