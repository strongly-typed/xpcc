#include "../../stm32f429_nucleo144.hpp"
#include <xpcc/debug/logger.hpp>
#include <xpcc/processing/timer.hpp>
#include <xpcc/driver/rtc/ds1302/ds1302.hpp>
#include <xpcc/driver/rtc/ds3231/ds3231.hpp>

// ----------------------------------------------------------------------------
// Set the log level
#undef	XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

// Create an IODeviceWrapper around the Uart Peripheral we want to use
xpcc::IODeviceWrapper< Usart3, xpcc::IOBuffer::BlockIfFull > loggerDevice;

// Set all four logger streams to use the UART
xpcc::log::Logger xpcc::log::debug(loggerDevice);
xpcc::log::Logger xpcc::log::info(loggerDevice);
xpcc::log::Logger xpcc::log::warning(loggerDevice);
xpcc::log::Logger xpcc::log::error(loggerDevice);

typedef I2cMaster2 MyI2cMaster;

xpcc::ds3231::Data data;
xpcc::Ds3231<MyI2cMaster> rtc(data);


class ThreadOne : public xpcc::pt::Protothread
{
public:
	bool
	update()
	{
		PT_BEGIN();

		XPCC_LOG_DEBUG << "Ping the device from ThreadOne" << xpcc::endl;

		// ping the device until it responds
		while (true)
		{
			// we wait until the task started
			if (PT_CALL(rtc.ping())) {
			 	break;
			}
			// otherwise, try again in 100ms
			this->timeout.restart(100);
			PT_WAIT_UNTIL(this->timeout.isExpired());
		}

		XPCC_LOG_DEBUG << "Device responded" << xpcc::endl;

		while (true)
		{
		// 	if (PT_CALL(rtc.initialize()))
		// 		break;
		// 	// otherwise, try again in 100ms
		// 	this->timeout.restart(100);
		// 	PT_WAIT_UNTIL(this->timeout.isExpired());
		// }
		//
		// XPCC_LOG_DEBUG << "Device initialized" << xpcc::endl;
		// this->timeout.restart(1);
		//
		// PT_CALL(distance.setIntegrationTime(10));
		//
		// while (true)
		// {
		// 	stamp = xpcc::Clock::now();
		//
		// 	if (PT_CALL(distance.readDistance()))
		// 	{
		// 		xpcc::vl6180::RangeErrorCode error = distance.getRangeError();
		// 		if (error == distance.RangeErrorCode::NoError)
		// 		{
		// 			uint8_t mm = distance.getData().getDistance();
		// 			XPCC_LOG_DEBUG << "mm: " << mm;
		// 			Board::LedGreen::set(mm > 160);
		// 			Board::LedBlue::set(mm > 110);
		// 			Board::LedRed::set(mm > 25);
		// 		}
		// 		else {
		// 			XPCC_LOG_DEBUG << "Error: " << (uint8_t(error) >> 4);
		// 			Board::LedGreen::set();
		// 			Board::LedBlue::set();
		// 			Board::LedRed::set();
		// 		}
		// 	}
		//
		// 	XPCC_LOG_DEBUG << "\tt=" << (xpcc::Clock::now() - stamp);
		// 	stamp = xpcc::Clock::now();
		//
		// 	if (PT_CALL(distance.readAmbientLight()))
		// 	{
		// 		xpcc::vl6180::ALS_ErrorCode error = distance.getALS_Error();
		// 		if (error == distance.ALS_ErrorCode::NoError)
		// 		{
		// 			uint32_t lux = distance.getData().getAmbientLight();
		// 			XPCC_LOG_DEBUG << "\tLux: " << lux;
		// 		}
		// 		else {
		// 			XPCC_LOG_DEBUG << "\tError: " << (uint8_t(error) >> 4);
		// 		}
		// 	}
		//
		// 	XPCC_LOG_DEBUG << " \tt=" << (xpcc::Clock::now() - stamp) << xpcc::endl;
		//
			PT_WAIT_UNTIL(this->timeout.isExpired());
			this->timeout.restart(40);
		}

		PT_END();
	}

private:
	xpcc::Timeout timeout;
	xpcc::Timestamp stamp;
};

ThreadOne one;

// ----------------------------------------------------------------------------
/**
 * Realtime Clock RTC DS3231 test.
 *
 * This example compares the internal oscillator of the STM with an external
 * RTC.
 *
 * The expected result should be:
 * \code
 * Welcome to RTC DS1302 demo.
 * CPU frequency is 168000000 Hz
 * .........  9.995 seconds by RTC are:  9
 * ..........  9.996 seconds by RTC are:  9
 * ..........  9.997 seconds by RTC are:  9
 * ..........  9.998 seconds by RTC are:  9
 * ..........  9.999 seconds by RTC are:  9
 * .......... 10.000 seconds by RTC are: 10
 * ..........  9.999 seconds by RTC are:  9
 * .......... 10.000 seconds by RTC are: 10
 * /endcode
 */
MAIN_FUNCTION
{
	Board::initialize();

	// initialize Uart3 for XPCC_LOG_
	GpioOutputD8::connect(Usart3::Tx);
	GpioInputD9::connect(Usart3::Rx, Gpio::InputType::PullUp);
	Usart3::initialize<Board::systemClock, 115200>(12);

	XPCC_LOG_INFO << xpcc::endl;
	XPCC_LOG_INFO << "Welcome to RTC DS3231 demo." << xpcc::endl;

	Board::LedBlue::set();

	GpioF0::connect(I2cMaster2::Sda);
	GpioF1::connect(I2cMaster2::Scl);

	MyI2cMaster::initialize<Board::systemClock, 400000>();

	XPCC_LOG_INFO << "\n\nWelcome to DS3231 demo!\n\n";

	xpcc::ShortPeriodicTimer tmr(500);

	while (true)
	{
		one.update();
		if (tmr.execute()) {
			Board::LedGreen::toggle();
		}
	}

	uint16_t tt = 9995;
	xpcc::Timeout timeout;
	timeout.restart(tt);

	// Periodically report progress
	xpcc::PeriodicTimer pt(1000);

	XPCC_LOG_DEBUG.printf("CPU frequency is %ld Hz\n", Board::systemClock::Frequency);

	while(true)
	{
		if (pt.execute()) {
			XPCC_LOG_DEBUG << ".";
			Board::LedBlue::toggle();
		}
		if (timeout.execute())
		{
			// xpcc::ds1302::Data rtc_data;
			// ds1302::readRtc(rtc_data);
			uint8_t seconds = 0; // rtc_data.getSeconds();

			XPCC_LOG_DEBUG.printf(" %2d.%03d seconds by RTC are: %2d\n",
				tt / 1000,
				tt % 1000,
				seconds);

			// Set seconds to 0
			// ds1302::write(0x80, 0x00);
			if (seconds >= 10) {
				--tt;
			} else {
				++tt;
			}
			timeout.restart(tt);
		}
	};
	return 0;
}
