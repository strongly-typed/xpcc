#ifndef XPCC_ENC28J60_HPP
#   error  "Don't include this file directly, use 'enc28j60.hpp' instead!"
#endif

namespace xpcc
{

template < typename SPI, typename CS >
void
Enc28j60<SPI, CS>::initialize()
{
    writeOp(SpiOperationCodes::SOFT_RESET, 0, 0xff /* ENC28J60_SOFT_RESET */);

    // Delay of at least 1 ms after reset
    // Errata sheet Issue 2.
    // CLKRDY set too early after reset
    xpcc::delayMilliseconds(2);

    // check CLKRDY bit to see if reset is complete
    while(not (read(ESTAT) & ESTAT_CLKRDY));

    // do bank 0 stuff
    // initialize receive buffer
    // 16-bit transfers, must write low byte first
    // set receive buffer start address
    nextPacketPtr = RXSTART_INIT;
    write(ERXSTL, RXSTART_INIT & 0xFF);
    write(ERXSTH, RXSTART_INIT >> 8);
    XPCC_LOG_DEBUG.printf("ERXST = %02x %02x == [%02x %02x]\n", read(ERXSTH), read(ERXSTL), RXSTART_INIT >> 8, RXSTART_INIT & 0xFF);

    // set receive pointer address
    write(ERXRDPTL, RXSTART_INIT & 0xFF);
    write(ERXRDPTH, RXSTART_INIT >> 8);
    XPCC_LOG_DEBUG.printf("ERXRDPT = %02x %02x == [%02x %02x]\n", read(ERXRDPTH), read(ERXRDPTL), RXSTART_INIT >> 8, RXSTART_INIT & 0xFF);

    // set receive buffer end
    // ERXND defaults to 0x1FFF (end of ram)
    write(ERXNDL, RXSTOP_INIT & 0xFF);
    write(ERXNDH, RXSTOP_INIT >> 8);
    XPCC_LOG_DEBUG.printf("ERXND = %02x %02x = [%02x %02x]\n", read(ERXNDH), read(ERXNDL), RXSTOP_INIT >> 8, RXSTOP_INIT & 0xFF);

    // set transmit buffer start
    // ETXST defaults to 0x0000 (beginning of ram)
    write(ETXSTL, TXSTART_INIT & 0xFF);
    write(ETXSTH, TXSTART_INIT >> 8);
    XPCC_LOG_DEBUG.printf("ETXST = %02x %02x = [%02x %02x]\n", read(ETXSTH), read(ETXSTL), TXSTART_INIT >> 8, TXSTART_INIT & 0xFF);

    /* MAC initialization */
    // do bank 2 stuff
    XPCC_LOG_DEBUG.printf("1 MACON1 %02x, MACON2 %02x\n", read(MACON1), read(MACON2));

    // reset MAC
    write(MACON2, 0x83);
    xpcc::delayMilliseconds(100);

    XPCC_LOG_DEBUG.printf("2 MACON1 %02x, MACON2 %02x\n", read(MACON1), read(MACON2));

    // bring MAC out of reset
    write(MACON2, 0x05);
    xpcc::delayMilliseconds(100);


    XPCC_LOG_DEBUG.printf("3 MACON1 %02x, MACON2 %02x\n", read(MACON1), read(MACON2));

    xpcc::delayMilliseconds(500);

    // bring MAC out of reset
    write(MACON2, 0x00);

    // enable MAC receive
    write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);

    // enable automatic padding and CRC operations
    // Fixme: Does BFS work on MACON3?
    // Documentation says it does not work

    XPCC_LOG_DEBUG.printf("4 MACON3 %02x\n", read(MACON3));

    writeOp(SpiOperationCodes::BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);

    XPCC_LOG_DEBUG.printf("5 MACON3 %02x\n", read(MACON3));

    // set inter-frame gap (non-back-to-back)
    write(MAIPGL, 0x12);
    write(MAIPGH, 0x0C);

    // set inter-frame gap (back-to-back)
    write(MABBIPG, 0x12);

