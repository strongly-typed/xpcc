#include <xpcc/architecture/platform.hpp>
#include <xpcc/processing/timer.hpp>
#include <xpcc/debug/logger.hpp>

#include "enc28j60.hpp"

#include "../../stm32f4_discovery.hpp"

#include <unistd.h>

namespace enc28j60
{
	typedef xpcc::stm32::GpioOutputB11 nReset;
	typedef xpcc::stm32::GpioOutputB12 Cs;
	typedef xpcc::stm32::SpiMaster2 Spi;
}

xpcc::IODeviceWrapper< xpcc::stm32::Usart2, xpcc::IOBuffer::BlockIfFull > loggerDevice;
xpcc::log::Logger xpcc::log::debug(loggerDevice);

// Set the log level
#undef  XPCC_LOG_LEVEL
#define XPCC_LOG_LEVEL xpcc::log::DEBUG


// ----------------------------------------------------------------------------
/**
 * Simple demo:
 * (1) Dumps all received Ethernet frames on serial console.
 *     For each received frame the red LED toggles.
 * (2) Sends a (dummy) ICMP echo ping request to 255.255.255.255 once a second
 *     For each transmittied frame green/blue LEDs toggle.
 *
 * The ping will not actually work because the IP address is not announced by ARP.
 */
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

	typedef xpcc::Enc28j60<enc28j60::Spi, enc28j60::Cs> enc28;

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
        while (not timer.execute())
        {
            uint16_t packet_length = enc28::receivePacket(BUFFER_SIZE, buffer);
            if (packet_length) {
                XPCC_LOG_DEBUG << "Receive pLength = " << packet_length << xpcc::endl;
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

        XPCC_LOG_DEBUG.printf("Sending\n");
        enc28::sendPacket(42, buff);

        Board::LedBlue::toggle();
        Board::LedGreen::toggle();
       }
	}

	return 0;
}
