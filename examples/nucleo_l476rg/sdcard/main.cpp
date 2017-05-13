#include <xpcc/architecture/platform.hpp>
#include <xpcc/processing.hpp>
#include <xpcc/architecture/interface/gpio.hpp>
#include <xpcc/driver/pressure/bmp085.hpp>
#include <xpcc/driver/display/ssd1306.hpp>

#ifdef XPCC_LOG_LEVEL
#undef XPCC_LOG_LEVEL
#endif

#define XPCC_LOG_LEVEL xpcc::log::DEBUG

// namespace usd
// {
// 	using Cs   = xpcc::stm32::GpioB4;  // LA Ch 0
// 	using Sck  = xpcc::stm32::GpioB5;  // LA Ch 1
// 	using Mosi = xpcc::stm32::GpioB3;  // LA Ch 2
// 	using Miso = xpcc::stm32::GpioA10; // LA Ch 3
// }

namespace usd
{
	using Cs   = xpcc::stm32::GpioC11;  // LA Ch 0
	using Sck  = xpcc::stm32::GpioC12;  // LA Ch 1
	using Mosi = xpcc::stm32::GpioD2;   // LA Ch 2
	using Miso = xpcc::stm32::GpioC8;   // LA Ch 3
}

typedef xpcc::SoftwareSpiMaster< usd::Sck, usd::Mosi, usd::Miso> mySpiMaster;

struct sdcard_spi
{
	enum class
	Command : uint8_t
	{
		GoIdleState      =  0,
		SendOpCond       =  1,
		SendIfCond       =  8,
		SendCid          = 10,
		SetBlockLen      = 16,
		ReadSingleBlock  = 17,
		WriteSingleBlock = 24,
		AppCmd           = 55,
		ReadOcr          = 58,

		// ACMD: Commands that must be prepend with CMD55 = AppCmd
		SdV2Initialize  = 41,

	};

	// Precalculated CRCs for use before switching to SPI mode.
	static constexpr uint8_t GoIdleStateCrc    = 0x95;
	static constexpr uint8_t SendIfCondCrc     = 0x87;
	static constexpr uint8_t AppCmdCrc         = 0x65;
	static constexpr uint8_t SdV2InitializeCrc = 0x5f; // 0xCD; // 0x77;
};

template < typename SpiMaster >
class SDCardSpi : public sdcard_spi
{
public:
	static void
	select()
	{
		xpcc::delayMicroseconds(10);
		usd::Cs::reset();
		xpcc::delayMicroseconds(10);
		uint8_t res = 0;
		do
		{
			res = SpiMaster::transferBlocking(0xff);
		} while (res != 0xff);
	}

	static void
	deselect()
	{
		xpcc::delayMicroseconds(10);
		usd::Cs::set();
		xpcc::delayMicroseconds(10);
		SpiMaster::transferBlocking(0xff);
	}

	static uint8_t
	sendCommand(Command command, uint32_t argument, uint8_t *response = nullptr)
	{
		// ... prepend / stop
		if ((command == Command::SdV2Initialize) /* or
			(command == Command::XXX) */)
		{
			uint8_t res = sendCommand(Command::AppCmd, 0x00);
		}

		deselect();
		select();

		uint8_t sendBuf[6];
		sendBuf[0] = static_cast<uint8_t>(command) | 0x40;
		sendBuf[1] = argument >> 24;
		sendBuf[2] = argument >> 16;
		sendBuf[3] = argument >>  8;
		sendBuf[4] = argument >>  0;

		uint8_t crc = 0x01;

		if (command == Command::GoIdleState) {
			crc = GoIdleStateCrc;
		} else if (command == Command::SendIfCond) {
			crc = SendIfCondCrc;
		} else if (command == Command::AppCmd) {
			crc = AppCmdCrc;
		} else if (command == Command::SdV2Initialize) {
			crc = SdV2InitializeCrc;
		}
		sendBuf[5] = crc;
		
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);

