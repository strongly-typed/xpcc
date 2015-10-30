#ifndef ENC28J60_AT_SPI2
#define ENC28J60_AT_SPI2

#include <xpcc/architecture.hpp>

#include "../../stm32f4_discovery.hpp"
#include "../../ethernet/enc28j60/enc28j60.hpp"

// Initialize Microchip ENC28J60 at SPI2 connected to STM32F4Discovery board.
// Avoids duplicated code in sender and receiver.

// ----------------------------------------------------------------------------
namespace enc28j60
{
	typedef xpcc::stm32::GpioOutputB11 nReset;
	typedef xpcc::stm32::GpioOutputB12 Cs;

	typedef xpcc::stm32::SpiMaster2 Spi;
	typedef xpcc::Enc28j60< Spi, Cs > enc28;
	typedef xpcc::Enc28j60Can < enc28 > EthernetDevice;
}

void
initEnc28J60()
{
	enc28j60::nReset::setOutput(xpcc::Gpio::High);
	enc28j60::Cs::setOutput(xpcc::Gpio::High);

	// Enable SPI 2
    xpcc::stm32::GpioB13::setOutput();
    xpcc::stm32::GpioB15::setOutput();
    xpcc::stm32::GpioB14::setInput(xpcc::stm32::Gpio::InputType::PullUp);

	xpcc::stm32::GpioB13::connect(enc28j60::Spi::Sck);
	xpcc::stm32::GpioB15::connect(enc28j60::Spi::Mosi);
	xpcc::stm32::GpioB14::connect(enc28j60::Spi::Miso);
	enc28j60::Spi::initialize<Board::systemClock, 10500000ul, xpcc::Tolerance::FivePercent>();

    // Hard Reset the device
    enc28j60::nReset::reset();
    xpcc::delayMilliseconds(100);

    enc28j60::nReset::set();
    xpcc::delayMilliseconds(100);

	uint8_t mymac[6] = {0xab,0xbc,0x6f,0x55,0x1c,0xc3};

	typedef xpcc::Enc28j60<enc28j60::Spi, enc28j60::Cs >  enc28;

	enc28::initialize();

	// Mac Adresse setzen(stack.h, dort wird auch die Ip festgelegt)
	enc28::nicSetMacAddress(mymac);

    //                      0000AAAABBBBSSS0
    enc28::phyWrite(0x14, 0b0000001011010010);
}

#endif // ENC28J60_AT_SPI2
