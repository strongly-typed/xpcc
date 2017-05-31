/* Copyright (c) 2017, Sascha Schade (strongly-typed)
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 * ------------------------------------------------------------------------- */
#include <xpcc/architecture/platform.hpp>
#include <xpcc/processing/timer.hpp>
#include <xpcc/driver/temperature/ds18b20.hpp>
#include <xpcc/driver/display/ssd1306.hpp>

using namespace xpcc::stm32;

using OneWirePin = Board::A0;

using Sda = Board::D14;
using Scl = Board::D15;
// using MyI2cMaster = I2cMaster1;
using MyI2cMaster = xpcc::SoftwareI2cMaster<Board::D15, Board::D14>;
xpcc::Ssd1306<MyI2cMaster> display;

class Environment{
public:
    Environment() : dirty(false)
    {}

public:
    int16_t tempA;
    int16_t tempB;

    bool dirty;
};

Environment environment;

class ThreadDisplay : public xpcc::pt::Protothread
{
public:
    ThreadDisplay()
    {
    }

    bool
    update()
    {
        PT_BEGIN();

        while(true)
        {
                // we wait until the task started
                if (PT_CALL(display.ping())) {
                        break;
                }
                // otherwise, try again in 100ms
                this->timeout.restart(100);
                PT_WAIT_UNTIL(this->timeout.isExpired());
        }

        XPCC_LOG_INFO << "[disp] Display responded" << xpcc::endl;

        display.initializeBlocking();
        display.setFont(xpcc::font::Assertion);
        display << "Hello Steffen!";
        display.update();

        while(true)
        {
            display.clear();
            display << "Hello Steffen! " << counter++ << xpcc::endl;
            display.printf("Temp A = %3d.%02d\n", environment.tempA / 100, environment.tempA % 100);
            display.printf("Temp B = %3d.%02d\n", environment.tempB / 100, environment.tempB % 100);
            display.update();

            PT_YIELD();
        }

        PT_END();
	}

protected:
	xpcc::ShortTimeout timeout;
	xpcc::Ssd1306<MyI2cMaster, /* Height */ 64> display;

    uint8_t counter;
};

class ThreadThermometer : public xpcc::pt::Protothread
{
public:
    ThreadThermometer()
    {
    }

    bool
    update()
    {
        PT_BEGIN();


        while(true)
        {
            XPCC_LOG_INFO << "[1w] Init:" << xpcc::endl;

        	ow.initialize();

        	if (ow.touchReset()) {
        		XPCC_LOG_INFO << "[1w] devices found." << xpcc::endl;
        		break;
        	}

        	XPCC_LOG_INFO << "[1w] No devices found!" << xpcc::endl;

            // otherwise, try again in 100ms
            this->timeout.restart(100);
            PT_WAIT_UNTIL(this->timeout.isExpired());
        }

		// search for connected DS18B20 devices
		ow.resetSearch(0x28);

		if (ow.searchNext(romA)) {
			XPCC_LOG_INFO << "[1w] found: " << xpcc::hex;
			for (uint8_t ii = 0; ii < 8; ++ii) {
				XPCC_LOG_INFO << romA[ii];
			}
			XPCC_LOG_INFO << xpcc::ascii << xpcc::endl;
		    this->timeout.restart(100);
		    PT_WAIT_UNTIL(this->timeout.isExpired());
		}
        if (ow.searchNext(romB)) {
            XPCC_LOG_INFO << "[1w] found: " << xpcc::hex;
            for (uint8_t ii = 0; ii < 8; ++ii) {
                XPCC_LOG_INFO << romB[ii];
            }
            XPCC_LOG_INFO << xpcc::ascii << xpcc::endl;
            this->timeout.restart(100);
            PT_WAIT_UNTIL(this->timeout.isExpired());
        }

		XPCC_LOG_INFO << "[1w] Search finished!" << xpcc::endl;

		while (true)
		{
            {
                // read the temperature from a connected DS18B20
                xpcc::Ds18b20< xpcc::SoftwareOneWireMaster<OneWirePin> > ds18b20A(romA);
                ds18b20A.startConversion();

                xpcc::delayMilliseconds(750);
        
                if (ds18b20A.isConversionDone())
    			{
    				{
    					int16_t temperature = ds18b20A.readTemperature();
    					XPCC_LOG_INFO << "TemperatureA: " << temperature << xpcc::endl;
                        environment.tempA = temperature;
    				}
    		        xpcc::delayMilliseconds(100);

    				// ds18b20A.startConversion();
    			}
            }

            {
                // read the temperature from a connected DS18B20
                xpcc::Ds18b20< xpcc::SoftwareOneWireMaster<OneWirePin> > ds18b20B(romB);
                ds18b20B.startConversion();

                xpcc::delayMilliseconds(750);
        
                if (ds18b20B.isConversionDone())
                {
                    {
                        int16_t temperature = ds18b20B.readTemperature();
                        XPCC_LOG_INFO << "TemperatureB: " << temperature << xpcc::endl;
                        environment.tempB = temperature;
                    }
                    xpcc::delayMilliseconds(100);
                }
            }

            PT_YIELD();
		}

        PT_END();
    }

protected:
	xpcc::ShortTimeout timeout;
	xpcc::SoftwareOneWireMaster<OneWirePin> ow;
	uint8_t romA[8];
    uint8_t romB[8];
};

ThreadDisplay threadDisplay;
ThreadThermometer ThreadThermometer;

int
main()
{
	Board::initialize();

    Board::D14::connect(MyI2cMaster::Sda);
    Board::D15::connect(MyI2cMaster::Scl);
    MyI2cMaster::initialize<Board::systemClock, MyI2cMaster::Baudrate::Standard>();

    while(true)
    {
    	threadDisplay.update();
    	ThreadThermometer.update();
    }

}

