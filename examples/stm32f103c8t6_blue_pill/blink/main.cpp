#include <xpcc/architecture/platform.hpp>
#include <xpcc/debug/logger.hpp>

#include <xpcc/processing/timer.hpp>
#include <xpcc/processing/protothread.hpp>

#include <xpcc/driver/can/mcp2515.hpp>

using namespace Board;

// Set the log level
#undef	XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

// Create an IODeviceWrapper around the Uart Peripheral we want to use
xpcc::IODeviceWrapper< Usart1, xpcc::IOBuffer::BlockIfFull > loggerDevice;

// Set all four logger streams to use the UART
xpcc::log::Logger xpcc::log::debug(loggerDevice);
xpcc::log::Logger xpcc::log::info(loggerDevice);
xpcc::log::Logger xpcc::log::warning(loggerDevice);
xpcc::log::Logger xpcc::log::error(loggerDevice);

using LedOrange = GpioOutputC7; // orange
using LedYellow = GpioOutputC6; // yellow
using LedRed = xpcc::GpioInverted<GpioOutputB15>;
using LedGreen = xpcc::GpioInverted<GpioOutputB14>;

using Button1 = GpioInputC8;
using Button2 = GpioInputC9;

xpcc_extern_c void
xpcc_abandon(const char * module,
			 const char * location,
			 const char * failure,
			 uintptr_t context)
{
	XPCC_LOG_ERROR << "Assertion '" << module << "." << location << "." << failure << "'";
	if (context) { XPCC_LOG_ERROR << " @ " << (void *) context << " (" << (uint32_t) context << ")"; }
	XPCC_LOG_ERROR << " failed! Abandoning..." << xpcc::endl;

	LedRed::setOutput();
	while(true) {
		LedRed::set();
		xpcc::delayMilliseconds(20);
		LedRed::reset();
		xpcc::delayMilliseconds(180);
	}
}

/*
 * Blinks the green user LED with 1 Hz.
 * It is on for 90% of the time and off for 10% of the time.
 */


/*
 *********** TEST PROTOCOL *************

 [OK] SystemClock at 8 MHz
 [OK] Four LEDs
 [OK] Two Buttons
 [OK] Power supply
 [OK] External Clock with oscilloscope
 [OK] Use External Clock
 [OK] UART
 [OK] GPIOs on PinHeader (some pins unusable because of debugging config)
 [OK] Observe MCO on Oscilloscope
      [OK] 25 MHz from HSE
      [OK] 32 MHz from PLL1 (:5 *4)
 [  ] Create Device file with alternate functions on pin header
 [OK] CAN ** FAIL ** PB8 has short circuit to RESET ** FAIL **
 [OK] PLL
 [  ] Fix Ethernet LEDs?
 [  ] Ethernet

 ToDo
 ====
 [  ] Fix RESET (Done with Wire)
 [  ] Use available uC pins
 [  ] More silkscreen
 [  ] Change LDO
 [  ] Sort schematic

 */

FLASH_STORAGE(uint8_t can_filter[]) =
{
	MCP2515_FILTER_EXTENDED(0),	// Filter 0
	MCP2515_FILTER_EXTENDED(0),	// Filter 1

	MCP2515_FILTER_EXTENDED(0),	// Filter 2
	MCP2515_FILTER_EXTENDED(0),	// Filter 3
	MCP2515_FILTER_EXTENDED(0),	// Filter 4
	MCP2515_FILTER_EXTENDED(0),	// Filter 5

	MCP2515_MASK_EXTENDED(0),	// Mask 0
	MCP2515_MASK_EXTENDED(0),	// Mask 1
};


