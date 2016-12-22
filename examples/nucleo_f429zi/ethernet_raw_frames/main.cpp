#include <xpcc/architecture/platform.hpp>
#include <xpcc/processing/timer/periodic_timer.hpp>

#include <xpcc/communication.hpp>
#include <xpcc/communication/xpcc/backend/ethernet.hpp>

#include "component_receiver/receiver.hpp"
#include "component_sender/sender.hpp"

#include "communication/postman.hpp"
#include "communication/identifiers.hpp"


#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_eth.h"

// Declare a ETH_HandleTypeDef handle structure, for example:
ETH_HandleTypeDef  heth;

extern "C" void ETH_IRQHandler(void)
{
  HAL_ETH_IRQHandler(&heth);
}


uint32_t SystemCoreClock = HSI_VALUE;

/* 4 Tx buffers of size ETH_TX_BUF_SIZE  */
// #define ETH_TXBUFNB                    ((uint32_t)4)       

/* 4 Tx buffers of size ETH_TX_BUF_SIZE  */
// #define ETH_RXBUFNB                    ((uint32_t)4)       

/* Ethernet Tx DMA Descriptor */
ETH_DMADescTypeDef xpcc_aligned(4) DMATxDscrTab[ETH_TXBUFNB];

ETH_DMADescTypeDef xpcc_aligned(4) DMARxDscrTab[ETH_RXBUFNB];

/* Ethernet Receive Buffer */
uint8_t xpcc_aligned(4) Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];

/* Ethernet Transmit Buffer */
uint8_t xpcc_aligned(4) Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];

extern "C" void HAL_Delay(__IO uint32_t Delay)
{
	xpcc::delayMilliseconds(Delay);
}

/* Callback for Init */
extern "C" void HAL_ETH_MspInit(ETH_HandleTypeDef * /* heth */)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable GPIOs clocks */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

/* Ethernet pins configuration ************************************************/
  /*
        RMII_REF_CLK ----------------------> PA1
        RMII_MDIO -------------------------> PA2
        RMII_MDC --------------------------> PC1
        RMII_MII_CRS_DV -------------------> PA7
        RMII_MII_RXD0 ---------------------> PC4
        RMII_MII_RXD1 ---------------------> PC5
        RMII_MII_RXER ---------------------> PG2
        RMII_MII_TX_EN --------------------> PG11
        RMII_MII_TXD0 ---------------------> PG13
        RMII_MII_TXD1 ---------------------> PB13
  */

  /* Configure PA1, PA2 and PA7 */
  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL; 
  GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure PB13 */
  GPIO_InitStructure.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* Configure PC1, PC4 and PC5 */
  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure PG2, PG11, PG13 and PG14 */
  GPIO_InitStructure.Pin =  GPIO_PIN_2 | GPIO_PIN_11 | GPIO_PIN_13;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
  
  /* Enable the Ethernet global Interrupt */
  HAL_NVIC_SetPriority(ETH_IRQn, 0x7, 0);
  HAL_NVIC_EnableIRQ(ETH_IRQn);
  
  /* Enable ETHERNET clock  */
  __HAL_RCC_ETH_CLK_ENABLE();
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  // RCC_ClkInitTypeDef RCC_ClkInitStruct;
  // RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  // __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  // __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  // RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  // RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  // RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  // RCC_OscInitStruct.PLL.PLLM = 8;
  // RCC_OscInitStruct.PLL.PLLN = 360;
  // RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  // RCC_OscInitStruct.PLL.PLLQ = 7;
  // if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  // {
  //  while(1) {};
  // }
  
  // if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  // {
  //  while(1) {};
  // }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  // RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  // RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  // RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  // if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  // {
  //  while(1) {};
  // }
}

/** ***************************************************************** */

namespace xpcc {

extern "C" {
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_eth.h"
}


typedef uint8_t EthernetFrame[64];

class EthernetDevice
{
public:
  static bool
  sendMessage(EthernetFrame &frame)
  {
    uint8_t *buffer = (uint8_t *)(heth.TxDesc->Buffer1Addr);
    memcpy(buffer, frame, 64);
    HAL_ETH_TransmitFrame(&heth, 64);

    return true;
  }

