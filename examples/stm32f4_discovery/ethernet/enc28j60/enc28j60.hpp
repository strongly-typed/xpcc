#ifndef XPCC_ENC28J60_HPP
#define XPCC_ENC28J60_HPP

#include <stdint.h>
#include <xpcc/architecture/driver/delay.hpp>
#include <xpcc/debug/logger.hpp>

namespace xpcc
{

// Own MAC address
static constexpr uint8_t ENC28J60_MAC0 = 0xab;
static constexpr uint8_t ENC28J60_MAC1 = 0xbc;
static constexpr uint8_t ENC28J60_MAC2 = 0x6f;
static constexpr uint8_t ENC28J60_MAC3 = 0x55;
static constexpr uint8_t ENC28J60_MAC4 = 0x1c;
static constexpr uint8_t ENC28J60_MAC5 = 0xc3;

template < typename SPI, typename CS >
class Enc28j60
{
public:
	static void
	initialize();

	static uint16_t
	receivePacket(uint16_t maxlen, uint8_t* packet);

    static void
    sendPacket(uint16_t len, uint8_t* packet);

protected:
	static constexpr uint8_t ADDR_MASK = 0x1F;
	static constexpr uint8_t BANK_MASK = 0x60;
	static constexpr uint8_t SPRD_MASK = 0x80;

	// SPI operation codes
    // Table 4-1 on page 26
    struct SpiOperationCodes
    {
        enum
        {
            READ_CTRL_REG  = 0x00,
            READ_BUF_MEM   = 0x3A,
            WRITE_CTRL_REG = 0x40,
            WRITE_BUF_MEM  = 0x7A,
            BIT_FIELD_SET  = 0x80,
            BIT_FIELD_CLR  = 0xA0,
            SOFT_RESET     = 0xFF,
        };
    };

    // buffer boundaries applied to internal 8K RAM
    // entire available packet buffer space is allocated
   	static constexpr uint16_t MAX_RAM = 0x1fff; // Highest address in 8 KiByte RAM

   	// Errata: use the range (0000h to n) for the receive buffer and
   	// ((n + 1) to 8191) for the transmit buffer.
    static constexpr uint16_t RXSTART_INIT = 0x0000;	// Start the receive buffer at 0
    static constexpr uint16_t RXSTOP_INIT  = 0x19ff;	// Leave space for TX Buffer for one full Ethernet frame (~1500 bytes)
    static constexpr uint16_t TXSTART_INIT = 0x1A00;	// Place the TX buffer after the receive buffer

    static constexpr uint16_t MAX_FRAMELEN = 1518;	// maximum ethernet frame length

    // All-bank registers
	static constexpr uint8_t EIE   = 0x1B;
	static constexpr uint8_t EIR   = 0x1C;
	static constexpr uint8_t ESTAT = 0x1D;
	static constexpr uint8_t ECON2 = 0x1E;
	static constexpr uint8_t ECON1 = 0x1F;

    static constexpr uint8_t BANK0 = 0x00;
    static constexpr uint8_t BANK1 = 0x30;
    static constexpr uint8_t BANK2 = 0x40;
    static constexpr uint8_t BANK3 = 0x60;

    // Bank 0 registers
    //                                     Addr Bank
    static constexpr uint8_t  ERDPTL	= (0x00|BANK0);
    static constexpr uint8_t  ERDPTH	= (0x01|BANK0);
    static constexpr uint8_t  EWRPTL	= (0x02|BANK0);
    static constexpr uint8_t  EWRPTH	= (0x03|BANK0);
    static constexpr uint8_t  ETXSTL	= (0x04|BANK0);
    static constexpr uint8_t  ETXSTH	= (0x05|BANK0);
    static constexpr uint8_t  ETXNDL	= (0x06|BANK0);
    static constexpr uint8_t  ETXNDH	= (0x07|BANK0);
    static constexpr uint8_t  ERXSTL	= (0x08|BANK0);
    static constexpr uint8_t  ERXSTH	= (0x09|BANK0);
    static constexpr uint8_t  ERXNDL	= (0x0A|BANK0);
    static constexpr uint8_t  ERXNDH	= (0x0B|BANK0);
    static constexpr uint8_t  ERXRDPTL	= (0x0C|BANK0);
    static constexpr uint8_t  ERXRDPTH	= (0x0D|BANK0);
    static constexpr uint8_t  ERXWRPTL	= (0x0E|BANK0);
    static constexpr uint8_t  ERXWRPTH	= (0x0F|BANK0);
    static constexpr uint8_t  EDMASTL	= (0x10|BANK0);
    static constexpr uint8_t  EDMASTH	= (0x11|BANK0);
    static constexpr uint8_t  EDMANDL	= (0x12|BANK0);
    static constexpr uint8_t  EDMANDH	= (0x13|BANK0);
    static constexpr uint8_t  EDMADSTL	= (0x14|BANK0);
    static constexpr uint8_t  EDMADSTH	= (0x15|BANK0);
    static constexpr uint8_t  EDMACSL	= (0x16|BANK0);
    static constexpr uint8_t  EDMACSH	= (0x17|BANK0);

