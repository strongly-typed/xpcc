#include <xpcc/architecture/platform.hpp>
#include <xpcc/processing.hpp>
#include <xpcc/io/iostream.hpp>
#include <xpcc/architecture/interface/gpio.hpp>
#include <xpcc/driver/pressure/bmp085.hpp>
#include <xpcc/driver/display/ssd1306.hpp>

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

	stream << "\n\nWelcome to BMP085 demo!\n\n";

	while (true)
	{
		one.update();
		Board::LedGreen::toggle();
	}

	return 0;
}