		SpiMaster::transferBlocking(sendBuf, nullptr, 6);
		XPCC_LOG_DEBUG.printf("Cmd = %2d (0x%02x), arg = %02x %02x %02x %02x, crc = %02x", 
			sendBuf[0] & 0x3f, sendBuf[0], 
			sendBuf[1], sendBuf[2], sendBuf[3], sendBuf[4], 
			sendBuf[5]);

		// if stop

		uint8_t res = 0;

		// Command Response time is 0 to 8 bytes for SDC and
		// 1 to 8 bytes for MMC.
		// Source: http://elm-chan.org/docs/mmc/mmc_e.html
		uint8_t timeout = 8;
		do
		{
			res = SpiMaster::transferBlocking(0xff);
		} while ((res & 0x80) and (--timeout > 0));

		if (timeout == 0)
		{
			XPCC_LOG_ERROR.printf("No response received for command %02x.\n", command);
			return 0;
		}
		XPCC_LOG_DEBUG.printf(", res = %02x\n", res);

		if (response != nullptr)
		{
			switch (command)
			{
				// R1 resonse
				case Command::GoIdleState:
				case Command::SendOpCond:
				case Command::ReadSingleBlock:
				case Command::AppCmd:
					break;

				// R3 and R7 response
				case Command::SendIfCond:
				case Command::ReadOcr:
				case Command::SdV2Initialize:
					SpiMaster::transferBlocking(nullptr, response, 4 /* Respone7Length */);
					XPCC_LOG_DEBUG.printf("Res =");
					{ 
						for (uint8_t ii = 0; ii < 4; ++ii) {
							XPCC_LOG_DEBUG.printf(" %02x", response[ii]);
						}
						XPCC_LOG_DEBUG << xpcc::endl;
					}
					break;
			}
		}
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);

		return res;
	}

	static void
	readBlock(uint8_t *buf, uint32_t count = 512)
	{
		uint8_t dataToken = 0xff;
		do
		{
			dataToken = SpiMaster::transferBlocking(0xff);
		} while(dataToken != 0xfe);

		SpiMaster::transferBlocking(nullptr, buf, count);
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);
		SpiMaster::transferBlocking(0xff);
	}

	static uint8_t
	writeBlock(uint8_t *buf, uint8_t token = 0xfe)
	{
		uint8_t res = 0;

		select();

		res = SpiMaster::transferBlocking(token);
		if (token != 0xFD) //Token 0xFD stops transmission
		{
			SpiMaster::transferBlocking(buf, nullptr, 512);

			// CRC
			SpiMaster::transferBlocking(0xFF);
			SpiMaster::transferBlocking(0xFF);

			res = SpiMaster::transferBlocking(0xFF);
			if ((res & 0x1F) != 0x05) {
				return 0;
			}
		}
		return res;

	}
};