    // Set the maximum packet size which the controller will accept
    write(MAMXFLL, MAX_FRAMELEN & 0xFF);
    write(MAMXFLH, MAX_FRAMELEN >> 8);
    XPCC_LOG_DEBUG.printf("MAMXFL = %02x %02x == [%02x %02x]\n", read(MAMXFLH), read(MAMXFLL), MAX_FRAMELEN >> 8, MAX_FRAMELEN & 0xFF);


    // do bank 3 stuff

    // write MAC address
    // NOTE: MAC address in ENC28J60 is byte-backward
    write(MAADR5, ENC28J60_MAC0);
    write(MAADR4, ENC28J60_MAC1);
    write(MAADR3, ENC28J60_MAC2);
    write(MAADR2, ENC28J60_MAC3);
    write(MAADR1, ENC28J60_MAC4);
    write(MAADR0, ENC28J60_MAC5);

    XPCC_LOG_DEBUG.printf("MAC readback = %02x %02x %02x %02x %02x %02x\n", read(MAADR0), read(MAADR1), read(MAADR2), read(MAADR3), read(MAADR4), read(MAADR5));

    // no loopback of transmitted frames
    phyWrite(PHCON2, PHCON2_HDLDIS);

    // switch to bank 0
    setBank(ECON1);

    // enable interrutps
    writeOp(SpiOperationCodes::BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);