static void
displayMessage(const xpcc::can::Message& message)
{
	static uint32_t receiveCounter = 0;
	receiveCounter++;

	XPCC_LOG_INFO<< "id  =" << message.getIdentifier();
	if (message.isExtended()) {
		XPCC_LOG_INFO<< " extended";
	}
	else {
		XPCC_LOG_INFO<< " standard";
	}
	if (message.isRemoteTransmitRequest()) {
		XPCC_LOG_INFO<< ", rtr";
	}
	XPCC_LOG_INFO<< xpcc::endl;

	XPCC_LOG_INFO<< "dlc =" << message.getLength() << xpcc::endl;
	if (!message.isRemoteTransmitRequest())
	{
		XPCC_LOG_INFO << "data=";
		for (uint32_t i = 0; i < message.getLength(); ++i) {
			XPCC_LOG_INFO<< xpcc::hex << message.data[i] << xpcc::ascii << ' ';
		}
		XPCC_LOG_INFO<< xpcc::endl;
	}
	XPCC_LOG_INFO<< "# received=" << receiveCounter << xpcc::endl << xpcc::endl;
}

template < class Mdc, class Mdio >
class Smi
{
public:
	static uint16_t
	read(uint8_t phy_address, uint8_t reg_address)
	{
		Mdio::setOutput();
		Mdio::set();
		// Preamble
		for (uint8_t ii = 0; ii < 64; ++ii)
		{
			Mdc::reset();
			xpcc::delayMicroseconds(10);
			// Mdio sampled on rising edge
			Mdc::set();
			xpcc::delayMicroseconds(10);
		}

		// SOF, Op, Phy Address, Reg Address
		uint16_t buf = 0x6000 | ( (phy_address & 0x1f) << 7) | ( (reg_address & 0x1f) << 2);
		// XPCC_LOG_DEBUG.printf("Read with %04x\n", buf);
		for (uint8_t ii = 0; ii < 16; ++ii)
		{
			Mdc::reset();
			Mdio::set(buf & 0x8000);
			// XPCC_LOG_DEBUG.printf(buf & 0x8000 ? "1" : "0");
			buf = buf << 1;
			xpcc::delayMicroseconds(10);

			Mdc::set();
			xpcc::delayMicroseconds(10);			
		}
		Mdio::setInput();
		// XPCC_LOG_DEBUG << xpcc::endl;

		uint16_t ret = 0;
		for (uint8_t ii = 0; ii < 16; ++ii)
		{
			// XPCC_LOG_DEBUG.printf(Mdio::read() ? "1" : "0");
			ret <<= 1;			
			ret = ret | Mdio::read();

			Mdc::reset();
			xpcc::delayMicroseconds(10);

			Mdc::set();
			xpcc::delayMicroseconds(10);
		}
		// XPCC_LOG_DEBUG << xpcc::endl;

		return ret;
	}
};

