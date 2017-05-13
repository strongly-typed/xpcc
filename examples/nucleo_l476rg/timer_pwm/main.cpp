#include <xpcc/architecture/platform.hpp>
#include <xpcc/processing.hpp>
#include <xpcc/io/iostream.hpp>
#include <xpcc/ui/led.hpp>

// set new log level
#undef XPCC_LOG_LEVEL
#define XPCC_LOG_LEVEL xpcc::log::DEBUG


xpcc::ui::Led red([](uint8_t brightness)
{
	Timer2::setCompareValue(3, xpcc::ui::table22_16_256[brightness]);
});

xpcc::ui::Led green([](uint8_t brightness)
{
	Timer3::setCompareValue(1, xpcc::ui::table22_16_256[brightness]);
	// XPCC_LOG_DEBUG.printf("G: %3d\n", brightness);
});

xpcc::ui::Led blue([](uint8_t brightness)
{
	Timer2::setCompareValue(2, xpcc::ui::table22_16_256[brightness]);
});

namespace Led {
using Blue  = GpioOutputB3;   // TIM2_CH2
using Red   = GpioOutputB10;  // TIM2_CH3
using Green = GpioOutputB4;   // TIM3_CH1
};

class ThreadTwo : public xpcc::pt::Protothread
{
public:
	ThreadTwo():
		timer(1),
		ii(0),
		rgb(red, green, blue)
	{
		Led::Blue::connect(Timer2::Channel2);
		Led::Red::connect(Timer2::Channel3);

		Led::Green::connect(Timer3::Channel1);


		// set up the timer for 16bit PWM
		Timer2::enable();
		Timer2::setMode(Timer2::Mode::UpCounter);

		// 48 MHz / 1 / 2^16 ~ 732 Hz refresh rate
		Timer2::setPrescaler(1);
		Timer2::setOverflow(65535);

		// configure the output channels
		Timer2::configureOutputChannel(/* Channel */ 2, Timer2::OutputCompareMode::Pwm, 0);
		Timer2::configureOutputChannel(/* Channel */ 3, Timer2::OutputCompareMode::Pwm, 0);
		Timer2::applyAndReset();

		Timer3::enable();
		Timer3::setMode(Timer3::Mode::UpCounter);

		Timer3::setPrescaler(1);
		Timer3::setOverflow(65535);
		Timer3::configureOutputChannel(/* Channel */ 1, Timer3::OutputCompareMode::Pwm, 0);

		Timer3::applyAndReset();

		// start the timer
		Timer2::start();
		Timer3::start();
	}

	bool
	update()
	{
		PT_BEGIN();

		{
			xpcc::color::RgbT<> rr(0, 0, 0x80);
			rgb.setColor(rr);
		}

		while(true)
		{
			PT_WAIT_UNTIL(timer.execute());
			rgb.update();

			if (/* Board::Button::read() and */not rgb.isFading()) {
				uint8_t ra = rand() % 256;
				xpcc::color::HsvT<> hsv(ra, 255, 255);
				// XPCC_LOG_DEBUG.printf("fade To H=%3d\n", ra);
				xpcc::delayMilliseconds(10);
				// rgb.setColor(hsv);
				rgb.fadeTo(hsv, 1000);
				// if (ra < RAND_MAX / 10) {
			}
		}

		PT_END();
	}

private:
	xpcc::ShortPeriodicTimer timer;
	uint16_t ii;

	xpcc::ui::RgbLed rgb;
};

ThreadTwo two;

// ----------------------------------------------------------------------------
int
main()
{
	Board::initialize();

	XPCC_LOG_DEBUG << "\n\nWelcome to PWM LED demo!\n\n";

	while (true)
	{
		two.update();
		Board::LedGreen::toggle();
	}

	return 0;
}