  static bool
  getMessage(uint8_t &length, EthernetFrame &frame)
  {
    if (HAL_ETH_GetReceivedFrame(&heth) == HAL_OK)
    {
      /* Get buffer */
      __IO ETH_DMADescTypeDef *dmarxdesc = heth.RxFrameInfos.FSRxDesc;
      uint8_t const *buffer = (uint8_t *)heth.RxFrameInfos.buffer;
      length = heth.RxFrameInfos.length;

      /* Copy buffer to frame */
      if (length <= 64) {
        memcpy(frame, buffer, length);
      }

      /* Release descriptors to DMA */
      /* Point to first descriptor */
      dmarxdesc = heth.RxFrameInfos.FSRxDesc;
      /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
      for (uint32_t i=0; i< heth.RxFrameInfos.SegCount; i++)
      {
        dmarxdesc->Status |= ETH_DMARXDESC_OWN;
        dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
      }

      /* Clear Segment_Count */
      heth.RxFrameInfos.SegCount = 0;

      /* When Rx Buffer unavailable flag is set: clear it and resume reception */
      if ((heth.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
      {
        /* Clear RBUS ETHERNET DMA flag */
        heth.Instance->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        heth.Instance->DMARPDR = 0;
      }

      return true;
    } else {
      return false;
    }
  }

};


} // xpcc namespace

/** ***************************************************************** */

static uint8_t
containerLut(const xpcc::Header& header)
{
  robot::container::Identifier ret = robot::container::Identifier::Gui;

  switch (header.destination)
  {
    case robot::component::Identifier::SENDER:
      ret = robot::container::Identifier::Sender;
      break;
    case robot::component::Identifier::RECEIVER:
      ret = robot::container::Identifier::Receiver;
      break;
  }

  return static_cast<uint8_t>(ret);

}

static xpcc::EthernetDevice ethDevice;
static xpcc::EthernetConnector< xpcc::EthernetDevice > connector(ethDevice, containerLut);

// create an instance of the generated postman
Postman postman;

xpcc::Dispatcher dispatcher(&connector, &postman);

namespace component
{
  Receiver receiver(robot::component::RECEIVER, dispatcher);
  Sender sender(robot::component::SENDER, dispatcher);
}


using namespace Board;

int
main()
{
	Board::initialize();
	Leds::setOutput();

	// Use the logging streams to print some messages.
	// Change XPCC_LOG_LEVEL above to enable or disable these messages
	XPCC_LOG_DEBUG   << "debug"   << xpcc::endl;
	XPCC_LOG_INFO    << "info"    << xpcc::endl;
	XPCC_LOG_WARNING << "warning" << xpcc::endl;
	XPCC_LOG_ERROR   << "error"   << xpcc::endl;

	uint32_t counter(0);

	// Fill parameters of Init structure in heth handle
  // SENDER = 0x01,
  // RECEIVER = 0x02,

  // Sender
	uint8_t macaddress[6]= { 0x8E, 0x52, 0x43, 0x41, 
    static_cast<uint8_t>(robot::container::Identifier::Sender),
    robot::component::Identifier::SENDER };

  // Receiver
  // uint8_t macaddress[6]= { 0x8E, 0x52, 0x43, 0x41, 
  //   static_cast<uint8_t>(robot::container::Identifier::Receiver),
  //   robot::component::Identifier::RECEIVER };

	#define LAN8742A_PHY_ADDRESS            0x00

	// HAL_Init();
	SystemClock_Config(); 

  heth.Instance = ETH;  
  heth.Init.MACAddr = macaddress;
  heth.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
  heth.Init.Speed = ETH_SPEED_100M;
  heth.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
  heth.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
  heth.Init.RxMode = ETH_RXINTERRUPT_MODE;
  heth.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
  heth.Init.PhyAddress = LAN8742A_PHY_ADDRESS;

  // Call HAL_ETH_Init() API to initialize the Ethernet peripheral (MAC, DMA, ...)
  XPCC_LOG_DEBUG.printf("HAL_ETH_Init ... ");
  if (HAL_ETH_Init(&heth) == HAL_OK) {
    XPCC_LOG_DEBUG.printf("OK\n");
  } else {
    XPCC_LOG_DEBUG.printf("FAIL\n");
  }

  // Initialize the ETH low level resources through the HAL_ETH_MspInit() API:

  // (##) Enable the Ethernet interface clock using
  // __HAL_RCC_ETHMAC_CLK_ENABLE();
  // __HAL_RCC_ETHMACTX_CLK_ENABLE();
  // __HAL_RCC_ETHMACRX_CLK_ENABLE();

	// (##) Initialize the related GPIO clocks
	// (##) Configure Ethernet pin-out
	// (##) Configure Ethernet NVIC interrupt (IT mode)   

	// (#)Initialize Ethernet DMA Descriptors in chain mode and point to allocated buffers:
	// (##) HAL_ETH_DMATxDescListInit(); for Transmission process
	// (##) HAL_ETH_DMARxDescListInit(); for Reception process

	/* Initialize Tx Descriptors list: Chain Mode */
	HAL_ETH_DMATxDescListInit(&heth, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
     
 	/* Initialize Rx Descriptors list: Chain Mode  */
	HAL_ETH_DMARxDescListInit(&heth, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

	// (#)Enable MAC and DMA transmission and reception:
	// (##) HAL_ETH_Start();
	HAL_ETH_Start(&heth);


  // uint8_t writeCounter = 0;
  // xpcc::ShortPeriodicTimer tmr(1500);

	while (true)
	{
		Leds::write(1 << (counter % Leds::width));

    // deliver received messages
    dispatcher.update();

    // component::receiver.update();
    component::sender.update();

		// xpcc::delayMilliseconds(Button::read() ? 100 : 500);

		// XPCC_LOG_INFO << "loop: " << counter++ << xpcc::endl;

		// if(HAL_ETH_GetReceivedFrame(&heth) == HAL_OK)
		// {
		// 	++counter;
		// 	__IO ETH_DMADescTypeDef *dmarxdesc = heth.RxFrameInfos.FSRxDesc;
		// 	uint8_t *buffer = (uint8_t *)heth.RxFrameInfos.buffer;
		// 	uint32_t length = heth.RxFrameInfos.length;
		// 	XPCC_LOG_DEBUG.printf("ETH: RX: %d ***********************\n", length);

		// 	for (uint32_t ii = 0; ii < length; ++ii) {
		// 		XPCC_LOG_DEBUG.printf("%02x ", *buffer);
		// 		++buffer;

		// 		if (ii % 8 == 7) {
		// 			XPCC_LOG_DEBUG.printf(" ");
		// 		}
		// 		if (ii % 16 == 15) {
		// 			XPCC_LOG_DEBUG.printf("\n");
		// 		}
		// 	}
		// 	XPCC_LOG_DEBUG.printf("\n");


		// 	/* Release descriptors to DMA */
		// 	/* Point to first descriptor */
		// 	dmarxdesc = heth.RxFrameInfos.FSRxDesc;
		// 	/* Set Own bit in Rx descriptors: gives the buffers back to DMA */
		// 	for (uint32_t i=0; i< heth.RxFrameInfos.SegCount; i++)
		// 	{  
		// 		dmarxdesc->Status |= ETH_DMARXDESC_OWN;
		// 		dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
		// 	}

		// 	/* Clear Segment_Count */
		// 	heth.RxFrameInfos.SegCount =0;

		// 	/* When Rx Buffer unavailable flag is set: clear it and resume reception */
		// 	if ((heth.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)  
		// 	{
		// 		/* Clear RBUS ETHERNET DMA flag */
		// 		heth.Instance->DMASR = ETH_DMASR_RBUS;
		// 		/* Resume DMA reception */
		// 		heth.Instance->DMARPDR = 0;
		// 	}

		// }

    // if (tmr.execute())
    // {
    //   uint8_t *buffer = (uint8_t *)(heth.TxDesc->Buffer1Addr);

    //   for (uint8_t ii = 0; ii < 64; ++ii) {
    //     buffer[ii] = 0xff;
    //   }

    //   HAL_ETH_TransmitFrame(&heth, 64);

    //   ++writeCounter;
    // }

	}

	return 0;
}