int
main()
{
	Board::initialize();

	LedOrange::setOutput();
	LedYellow::setOutput();
	LedRed::setOutput();
	LedGreen::setOutput();

	LedRed::reset();
	LedGreen::reset();

	Button1::setInput(Gpio::InputType::PullUp);
	Button2::setInput(Gpio::InputType::PullUp);

	// initialize Uart2 for XPCC_LOG_*
	GpioOutputA9::connect(Usart1::Tx);
	GpioInputA10::connect(Usart1::Rx, Gpio::InputType::PullUp);
	Usart1::initialize<Board::systemClock, 115200>(12);

	// Init CAN
	GpioOutputB9::connect(Can1::Tx, Gpio::OutputType::PushPull);
	GpioInputB8::connect(Can1::Rx, Gpio::InputType::PullUp);

	static constexpr xpcc::Can::Bitrate can_bitrate = xpcc::Can::Bitrate::kBps125; // xpcc::Can::Bitrate::MBps1;
	Can1::initialize<Board::systemClock, can_bitrate>(12);

	// Receive every CAN message
	CanFilter::setFilter(0, CanFilter::FIFO0,
                        CanFilter::ExtendedIdentifier(0),
                        CanFilter::ExtendedFilterMask(0));


	// 25 MHz HSE at MCO
	if (false) {
		ClockControl::enableClockOutput(ClockControl::ClockOutputSource::SystemClock);
		GpioOutputA8::connect(ClockControl::ClockOutput);
	}

	if (false) {
		// 25 MHz / 5 * 4 = 20 MHz PLL
		bool ret = ClockControl::enablePll(
			ClockControl::PllSource::ExternalClock,
			/* pllMul    = */ 4,
			/* pllPrediv = */ 5);

		ret ? LedGreen::set() : LedRed::set();

		// Observe 20 MHz / 2 = 10 MHz at MCO
		ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll);
		GpioOutputA8::connect(ClockControl::ClockOutput);
	}


	if (false) {
		// 25 MHz / 5 * 9 = 45 MHz PLL
		// PLL Input:   3.0 < 25/5 = 5 <  12.00 MHz
		// PLL Output: 18.0 < 45       <  72.00 MHz
		// PLL VCO:    36.0 < 45*2=90  < 144.00 MHz

		bool ret = ClockControl::enablePll(
			ClockControl::PllSource::ExternalClock,
			/* pllMul    = */ 9,
			/* pllPrediv = */ 5);

		ret ? LedGreen::set() : LedRed::set();

		// Observe 45 MHz / 2 = 22.5 MHz at MCO
		ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll);
		GpioOutputA8::connect(ClockControl::ClockOutput);
	}


	if (false)
	{
		// PLL2 at 40 MHz to MCO
		//  3.0 MHz < PLL2 in  <  5.0 MHz
		// 40.0 MHz < Pll2 out < 74.0 MHz
		ClockControl::setPll2Pll3Prescaler(5); // 25 / 5 = 5 OK
		bool ret = ClockControl::enablePll2(8); // 5 * 8 = 40 OK

		ret ? LedGreen::set() : LedRed::set();

		ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll2);
		GpioOutputA8::connect(ClockControl::ClockOutput);
	}


	if (true)
	{
		// PLL2 at 40 MHz to Pll
		//  3.0 MHz < PLL2 in  <  5.0 MHz
		// 40.0 MHz < Pll2 out < 74.0 MHz
		ClockControl::setPll2Pll3Prescaler(5); // 25 / 5 = 5 OK
		bool ret = ClockControl::enablePll2(8); // 5 * 8 = 40 OK

		// PLL3 at 50 MHz to MCO
		//  3.0 MHz < PLL3 in  <  5.0 MHz
		// 40.0 MHz < Pll3 out < 74.0 MHz

		// Shared prescaler
		// ClockControl::setPll2Pll3Prescaler(5); // 25 / 5 = 5 OK
		ret &= ClockControl::enablePll3(10); // 5 * 10 = 50 OK

		// PLL at 72 MHz
		ret &= ClockControl::setPrediv1Source(ClockControl::Prediv1Source::Pll2);

		ret &= ClockControl::enablePll(
			ClockControl::PllSource::Hse,
			/* pllMul    = */ 9,
			/* pllPrediv = */ 5);

		// 72 / 2 = 36 MHz from PLL
		// ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll);

		// 50 MHz from PLL3 to Ethernet
		ret &= ClockControl::enableClockOutput(ClockControl::ClockOutputSource::Pll3);
		GpioOutputA8::connect(ClockControl::ClockOutput);

		// Run core on PLL with 72 MHz
		ret &= ClockControl::enableSystemClock(ClockControl::SystemClockSource::Pll);

		ret ? LedGreen::set() : LedRed::set();
	}

	// Unusable (debug pins)
	// GpioOutputB4::disconnect();
	// GpioOutputB3::disconnect();

	// xpcc::can::Message msg1(1, 1);
 //    msg1.setExtended(true);
 //    msg1.data[0] = 0x11;
 //    Can1::sendMessage(msg1);

	// GpioOutputB9::setOutput();

	while(false)
	{
		LedGreen::toggle();
		Usart1::write('s');
		// GpioOutputB9::toggle();
		xpcc::delayMilliseconds(500);
	}

	// Unusable (debug pins)
	// GpioOutputB4::disconnect();
	// GpioOutputB3::disconnect();

	XPCC_LOG_DEBUG << "Hello at MCP2515 demo" << xpcc::endl;

	// xpcc::can::Message msg1(1, 1);
 //    msg1.setExtended(true);
 //    msg1.data[0] = 0x11;
 //    Can1::sendMessage(msg1);

	// GpioOutputB9::setOutput();

	while(false)
	{
		GpioOutputC10::connect(SpiMaster3::Sck);
		GpioOutputC12::connect(SpiMaster3::Mosi);
		GpioInputC11::connect(SpiMaster3::Miso);
		using Cs = GpioOutputD2;
		Cs::setOutput();

		SpiMaster3::initialize<Board::systemClock, 9000000ul>();

		while (false)
		{
			uint8_t buf[16];
			for (uint8_t ii = 0; ii < 16; ++ii)
			{
				buf[ii] = ii ^ (ii + 5);
			}
			while(true){
				SpiMaster3::transferBlocking(buf, nullptr, 16);
				xpcc::delayMilliseconds(100);
				// if (SpiHal3::isTransmitRegisterEmpty())
				// {
				// 	SpiHal3::write(*(buf));
				// }
			}
		}

		xpcc::Mcp2515< SpiMaster3, Cs, xpcc::GpioUnused> mcp;

		if (not mcp.initialize<xpcc::clock::MHz8, can_bitrate>())
		{
			LedRed::toggle();
			xpcc::delayMilliseconds(100);
		}

		mcp.setFilter(xpcc::accessor::asFlash(can_filter));

		mcp.setMode(xpcc::Can::Mode::Normal);

		XPCC_LOG_DEBUG << "RX loop starting ..." << xpcc::endl;

		while(false)
		{
			xpcc::can::Message message;
			if (mcp.getMessage(message))
			{
				LedGreen::toggle();
				displayMessage(message);
			}
		}

		xpcc::PeriodicTimer pt(500);
		uint8_t idx = 0;

		while(true)
		{
			if (pt.execute())
			{
				LedGreen::reset();
				LedRed::reset();
			
				xpcc::can::Message message(0x1055aa99);

				// XPCC's default is extended CAN messages
				// message.setExtended(false);
				message.length = 8;
				message.data[0] = idx++;
				message.data[1] = idx ^ (idx + 5);
				// Just try to send. Driver returns false when not ready to send.
				if (mcp.sendMessage(message))
				{
					LedGreen::set();
				} else {
					LedRed::set();
				}

				{
					xpcc::can::Message message(0x04);
					message.length = 2;
					message.data[0] = 0xaa;
					message.data[1] = 0xaa;
					// Can1::sendMessage(message);
				}

				// LedGreen::reset();
			}
		}
	}

	while (false)
	{
		Usart1::write('s');
		xpcc::delayMilliseconds(100);

		uint8_t cc;
		if (Usart1::read(cc)) { 
			Usart1::write(cc + 1);
		}
	}

	while (false)
	{
		XPCC_LOG_DEBUG << "STM32F107 LAN8720 Ethernet Demo" << xpcc::endl;
		xpcc::delayMilliseconds(500);
		// LAN8720
		// MDIO = PA2
		// MDC  = PC1
		using Mdio = GpioA2;
		using Mdc  = GpioOutputC1;

		Mdio::setOutput();
		Mdc::setOutput();

		using smi = Smi<Mdc, Mdio>;

		// while (true)
		{

			// for (uint8_t phy = 0; phy <= 0x1f; ++phy)
			for (uint8_t reg = 0; reg <= 0x1f; ++reg)
			{
				uint16_t ret = smi::read(/* phy */ 0, /* reg */ reg);
				XPCC_LOG_DEBUG.printf("%02x = %04x\n", reg, ret);
				// xpcc::delayMilliseconds(250);
			}
		}

		while(true)
			{};
	}

	while (true)
	{
		Eth::initialize();

		// Alternate functions
		GpioOutputC1::connect(Eth::Mdc);
		GpioOutputA2::connect(Eth::Mdio);

		GpioOutputB11::connect(Eth::TxEn);
		GpioOutputB12::connect(Eth::TxD0);
		GpioOutputB13::connect(Eth::TxD1);

		for (uint8_t reg_address = 0; reg_address < 32; ++reg_address)
		{
			XPCC_LOG_DEBUG.printf("%02x -> %04x\n", reg_address, Eth::readPhy(reg_address));
		}

		while (true)
			{};

	}




	return 0;
}