    // Bank 1 registers
    //                                     Addr Bank
    static constexpr uint8_t EHT0		= (0x00|BANK1);
    static constexpr uint8_t EHT1		= (0x01|BANK1);
    static constexpr uint8_t EHT2		= (0x02|BANK1);
    static constexpr uint8_t EHT3		= (0x03|BANK1);
    static constexpr uint8_t EHT4		= (0x04|BANK1);
    static constexpr uint8_t EHT5		= (0x05|BANK1);
    static constexpr uint8_t EHT6		= (0x06|BANK1);
    static constexpr uint8_t EHT7		= (0x07|BANK1);
    static constexpr uint8_t EPMM0		= (0x08|BANK1);
    static constexpr uint8_t EPMM1		= (0x09|BANK1);
    static constexpr uint8_t EPMM2		= (0x0A|BANK1);
    static constexpr uint8_t EPMM3		= (0x0B|BANK1);
    static constexpr uint8_t EPMM4		= (0x0C|BANK1);
    static constexpr uint8_t EPMM5		= (0x0D|BANK1);
    static constexpr uint8_t EPMM6		= (0x0E|BANK1);
    static constexpr uint8_t EPMM7		= (0x0F|BANK1);
    static constexpr uint8_t EPMCSL		= (0x10|BANK1);
    static constexpr uint8_t EPMCSH		= (0x11|BANK1);
    static constexpr uint8_t EPMOL		= (0x14|BANK1);
    static constexpr uint8_t EPMOH		= (0x15|BANK1);
    static constexpr uint8_t EWOLIE		= (0x16|BANK1);
    static constexpr uint8_t EWOLIR		= (0x17|BANK1);
    static constexpr uint8_t ERXFCON	= (0x18|BANK1);
    static constexpr uint8_t EPKTCNT	= (0x19|BANK1);

    // Bank 2 registers
    //                                     Addr Bank Dummy Read Needed
    static constexpr uint8_t MACON1		= (0x00|BANK2|0x80);
    static constexpr uint8_t MACON2		= (0x01|BANK2|0x80);
    static constexpr uint8_t MACON3		= (0x02|BANK2|0x80);
    static constexpr uint8_t MACON4		= (0x03|BANK2|0x80);
    static constexpr uint8_t MABBIPG	= (0x04|BANK2|0x80);
    static constexpr uint8_t MAIPGL		= (0x06|BANK2|0x80);
    static constexpr uint8_t MAIPGH		= (0x07|BANK2|0x80);
    static constexpr uint8_t MACLCON1	= (0x08|BANK2|0x80);
    static constexpr uint8_t MACLCON2	= (0x09|BANK2|0x80);
    static constexpr uint8_t MAMXFLL	= (0x0A|BANK2|0x80);
    static constexpr uint8_t MAMXFLH	= (0x0B|BANK2|0x80);
    static constexpr uint8_t MAPHSUP	= (0x0D|BANK2|0x80);
    static constexpr uint8_t MICON		= (0x11|BANK2|0x80);
    static constexpr uint8_t MICMD		= (0x12|BANK2|0x80);
    static constexpr uint8_t MIREGADR	= (0x14|BANK2|0x80);
    static constexpr uint8_t MIWRL		= (0x16|BANK2|0x80);
    static constexpr uint8_t MIWRH		= (0x17|BANK2|0x80);
    static constexpr uint8_t MIRDL		= (0x18|BANK2|0x80);
    static constexpr uint8_t MIRDH		= (0x19|BANK2|0x80);