// ----------------------------------------------------------------------------
int
main()
{
	Board::initialize();

	usd::Sck::setOutput();
	usd::Mosi::setOutput();
	usd::Cs::setOutput(xpcc::Gpio::High);
	usd::Miso::setInput(Gpio::InputType::PullUp);
	mySpiMaster::initialize<Board::systemClock, 400000ul>();

	using sdcard = SDCardSpi<mySpiMaster>;

	XPCC_LOG_ERROR << "\n\nWelcome to SD Card demo!\n\n";

	while (not Board::Button::read())
		{};

	enum class CardType : uint8_t
	{
		Unknonw = 0x00,
		MMC = 0x01,
		SD1 = 0x02,
		SD2 = 0x04,
		SDC = 0x06,
	};

	{
		// **********
		// Card Init
		// **********

		usd::Cs::set();

		XPCC_LOG_ERROR.printf("\n\n *** INIT ***\n\n");

		// 74 or more clock pulses to SCLK to wake up SD card
		for (uint8_t ii = 0; ii < 50; ++ii) {
			mySpiMaster::transferBlocking(0xff);
		}

		// Select
		usd::Cs::reset();
	
		// Woken up
		uint8_t res = sdcard::sendCommand(sdcard::Command::GoIdleState, 0x00);
		if (res != 0x01)
		{
			// Card not in idle
			XPCC_LOG_ERROR.printf("Card did not wake up in Idle state.\n");
		}

		// Check for SDv2
		uint8_t receiveBuffer[5];
		res = sdcard::sendCommand(sdcard::Command::SendIfCond, 0x000001aa, receiveBuffer);
		if (res == 0x01)
		{
			// e.g. 'new' Transcend 16GB et cetera

			// Interpret response
			if ((receiveBuffer[2] = 0x01) /* voltage range */ and 
			    (receiveBuffer[3] = 0xaa) /* check pattern */)
			{
				// Read OCR
				res = sdcard::sendCommand(sdcard::Command::ReadOcr, 0x00, receiveBuffer);
				XPCC_LOG_DEBUG.printf("OCR = %02x %02x %02x %02x\n", receiveBuffer[0], receiveBuffer[1], receiveBuffer[2], receiveBuffer[3]);
				// OCR is not valid if OCR[31] is not set.
			
				// Card is SDv2 and works from 2.7 to 3.6 volts ??

				// Initialize SDv2
				do {
					res = sdcard::sendCommand(sdcard::Command::SdV2Initialize, 0x00); // 1 << 30);// | 0x100000);
					xpcc::delayMilliseconds(50);
				} while ( (res & 0x01) != 0x00); // Wait until card is not in idle anymore

				if (res != 0x00) {
					XPCC_LOG_ERROR.printf("SdV2Initialize returned %d\n", res);
				}

				res = sdcard::sendCommand(sdcard::Command::ReadOcr, 0x00, receiveBuffer);
				// if (res != 0x00) {
				// 	XPCC_LOG_ERROR.printf("ReadOcr returned %d\n", res);	
				// }
				XPCC_LOG_DEBUG.printf("OCR = %02x %02x %02x %02x\n", receiveBuffer[0], receiveBuffer[1], receiveBuffer[2], receiveBuffer[3]);
				// OCR[0] & 0x80 should be set

				// Interpret result
				// ???

			}
		} 
		// else {
		// 	// e.g. 'old' Nokia Card 1G, Kingstion HC 4GB
		// 	// ???
		// }

		// Read CID
		res = sdcard::sendCommand(sdcard::Command::SendCid, 0);
		if (res != 0x00)
		{
			XPCC_LOG_ERROR.printf("Card does not like read CID.\n");
			while (true)
				{};
		}


		// res = sdcard::sendCommand(sdcard::Command::SetBlockLen, 0x00000200);


		// **********
		// Read Test
		// **********

		XPCC_LOG_DEBUG << "Read Test" << xpcc::endl;
		
		uint8_t block[512] = {0};
		memset(block, 0, 512);

		// res = sdcard::sendCommand(sdcard::Command::ReadSingleBlock, 0);
		// if (res != 0x00) {
		// 	XPCC_LOG_ERROR.printf("res = %02x\n", res);
		// 	while(true)
		// 		{};
		// }

		sdcard::readBlock(block);

		for (uint16_t ii = 0; ii < 512; ++ii)
		{
			if (ii % 16 == 0) {
				XPCC_LOG_DEBUG.printf("%08x ", ii);
			}
			XPCC_LOG_DEBUG.printf("%02x ", block[ii]);
			if (ii % 8 == 7) {
				XPCC_LOG_DEBUG.printf(" ");
			}
			if (ii % 16 == 15) {
				XPCC_LOG_DEBUG.printf("\n");
			}
		}

		usd::Cs::set();
		while(true)
			{};

		XPCC_LOG_DEBUG << "Write Test" << xpcc::endl;
		memset(block, 0x82, 512);
		res = sdcard::sendCommand(sdcard::Command::WriteSingleBlock, 0);
		sdcard::writeBlock(block);




		usd::Cs::set();
	}

	while (true)
	{
		Board::LedGreen::toggle();
	}

	return 0;
}