    // enable packet reception
    writeOp(SpiOperationCodes::BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

template < typename SPI, typename CS >
uint16_t
Enc28j60<SPI, CS>::receivePacket(uint16_t maxlen, uint8_t* packet)
{
    uint16_t rxstat;

    // check if a packet has been received and buffered
    // Errata: The Receive Packet Pending Interrupt Flag (EIR.PKTIF)
    // does not reliably/accurately report the status of pending packets.
    // If polling to see if a packet is pending, check the value in EPKTCNT.
    if( read(EPKTCNT) == 0 ) {
        return 0;
    }

    // Make absolutely certain that any previous packet was discarded
    //if( WasDiscarded == FALSE)
    //  MACDiscardRx();

    // Set the read pointer to the start of the received packet
    write(ERDPTL, (nextPacketPtr));
    write(ERDPTH, (nextPacketPtr) >> 8);
    XPCC_LOG_DEBUG.printf("Receive Packet: Write nextPacketPtr = %04x\n", nextPacketPtr);

    // read the next packet pointer
    nextPacketPtr  = readOp(SpiOperationCodes::READ_BUF_MEM, 0);
    nextPacketPtr |= readOp(SpiOperationCodes::READ_BUF_MEM, 0) << 8;
    XPCC_LOG_DEBUG.printf("Receive Packet: Read  nextPacketPtr = %04x\n", nextPacketPtr);


    // read the packet length
    uint16_t len  = 0;
    len  = readOp(SpiOperationCodes::READ_BUF_MEM, 0);
    len |= readOp(SpiOperationCodes::READ_BUF_MEM, 0) << 8;
    XPCC_LOG_DEBUG.printf("Receive Packet: Len = %4d\n", len);

    // read the receive status
    rxstat  = readOp(SpiOperationCodes::READ_BUF_MEM, 0);
    rxstat |= readOp(SpiOperationCodes::READ_BUF_MEM, 0) << 8;

    // limit retrieve length
    // (we reduce the MAC-reported length by 4 to remove the CRC)
    len -= 4;
    if (len >= maxlen) {
        len = maxlen;
    }

    // copy the packet from the receive buffer
    readBuffer(len, packet);

    // Move the RX read pointer to the start of the next received packet
    // This frees the memory we just read out
    write(ERXRDPTL, (nextPacketPtr));
    write(ERXRDPTH, (nextPacketPtr) >> 8);
    XPCC_LOG_DEBUG.printf("Receive Packet: Write nextPacketPtr = %04x\n", nextPacketPtr);

    // decrement the packet counter indicate we are done with this packet
    writeOp(SpiOperationCodes::BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

    return len;
}


template < typename SPI, typename CS >
void
Enc28j60<SPI, CS>::sendPacket(uint16_t len, uint8_t* packet)
{
    // Errata: Transmit Logic reset
    writeOp(SpiOperationCodes::BIT_FIELD_SET, ECON1, ECON1_TXRST);
    writeOp(SpiOperationCodes::BIT_FIELD_CLR, ECON1, ECON1_TXRST);

    // Set the write pointer to start of transmit buffer area
    write(EWRPTL, TXSTART_INIT & 0xff);
    write(EWRPTH, TXSTART_INIT >> 8);
    XPCC_LOG_DEBUG.printf("Send Packet: Write Pointer Start = %04x\n", TXSTART_INIT);

    // Set the TXND pointer to correspond to the packet size given
    uint16_t txStartEnd = TXSTART_INIT + len;
    write(ETXNDL, (txStartEnd));
    write(ETXNDH, (txStartEnd) >> 8);
    XPCC_LOG_DEBUG.printf("Send Packet: Write Pointer End = %04x\n", txStartEnd);

    // write per-packet control byte
    writeOp(SpiOperationCodes::WRITE_BUF_MEM, 0, 0x00);

    // copy the packet into the transmit buffer
    writeBuffer(len, packet);

    // send the contents of the transmit buffer onto the network
    writeOp(SpiOperationCodes::BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

#define DUMP(REG)  XPCC_LOG_DEBUG.printf(#REG); XPCC_LOG_DEBUG.printf(" %02x %02x\n", read(REG ## H), read(REG ## L))
#define DUMP8(REG) XPCC_LOG_DEBUG.printf(#REG); XPCC_LOG_DEBUG.printf(" %02x\n", read(REG))

template < typename SPI, typename CS >
void
Enc28j60<SPI, CS>::dumpRegisters()
{
    XPCC_LOG_DEBUG.printf("Bank independent Status\n");
    DUMP8(EIE);
    DUMP8(EIR);
    DUMP8(ESTAT);
    DUMP8(ECON2);
    DUMP8(ECON1);

    XPCC_LOG_DEBUG.printf("Bank 0\n");
    DUMP(ERDPT);
    DUMP(EWRPT);
    DUMP(ETXST);
    DUMP(ETXND);
    DUMP(ERXST);
    DUMP(ERXND);
    DUMP(ERXRDPT);
    DUMP(ERXWRPT);
    DUMP(EDMAST);
    DUMP(EDMAND);
    DUMP(EDMADST);
    DUMP(EDMACS);

    XPCC_LOG_DEBUG.printf("Bank 1\n");
    DUMP8(EHT0);
    DUMP8(EHT1);
    DUMP8(EHT2);
    DUMP8(EHT3);
    DUMP8(EHT4);
    DUMP8(EHT5);
    DUMP8(EHT6);
    DUMP8(EHT7);
    DUMP8(EPMM0);
    DUMP8(EPMM1);
    DUMP8(EPMM2);
    DUMP8(EPMM3);
    DUMP8(EPMM4);
    DUMP8(EPMM5);
    DUMP8(EPMM6);
    DUMP8(EPMM7);
    DUMP(EPMO);
    DUMP8(EWOLIE);
    DUMP8(EWOLIR);
    DUMP8(ERXFCON);
    DUMP8(EPKTCNT);

    XPCC_LOG_DEBUG.printf("Bank 2\n");
    DUMP8(MACON1);
    DUMP8(MACON2);
    DUMP8(MACON3);
    DUMP8(MACON4);
    DUMP8(MABBIPG);
    DUMP(MAIPG);
    DUMP8(MACLCON1);
    DUMP8(MACLCON2);
    DUMP(MAMXFL);
    DUMP8(MAPHSUP);
    DUMP8(MICON);
    DUMP8(MICMD);
    DUMP8(MIREGADR);
    DUMP(MIWR);
    DUMP(MIRD);

    XPCC_LOG_DEBUG.printf("Bank 3\n");
    DUMP8(MAADR0);
    DUMP8(MAADR1);
    DUMP8(MAADR2);
    DUMP8(MAADR3);
    DUMP8(MAADR4);
    DUMP8(MAADR5);
    DUMP8(EBSTSD);
    DUMP8(EBSTCON);
    DUMP(EBSTCS);
    DUMP8(MISTAT);
}

} // xpcc namespace