    // Bank 3 registers
    //                                     Addr Bank Dummy Read Needed
    static constexpr uint8_t MAADR1		= (0x00|BANK3|0x80);
    static constexpr uint8_t MAADR0		= (0x01|BANK3|0x80);
    static constexpr uint8_t MAADR3		= (0x02|BANK3|0x80);
    static constexpr uint8_t MAADR2		= (0x03|BANK3|0x80);
    static constexpr uint8_t MAADR5		= (0x04|BANK3|0x80);
    static constexpr uint8_t MAADR4		= (0x05|BANK3|0x80);
    static constexpr uint8_t EBSTSD		= (0x06|BANK3);
    static constexpr uint8_t EBSTCON	= (0x07|BANK3);
    static constexpr uint8_t EBSTCSL	= (0x08|BANK3);
    static constexpr uint8_t EBSTCSH	= (0x09|BANK3);
    static constexpr uint8_t MISTAT		= (0x0A|BANK3|0x80);
    static constexpr uint8_t EREVID		= (0x12|BANK3);
    static constexpr uint8_t ECOCON		= (0x15|BANK3);
    static constexpr uint8_t EFLOCON	= (0x17|BANK3);
    static constexpr uint8_t EPAUSL		= (0x18|BANK3);
    static constexpr uint8_t EPAUSH		= (0x19|BANK3);

    // PHY registers
    static constexpr uint8_t PHCON1   = 0x00;
    static constexpr uint8_t PHSTAT1  = 0x01;
    static constexpr uint8_t PHHID1   = 0x02;
    static constexpr uint8_t PHHID2   = 0x03;
    static constexpr uint8_t PHCON2	  = 0x10;
    static constexpr uint8_t PHSTAT2  = 0x11;
    static constexpr uint8_t PHIE     = 0x12;
    static constexpr uint8_t PHIR     = 0x13;
    static constexpr uint8_t PHLCON   = 0x14;

    // ENC28J60 EIE Register Bit Definitions
    static constexpr uint8_t EIE_INTIE  = 0x80;
    static constexpr uint8_t EIE_PKTIE  = 0x40;
    static constexpr uint8_t EIE_DMAIE  = 0x20;
    static constexpr uint8_t EIE_LINKIE = 0x10;
    static constexpr uint8_t EIE_TXIE   = 0x08;
    static constexpr uint8_t EIE_WOLIE  = 0x04;
    static constexpr uint8_t EIE_TXERIE = 0x02;
    static constexpr uint8_t EIE_RXERIE = 0x01;

    // ENC28J60 EIR Register Bit Definitions
    static constexpr uint8_t  EIR_PKTIF		= 0x40;
    static constexpr uint8_t  EIR_DMAIF		= 0x20;
    static constexpr uint8_t  EIR_LINKIF	= 0x10;
    static constexpr uint8_t  EIR_TXIF		= 0x08;
    static constexpr uint8_t  EIR_WOLIF		= 0x04;
    static constexpr uint8_t  EIR_TXERIF	= 0x02;
    static constexpr uint8_t  EIR_RXERIF	= 0x01;

    // ENC28J60 ESTAT Register Bit Definitions
    static constexpr uint8_t  ESTAT_INT		= 0x80;
    static constexpr uint8_t  ESTAT_LATECOL	= 0x10;
    static constexpr uint8_t  ESTAT_RXBUSY	= 0x04;
    static constexpr uint8_t  ESTAT_TXABRT	= 0x02;
    static constexpr uint8_t  ESTAT_CLKRDY	= 0x01;

    // ENC28J60 ECON2 Register Bit Definitions
    static constexpr uint8_t  ECON2_AUTOINC = 0x80;
    static constexpr uint8_t  ECON2_PKTDEC  = 0x40;
    static constexpr uint8_t  ECON2_PWRSV   = 0x20;
    static constexpr uint8_t  ECON2_VRPS    = 0x08;

