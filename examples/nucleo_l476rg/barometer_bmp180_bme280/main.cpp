#include <xpcc/architecture/platform.hpp>
#include <xpcc/processing.hpp>
#include <xpcc/io/iostream.hpp>
#include <xpcc/architecture/interface/gpio.hpp>
#include <xpcc/driver/pressure/bmp085.hpp>
#include <xpcc/driver/display/ssd1306.hpp>
#include <xpcc/driver/display/nokia5110.hpp>
#include <xpcc/ui/display/image/home_16x16.hpp>

xpcc::IODeviceWrapper< Board::stlink::Uart, xpcc::IOBuffer::BlockIfFull > device;
xpcc::IOStream stream(device);

/**
 * Example to demonstrate a XPCC driver for barometer and
 * thermometer BMP180 and BME280 (which are compatible).
 *
 * PB7	SDA
 * PB6	SCL
 *
 * GND and +3V3 are connected to the barometer module.
 *
 */

// typedef I2cMaster1 MyI2cMaster;
typedef xpcc::SoftwareI2cMaster</* SCL */ GpioC2, /* SDA */ GpioC3> MyI2cMaster;

namespace lcd
{
	using Clk = xpcc::stm32::GpioA5;
	using Din = xpcc::stm32::GpioA7;

	using Dc = xpcc::stm32::GpioOutputB6;
	using Ce = xpcc::stm32::GpioOutputC7;
	using Reset = xpcc::stm32::GpioOutputA9;
}

// typedef xpcc::SoftwareSpiMaster< lcd::Clk, lcd::Din> mySpiMaster;
typedef SpiMaster1 mySpiMaster;

class ThreadOne : public xpcc::pt::Protothread
{
public:
	ThreadOne() :
		barometer(data, 0x77)
	{
	}

	bool
	update()
	{
		PT_BEGIN()

		lcd::Reset::setOutput(xpcc::Gpio::Low);
		lcd::Ce::setOutput(xpcc::Gpio::High);
		lcd::Dc::setOutput(xpcc::Gpio::Low);

		// Initialize
		lcd::Reset::set();
		lcDisplay.initialize();

		lcDisplay.setCursor(0, 0);

		// Write the standard welcome message ;-)
		lcDisplay << "Hello xpcc.io";
		lcDisplay.update();

		PT_CALL(display.initialize());
		display.setFont(xpcc::font::Assertion);

		stream << "Ping the device from ThreadOne" << xpcc::endl;

		// ping the device until it responds
		while(true)
		{
			// we wait until the task started
			if (PT_CALL(barometer.ping()))
				break;
			// otherwise, try again in 100ms
			this->timeout.restart(100);
			PT_WAIT_UNTIL(this->timeout.isExpired());
		}

		stream << "Device responded" << xpcc::endl;

		// Configure the device until it responds
		while(true)
		{
			// we wait until the task started
			if (PT_CALL(barometer.initialize()))
				break;
			// otherwise, try again in 100ms
			this->timeout.restart(100);
			PT_WAIT_UNTIL(this->timeout.isExpired());
		}

		stream << "Device configured" << xpcc::endl;

		static xpcc::bmp085::Calibration &cal = data.getCalibration();

		stream << "Calibration data is: \n";
		stream.printf(" ac1 %d\n", cal.ac1);
		stream.printf(" ac2 %d\n", cal.ac2);
		stream.printf(" ac3 %d\n", cal.ac3);
		stream.printf(" ac4 %d\n", cal.ac4);
		stream.printf(" ac5 %d\n", cal.ac5);
		stream.printf(" ac6 %d\n", cal.ac6);
		stream.printf(" b1 %d\n", cal.b1);
		stream.printf(" b2 %d\n", cal.b2);
		stream.printf(" mb %d\n", cal.mb);
		stream.printf(" mc %d\n", cal.mc);
		stream.printf(" md %d\n", cal.md);

		while (true)
		{
			static xpcc::ShortPeriodicTimer timer(250);

			PT_WAIT_UNTIL(timer.execute());

			// Returns when new data was read from the sensor
			PT_CALL(barometer.readout());

			{
				int16_t temp  = data.getTemperature();
				int32_t press = data.getPressure();

				stream.printf("Calibrated temperature in 0.1 degree Celsius is: %d\n",   temp  );
				stream.printf("Calibrated pressure in Pa is                   : %d\n\n", press );

				display.clear();

				display.drawPixel(0,0);
				display.drawPixel(127,0);
				display.drawPixel(0,31);
				display.drawPixel(127,31);

				display.setCursor(0, 0);
				display.printf("T = %2d.%1d C xpcc", temp/10, temp %10);

				display.setCursor(0, 16);
				display.printf("P = %6d Pa", press);

				if (true) {
					lcDisplay.clear();

					lcDisplay.drawPixel(0,0);
					lcDisplay.drawPixel(83,0);
					lcDisplay.drawPixel(83,47);
					lcDisplay.drawPixel(0,47);

					lcDisplay.setCursor(0,0);
					lcDisplay.printf("T = %2d.%1d C", temp/10, temp %10);

					lcDisplay.setCursor(0, 10);
					lcDisplay.printf("P = %6d Pa", press);

					lcDisplay.setCursor(0, 20);
					lcDisplay.printf("xpcc");

					lcDisplay.drawImage(xpcc::glcd::Point(0, 32), xpcc::accessor::asFlash(bitmap::home_16x16));

					lcDisplay.update();
				}

				display.update();
			}
		}

		PT_END();
	}

private:
	xpcc::ShortTimeout timeout;

	xpcc::bmp085::Data data;
	xpcc::Bmp085<MyI2cMaster> barometer;

	xpcc::Ssd1306<MyI2cMaster, 32> display;

	xpcc::Nokia5110< mySpiMaster, lcd::Ce, lcd::Dc, lcd::Reset > lcDisplay;
};


ThreadOne one;

// ----------------------------------------------------------------------------
int
main()
{
	Board::initialize();

	GpioC3::connect(MyI2cMaster::Sda, Gpio::InputType::PullUp);
	GpioC2::connect(MyI2cMaster::Scl, Gpio::InputType::PullUp);
    MyI2cMaster::initialize<Board::systemClock, MyI2cMaster::Baudrate::Standard>();

	lcd::Din::connect(SpiMaster1::Mosi);
	lcd::Clk::connect(SpiMaster1::Sck);
	// lcd::Clk::setOutput();
	// lcd::Din::setOutput();
	mySpiMaster::initialize<Board::systemClock, 3000000ul>();

	stream << "\n\nWelcome to BMP085 demo!\n\n";

	while (true)
	{
		one.update();
		Board::LedGreen::toggle();
	}

	return 0;
}
