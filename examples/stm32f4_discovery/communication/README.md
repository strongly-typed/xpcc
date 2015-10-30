# XPCC Communication over Raw Ethernet Frames using ENC28J60 Stand-Alone Ethernet Controller with SPI Interface

Two STM32F4 Discovery boards communicate with the XPCC protocol over raw Ethernet frames.

The class Enc28j60Can provides the same interface as a CAN device so that a CanConnector can use this backend.

Only Ethernet broadcast frames (destination FF:FF:FF:FF:FF:FF) are used.


## Outlook
A further version should make use of the MAC addressing of Ethernet frames by writing a new Ethernet backend. Framgemntation will not be necessary because the payload of an Ethernet frame is much larger than a CAN message.
