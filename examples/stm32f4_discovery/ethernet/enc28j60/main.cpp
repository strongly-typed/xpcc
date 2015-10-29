#include <xpcc/architecture/platform.hpp>
#include <xpcc/processing/timer.hpp>
#include <xpcc/debug/logger.hpp>
#include "../../stm32f4_discovery.hpp"

#include <unistd.h>

static constexpr uint8_t ENC28J60_MAC0 = 0xab;
static constexpr uint8_t ENC28J60_MAC1 = 0xbc;
static constexpr uint8_t ENC28J60_MAC2 = 0x6f;
static constexpr uint8_t ENC28J60_MAC3 = 0x55;
static constexpr uint8_t ENC28J60_MAC4 = 0x1c;
static constexpr uint8_t ENC28J60_MAC5 = 0xc3;

namespace enc28j60
{
	typedef xpcc::stm32::GpioOutputB11 nReset;
	typedef xpcc::stm32::GpioOutputB12 Cs;
	typedef xpcc::stm32::SpiMaster2 Spi;
}

template < typename SPI, typename CS >
class Enc28j60
{
public:
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

    // buffer boundaries applied to internal 8K ram
    // entire available packet buffer space is allocated
    static constexpr uint16_t TXSTART_INIT = 0x0000;	// start TX buffer at 0
    static constexpr uint16_t RXSTART_INIT = 0x0600;	// give TX buffer space for one full ethernet frame (~1500 bytes)
    static constexpr uint16_t RXSTOP_INIT  = 0x1FFF;	// receive buffer gets the rest

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


	static void
	initialize()
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
        XPCC_LOG_DEBUG.printf("ERXST = %02x %02x == [06 00]\n", read(ERXSTH), read(ERXSTL));

		// set receive pointer address
		write(ERXRDPTL, RXSTART_INIT & 0xFF);
		write(ERXRDPTH, RXSTART_INIT >> 8);
        XPCC_LOG_DEBUG.printf("ERXRDPT = %02x %02x == [06 00]\n", read(ERXRDPTH), read(ERXRDPTL));

		// set receive buffer end
		// ERXND defaults to 0x1FFF (end of ram)
		write(ERXNDL, RXSTOP_INIT & 0xFF);
		write(ERXNDH, RXSTOP_INIT >> 8);
        XPCC_LOG_DEBUG.printf("ERXND = %02x %02x = [1F FF]\n", read(ERXNDH), read(ERXNDL));

		// set transmit buffer start
		// ETXST defaults to 0x0000 (beginning of ram)
		write(ETXSTL, TXSTART_INIT & 0xFF);
		write(ETXSTH, TXSTART_INIT >> 8);
        XPCC_LOG_DEBUG.printf("ETXST = %02x %02x = [00 00]\n", read(ETXSTH), read(ETXSTL));

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

