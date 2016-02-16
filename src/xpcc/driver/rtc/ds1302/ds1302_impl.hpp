#ifndef XPCC_DS1302_HPP
#error "Do not include ds1302_impl.hpp. Only include ds1302.hpp"
#endif

template < class PinSet >
void
xpcc::Ds1302< PinSet >::initialize()
{
  Ce::reset();
  Sclk::reset();
  Io::setInput();
}

template< class PinSet >
void
xpcc::Ds1302< PinSet >::writeByte(uint8_t byte)
{
  Io::setOutput();
  xpcc::delayMicroseconds(delay_ce);

  for (uint8_t ii = 8; ii > 0; --ii)
  {
    Io::set( byte & 0x01);
    xpcc::delayMicroseconds(delay_clk);
    Sclk::set();
    xpcc::delayMicroseconds(delay_clk);
    Sclk::reset();

    byte >>= 1;
  }
}

template< class PinSet >
void
xpcc::Ds1302< PinSet >::write(const uint8_t addr, const uint8_t data)
{
  Ce::set();
  writeByte(addr);
  writeByte(data);
  Io::setInput();
  Ce::reset();
}

template< class PinSet >
uint8_t
xpcc::Ds1302< PinSet >::read(const uint8_t addr)
{
  Ce::set();
  writeByte(addr);

  uint8_t ret = 0;

  Io::setInput();
  xpcc::delayMicroseconds(delay_ce);
  for (uint8_t ii = 8; ii > 0; --ii)
  {
    bool rr = Io::read();
    ret >>= 1;
    ret |= (rr << 7);
    Sclk::set();
    xpcc::delayMicroseconds(delay_clk);
    Sclk::reset();
    xpcc::delayMicroseconds(delay_clk);
  }

  Ce::reset();

  return ret;
}


template< class PinSet >
void
xpcc::Ds1302< PinSet >::readRtc(ds1302::Data &storage)
{
	Ce::set();
	writeByte(0x81); // Start address
	Io::setInput();

	xpcc::delayMicroseconds(delay_ce);

	for (uint8_t jj = 0; jj < 8; ++jj)
	{
		uint8_t ret = 0;
		for (uint8_t ii = 8; ii > 0; --ii)
		{
		  bool rr = Io::read();
		  ret >>= 1;
		  ret |= (rr << 7);
		  Sclk::set();
		  xpcc::delayMicroseconds(delay_clk);
		  Sclk::reset();
		  xpcc::delayMicroseconds(delay_clk);
		}
		storage.data[jj] = ret;
	}

	Ce::reset();

}
