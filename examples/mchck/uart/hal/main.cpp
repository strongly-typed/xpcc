#include <xpcc/architecture/driver/atomic/lock.hpp>
#include <xpcc/architecture.hpp>
#include "../../mchck.hpp"


class DummyClock
{
public:
	static constexpr int Uart0 = 21 * 1000 * 1000 / 2;
};


// ----------------------------------------------------------------------------
MAIN_FUNCTION
{

	StatusLed::setOutput(xpcc::Gpio::HIGH);

	// Initialize Uart Hal
	PL12Output::connect(UartHal0::Tx);
	UartHal0::initialize<DummyClock, 115200>(UartHal0::Parity::Odd);
	UartHal0::setTransmitterEnable(true);

/*
	int i = 0;
	while (i < 10)
	{
		if(UartHal0::isTransmitRegisterEmpty()) {
			UartHal0::write('A');
			++i;
		}
	}
*/
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
	PIT_MCR = 0x00;
	uint32_t IRQn = 30;
	NVIC_SetPriority(static_cast<IRQn_Type>(IRQn), 2);
	NVIC_EnableIRQ(static_cast<IRQn_Type>(IRQn));
	PIT_LDVAL0 = 0x2faf080;
	PIT_TCTRL0 = PIT_TCTRL_TIE_MASK;
	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
	PIT_TFLG0 |= 1;


	// UartHal0::setTransmitWatermark(1);

	//UartHal0::enableInterrupt(UartHal0::Interrupt::TxEmpty);
	// UART0_C5 = 0;
	// UART0_C2 = UART_C2_TE_MASK | UART_C2_TCIE_MASK;

	// UartHal0::enableInterruptVector(true, 14);
	//uint32_t IRQn = 16;
	//NVIC_SetPriority(static_cast<IRQn_Type>(IRQn), 2);
	//NVIC_EnableIRQ(static_cast<IRQn_Type>(16));
	//NVIC->ISER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));


	//UartHal0::setTransmitterEnable(true);


	while (1)
	{
	}

	return 0;
}


// Interrupt Handler
extern "C" void
PIT0_IRQHandler(void)
{
	StatusLed::toggle();
}

/*


extern "C" void
UART0_RX_TX_IRQHandler(void)
{
	while(true) {
		StatusLed::toggle();
		xpcc::delay_ms(300);
	}


	// UartHal0::getInterruptFlags();
	// if(UartHal0::isTransmitRegisterEmpty()) {
	//	UartHal0::write('A');
	//}
}

extern "C" void
UART0_RX_TX_IRQHandler_2(void)
{
	while(true) {
		StatusLed::toggle();
		xpcc::delay_ms(100);
	}
}*/