    // ENC28J60 ECON1 Register Bit Definitions
    static constexpr uint8_t  ECON1_TXRST		= 0x80;
    static constexpr uint8_t  ECON1_RXRST		= 0x40;
    static constexpr uint8_t  ECON1_DMAST		= 0x20;
    static constexpr uint8_t  ECON1_CSUMEN      = 0x10;
    static constexpr uint8_t  ECON1_TXRTS		= 0x08;
    static constexpr uint8_t  ECON1_RXEN		= 0x04;
    static constexpr uint8_t  ECON1_BSEL1		= 0x02;
    static constexpr uint8_t  ECON1_BSEL0		= 0x01;

    // ENC28J60 MACON1 Register Bit Definitions
    static constexpr uint8_t  MACON1_LOOPBK	    = 0x10;
    static constexpr uint8_t  MACON1_TXPAUS	    = 0x08;
    static constexpr uint8_t  MACON1_RXPAUS	    = 0x04;
    static constexpr uint8_t  MACON1_PASSALL	= 0x02;
    static constexpr uint8_t  MACON1_MARXEN	    = 0x01;

    // ENC28J60 MACON2 Register Bit Definitions
    static constexpr uint8_t  MACON2_MARST      = 0x80;
    static constexpr uint8_t  MACON2_RNDRST     = 0x40;
    static constexpr uint8_t  MACON2_MARXRST	= 0x08;
    static constexpr uint8_t  MACON2_RFUNRST	= 0x04;
    static constexpr uint8_t  MACON2_MATXRST	= 0x02;
    static constexpr uint8_t  MACON2_TFUNRST	= 0x01;

    // ENC28J60 MACON3 Register Bit Definitions
    static constexpr uint8_t  MACON3_PADCFG2	= 0x80;
    static constexpr uint8_t  MACON3_PADCFG1	= 0x40;
    static constexpr uint8_t  MACON3_PADCFG0	= 0x20;
    static constexpr uint8_t  MACON3_TXCRCEN	= 0x10;
    static constexpr uint8_t  MACON3_PHDRLEN	= 0x08;
    static constexpr uint8_t  MACON3_HFRMLEN	= 0x04;
    static constexpr uint8_t  MACON3_FRMLNEN	= 0x02;
    static constexpr uint8_t  MACON3_FULDPX 	= 0x01;

    // ENC28J60 MICMD Register Bit Definitions
    static constexpr uint8_t  MICMD_MIISCAN	    = 0x02;
    static constexpr uint8_t  MICMD_MIIRD		= 0x01;

    // ENC28J60 MISTAT Register Bit Definitions
    static constexpr uint8_t MISTAT_NVALID      = 0x04;
    static constexpr uint8_t MISTAT_SCAN        = 0x02;
    static constexpr uint8_t MISTAT_BUSY        = 0x01;

    // ENC28J60 PHY PHCON1 Register Bit Definitions
    static constexpr uint16_t PHCON1_PRST    = 0x8000;
    static constexpr uint16_t PHCON1_PLOOPBK = 0x4000;
    static constexpr uint16_t PHCON1_PPWRSV  = 0x0800;
    static constexpr uint16_t PHCON1_PDPXMD  = 0x0100;

    // ENC28J60 PHY PHSTAT1 Register Bit Definitions
    static constexpr uint16_t PHSTAT1_PFDPX  = 0x1000;
    static constexpr uint16_t PHSTAT1_PHDPX  = 0x0800;
    static constexpr uint16_t PHSTAT1_LLSTAT = 0x0004;
    static constexpr uint16_t PHSTAT1_JBSTAT = 0x0002;

    // ENC28J60 PHY PHCON2 Register Bit Definitions
    static constexpr uint16_t PHCON2_FRCLINK = 0x4000;
    static constexpr uint16_t PHCON2_TXDIS   = 0x2000;
    static constexpr uint16_t PHCON2_JABBER  = 0x0400;
    static constexpr uint16_t PHCON2_HDLDIS  = 0x0100;