	static uint16_t
	receivePacket(uint16_t maxlen, uint8_t* packet)
	{
		uint16_t rxstat;

		// check if a packet has been received and buffered
		if( not(read(EIR) & EIR_PKTIF) )
			return 0;

		// Make absolutely certain that any previous packet was discarded
		//if( WasDiscarded == FALSE)
		//	MACDiscardRx();

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

    static void
    sendPacket(uint16_t len, uint8_t* packet)
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

protected:
	static uint8_t
	bank;

	static uint16_t
	nextPacketPtr;
};

template < typename SPI, typename CS >
uint8_t Enc28j60<SPI, CS>::bank = 0;

template < typename SPI, typename CS >
uint16_t Enc28j60<SPI, CS>::nextPacketPtr = 0;


xpcc::IODeviceWrapper< xpcc::stm32::Usart2, xpcc::IOBuffer::BlockIfFull > loggerDevice;
xpcc::log::Logger xpcc::log::debug(loggerDevice);

// Set the log level
#undef  XPCC_LOG_LEVEL
#define XPCC_LOG_LEVEL xpcc::log::DEBUG


// ----------------------------------------------------------------------------
MAIN_FUNCTION
{
    Board::systemClock::enable();
    xpcc::cortex::SysTickTimer::initialize<Board::systemClock>();

    // UART
    xpcc::stm32::GpioA2::connect(xpcc::stm32::Usart2::Tx);
    xpcc::stm32::GpioA3::connect(xpcc::stm32::Usart2::Rx, Gpio::InputType::PullUp);
    xpcc::stm32::Usart2::initialize<Board::systemClock, 115200>(12);

    XPCC_LOG_DEBUG << "Hello ENC28" << xpcc::endl;

    // Setup GPIO
	Board::LedOrange::setOutput(xpcc::Gpio::Low);
	Board::LedGreen::setOutput(xpcc::Gpio::Low);
	Board::LedRed::setOutput(xpcc::Gpio::High);
	Board::LedBlue::setOutput(xpcc::Gpio::High);

	enc28j60::nReset::setOutput(xpcc::Gpio::High);
	enc28j60::Cs::setOutput(xpcc::Gpio::High);

	// Enable SPI 2
    xpcc::stm32::GpioB13::setOutput();
    xpcc::stm32::GpioB15::setOutput();
    xpcc::stm32::GpioB14::setInput(Gpio::InputType::PullUp);

	xpcc::stm32::GpioB13::connect(enc28j60::Spi::Sck);
	xpcc::stm32::GpioB15::connect(enc28j60::Spi::Mosi);
	xpcc::stm32::GpioB14::connect(enc28j60::Spi::Miso);
	enc28j60::Spi::initialize<Board::systemClock, 10500000ul, xpcc::Tolerance::FivePercent>();

    // Hard Reset the device
    enc28j60::nReset::reset();
    xpcc::delayMilliseconds(100);
    enc28j60::nReset::set();
    xpcc::delayMilliseconds(100);

	uint8_t mymac[6] = {0xab,0xbc,0x6f,0x55,0x1c,0xc2};

	typedef Enc28j60<enc28j60::Spi, enc28j60::Cs> enc28;

	enc28::initialize();

	// Mac Adresse setzen(stack.h, dort wird auch die Ip festgelegt)
	enc28::nicSetMacAddress(mymac);

    //                      0000AAAABBBBSSS0
    enc28::phyWrite(0x14, 0b0000001011010010);

	/* Buffer Size */
	static constexpr uint16_t BUFFER_SIZE = 1536;
	uint8_t buffer[BUFFER_SIZE];

    xpcc::PeriodicTimer timer(1000);

    uint16_t seq = 42;

    // Wait some time until link is up
    xpcc::delayMilliseconds(5000);

	while (true)
	{

        // XPCC_LOG_DEBUG.printf("Phy: 0x%04x\n", enc28::phyRead(0x01));
        uint16_t packet_length = enc28::receivePacket(BUFFER_SIZE, buffer);

		/* When a packet was received, packet_length > 0 */
		if (packet_length) {
            XPCC_LOG_DEBUG << "pLength = " << packet_length << xpcc::endl;
            Board::LedRed::toggle();
            uint16_t addr = 0;
            XPCC_LOG_DEBUG.printf("%04x ", addr);
            for (std::size_t ii = 0; ii < packet_length; ++ii) {
                XPCC_LOG_DEBUG.printf("%02x ", buffer[ii]);
                if (ii % 16 == 15) {
                    XPCC_LOG_DEBUG << xpcc::endl;
                    XPCC_LOG_DEBUG.printf("%04x ", addr);
                    addr += 16;
                }
                if (ii % 16 == 7) {
                    XPCC_LOG_DEBUG << " ";
                }
            }
            XPCC_LOG_DEBUG << xpcc::endl;
		}

        // Always send a ping
        uint8_t buff[42] = {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Dest   = Broadcast
            0x68, 0x5b, 0x35, 0x91, 0xb5, 0xdd, // Source
            0x08, 0x00,                         // Type   = IP
            0x45,                               // IPv4
            0x00,                               // DSCP
            0x00, 0x1c,                         // Total length = 20
            0x41, 0xa7,                         // Identification
            0x00,                               // Flags
            0x00,                               // Offset
            0x40,                               // TTL
            0x01,                               // Protocol = ICMP
            0x39, 0x3b,                         // Header Checksum
            0x00, 0x00, 0x00, 0x00,             // Source IP
            0xff, 0xff, 0xff, 0xff,             // Destination IP
                  0x08,                         // ICMP Echo
                  0x00,                         // Code
                  0xb6, 0x58,                   // Checksum
                  0x41, 0xa7,                   // Identifier
                  0x00, 0x00                    // Sequence number
        };
        buff[40] = seq >> 8;
        buff[41] = seq & 0xff;
        ++seq;

        while (not timer.execute())
            ;
        {
            enc28::sendPacket(42, buff);
            Board::LedBlue::toggle();
            Board::LedGreen::toggle();
       }
	}

	return 0;
}
