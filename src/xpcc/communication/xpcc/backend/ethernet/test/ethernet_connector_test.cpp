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

uint8_t
lut(const uint8_t component) {
	// XPCC_LOG_DEBUG.printf("Componten = %02x\n", component);
	switch (component)
	{
		case 0x00:
		case 0x0a:
		case 0x34:
		case 0xcc:
		case 0xaa:
			return 0x12;
		case 0xce:
		case 0x78:
			return 0x9a;
	}
	return 0xff;
}

void
EthernetConnectorTest::testConversionToEthernetFrame()
{
	// xpcc::EthernetConnector connector;
	xpcc::Header header;
	xpcc::SmartPointer sp;

	xpcc::EthernetFrame ethFrame;

	// type(Type::REQUEST),	isAcknowledge(false), destination(0), source(0), packetIdentifier(0)
	header = xpcc::Header(xpcc::Header::Type::REQUEST, 
							/* ack */ false, /* dst */ 0x34, /* src */ 0x78, /* id */ 0x56);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(
		/* header */ header, /* payload */sp, lut,
		/* EthernetFrame */ ethFrame);

	xpcc::EthernetFrame expectedEthernetFrame;

	// clear
	memset(expectedEthernetFrame, 0, 64);

	expectedEthernetFrame[ 0] = 'R';
	expectedEthernetFrame[ 1] = 'C';
	expectedEthernetFrame[ 2] = 'A';
	expectedEthernetFrame[ 3] = 0x12; // dst container
	expectedEthernetFrame[ 4] = 0x34; // dst component
	expectedEthernetFrame[ 5] = 0x56; // packet id
	
	expectedEthernetFrame[ 6] = 'R';
	expectedEthernetFrame[ 7] = 'C';
	expectedEthernetFrame[ 8] = 'A';
	expectedEthernetFrame[ 9] = 0x9a; // src container
	expectedEthernetFrame[10] = 0x78; // src component
	expectedEthernetFrame[11] = 0x56; // packet id
	
	expectedEthernetFrame[12] = 0x82;
	expectedEthernetFrame[13] = 0x11;

	expectedEthernetFrame[14] = 0; // type
	expectedEthernetFrame[15] = 0; // ack
	expectedEthernetFrame[16] = 0; // size

	// No payload
	// expectedEthernetFrame.buffer[18];

	//  TEST_ASSERT_EQUALS_ARRAY(array1, array2, count, start)
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::RESPONSE, false, 0x34, 0x78, 0x56);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, lut, ethFrame);
	expectedEthernetFrame[14] = 1; // type = RESPONSE
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::NEGATIVE_RESPONSE, false, 0x34, 0x78, 0x56);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, lut, ethFrame);
	expectedEthernetFrame[14] = 2; // type = NEGATIVE_RESPONSE
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0x34, 0x78, 0x56);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, lut, ethFrame);
	expectedEthernetFrame[14] = 0; // type = REQUEST
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, true, 0x34, 0x78, 0x56);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, lut, ethFrame);
	expectedEthernetFrame[15] = 1; // ack
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0xaa, 0x78, 0x56);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, lut, ethFrame);
	expectedEthernetFrame[ 4] = 0xaa; // dst
	expectedEthernetFrame[15] = 0; // not ack
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0xcc, 0x78, 0x56);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, lut, ethFrame);
	expectedEthernetFrame[ 4] = 0xcc; // dst
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0x0a, 0x78, 0x56);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, lut, ethFrame);
	expectedEthernetFrame[ 4] = 0x0a; // dst
	TEST_ASSERT_EQUALS_ARRAY(expectedEthernetFrame, ethFrame, 64, 0);

	header = xpcc::Header(xpcc::Header::Type::REQUEST, false, 0x00, 0xce, 0x56);
	xpcc::XpccOverEthernet::ethernetFrameFromXpccPacket(header, sp, lut, ethFrame);
	expectedEthernetFrame[    4] = 0x00; // dst
	expectedEthernetFrame[6 + 4] = 0xce; // src
	expectedEthernetFrame[6 + 3] = 0x9a;
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
	ethernetFrame[ 4] = 0x00; // dst
	ethernetFrame[ 5] = 0x00;
	
	ethernetFrame[ 6] = 0x8e;
	ethernetFrame[ 7] = 'R';
	ethernetFrame[ 8] = 'C';
	ethernetFrame[ 9] = 'A';
	ethernetFrame[10] = 0x00; // src
	ethernetFrame[11] = 0x00;
	
	ethernetFrame[12] = 0x82;
	ethernetFrame[13] = 0x11;

	ethernetFrame[14] = 0; // type
	ethernetFrame[15] = 0; // ack
	ethernetFrame[16] = 0; // size

	/* Check Header Extraction from Ethernet Frame */
	xpcc::Header header;
	uint8_t size = xpcc::XpccOverEthernet::xpccPacketHeaderFromEthernetFrame(ethernetFrame, header);
	TEST_ASSERT_EQUALS(size, 0);
	TEST_ASSERT_EQUALS(header, xpcc::Header(xpcc::Header::Type::REQUEST, false, 0, 0, 0));

	/* Check Payload Reconstruciton from Ethernet Frame */
	ethernetFrame[16] = 3;
	ethernetFrame[18] = 0x82;
	ethernetFrame[19] = 0x11;
	ethernetFrame[20] = 0x22;

	size = xpcc::XpccOverEthernet::xpccPacketHeaderFromEthernetFrame(ethernetFrame, header);
	TEST_ASSERT_EQUALS(size, 3);

	xpcc::SmartPointer payload = xpcc::SmartPointer(size);
	std::memcpy(payload.getPointer(), ethernetFrame + 18, size);
	uint8_t expected_payload[] = {0x82, 0x11, 0x22};
	TEST_ASSERT_EQUALS_ARRAY(payload.getPointer(), expected_payload, 3, 0);
}