    // ENC28J60 Packet Control Byte Bit Definitions
    static constexpr uint8_t PKTCTRL_PHUGEEN   = 0x08;
    static constexpr uint8_t PKTCTRL_PPADEN    = 0x04;
    static constexpr uint8_t PKTCTRL_PCRCEN    = 0x02;
    static constexpr uint8_t PKTCTRL_POVERRIDE = 0x01;

protected:
	static void
	writeOp(uint8_t op, uint8_t addr, uint8_t data)
	{
		CS::reset();
		SPI::transferBlocking(op | (addr & ADDR_MASK));
		SPI::transferBlocking(data);
		CS::set();
        xpcc::delayMicroseconds(10);
	}

	static uint8_t
	readOp(uint8_t op, uint8_t address)
	{
		CS::reset();
		SPI::transferBlocking(op | (address & ADDR_MASK));
		uint8_t ret = SPI::transferBlocking(0x00);

		// do dummy read if needed
		if (address & 0x80) {
			ret = SPI::transferBlocking(0x00);
		}

		CS::set();
        xpcc::delayMicroseconds(10);

		return ret;
	}

	static void
    readBuffer(uint16_t len, uint8_t* data)
	{
		// assert CS
		CS::reset();

		// issue read command
		SPI::transferBlocking(SpiOperationCodes::READ_BUF_MEM);

        // read data
        SPI::transferBlocking(0, data, len);

		// release CS
		CS::set();
        xpcc::delayMicroseconds(10);
	}

    static void
    writeBuffer(uint16_t len, uint8_t* data)
    {
		// assert CS
		CS::reset();

        // issue write command
        SPI::transferBlocking(SpiOperationCodes::WRITE_BUF_MEM);

        // write data
        SPI::transferBlocking(data, 0, len);

        // release CS
		CS::set();
        xpcc::delayMicroseconds(10);
    }

	static void
	setBank(uint8_t address)
	{
		// set the bank (if needed)
		if ((address & BANK_MASK) != bank)
		{
            // XPCC_LOG_DEBUG.printf("Change bank from %02x to %02x\n", bank, (address & BANK_MASK));
			// set the bank
			writeOp(SpiOperationCodes::BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
			writeOp(SpiOperationCodes::BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
			bank = (address & BANK_MASK);
		}

       // XPCC_LOG_DEBUG.printf("Bank = 0x%02x\n", bank);
	}

	static uint8_t
	read(uint8_t address)
	{
		setBank(address);
		return readOp(SpiOperationCodes::READ_CTRL_REG, address);
	}

	static void
	write(uint8_t address, uint8_t data)
	{
		setBank(address);
		writeOp(SpiOperationCodes::WRITE_CTRL_REG, address, data);
	}

    static uint16_t
    phyRead(uint8_t address)
    {
        // set the PHY register address
		write(MIREGADR, address);

        write(MICMD, MICMD_MIIRD);

 		// wait until the PHY read completes
		while(read(MISTAT) & MISTAT_BUSY)
            ;

        // Clear MICMD_MIIRD
        write(MICMD, 0);

        uint16_t ret = 0;
        ret = read(MIRDH) << 8;
        ret |= read(MIRDL);

        return ret;
    }

public:
	static void
	phyWrite(uint8_t address, uint16_t data)
	{
		// set the PHY register address
		write(MIREGADR, address);

		// write the PHY data
		write(MIWRL, data & 0xff);
		write(MIWRH, data >> 8);

		// wait until the PHY write completes
		while(read(MISTAT) & MISTAT_BUSY);
	}

	static void
	nicSetMacAddress(uint8_t* macaddr)
	{
		// write MAC address
		// NOTE: MAC address in ENC28J60 is byte-backward
		write(MAADR5, *macaddr++);
		write(MAADR4, *macaddr++);
		write(MAADR3, *macaddr++);
		write(MAADR2, *macaddr++);
		write(MAADR1, *macaddr++);
		write(MAADR0, *macaddr++);
	}

protected:
	// Keep track of current bank in ENC28J60
	static uint8_t
	bank;

	// Remember where the next frame will be stored
	static uint16_t
	nextPacketPtr;
};

} // namespace xpcc

template < typename SPI, typename CS >
uint8_t xpcc::Enc28j60<SPI, CS>::bank = 0;

template < typename SPI, typename CS >
uint16_t xpcc::Enc28j60<SPI, CS>::nextPacketPtr = 0;

#include "enc28j60_impl.hpp"

#endif // XPCC_ENC28J60_HPP
