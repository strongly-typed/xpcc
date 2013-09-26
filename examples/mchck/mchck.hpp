// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
* All Rights Reserved.
*
* The file is part of the xpcc library and is released under the 3-clause BSD
* license. See the file `LICENSE` for the full license governing this code.
*/
// ----------------------------------------------------------------------------

//
// mchck Board File
// https://mchck.org/
//

#ifndef XPCC_MCHCK_HPP
#define XPCC_MCHCK_HPP

using namespace xpcc::kinetis;

// Port A
typedef GpioA0       SwdClk;
typedef GpioOutputA0 SwdClkOutput;
typedef GpioInputA0  SwdClkInput;
typedef GpioA1       PL4;
typedef GpioOutputA1 PL4Output;
typedef GpioInputA1  PL4Input;
typedef GpioA2       PL12;
typedef GpioOutputA2 PL12Output;
typedef GpioInputA2  PL12Input;
typedef GpioA3       SwdIO;
typedef GpioOutputA3 SwdIOOutput;
typedef GpioInputA3  SwdIOInput;
typedef GpioA4       PL5;
typedef GpioOutputA4 PL5Output;
typedef GpioInputA4  PL5Input;
typedef GpioA18       PL6;
typedef GpioOutputA18 PL6Output;
typedef GpioInputA18  PL6Input;
typedef GpioA19       PL7;
typedef GpioOutputA19 PL7Output;
typedef GpioInputA19  PL7Input;

// Port B
typedef GpioB0       PL8;
typedef GpioOutputB0 PL8Output;
typedef GpioInputB0  PL8Input;
typedef GpioB1       PL9;
typedef GpioOutputB1 PL9Output;
typedef GpioInputB1  PL9Input;
typedef GpioB2       PL10;
typedef GpioOutputB2 PL10Output;
typedef GpioInputB2  PL10Input;
typedef GpioB3       PL11;
typedef GpioOutputB3 PL11Output;
typedef GpioInputB3  PL11Input;
typedef GpioB16       PR6;
typedef GpioOutputB16 PR6Output;
typedef GpioInputB16  PR6Input;
typedef GpioB17       PU14;
typedef GpioOutputB17 PU14Output;
typedef GpioInputB17  PU14Input;

// Port C
typedef GpioC0       PR5;
typedef GpioOutputC0 PR5Output;
typedef GpioInputC0  PR5Input;
typedef GpioC1       PU13;
typedef GpioOutputC1 PU13Output;
typedef GpioInputC1  PU13Input;
typedef GpioC2       PR3;
typedef GpioOutputC2 PR3Output;
typedef GpioInputC2  PR3Input;
typedef GpioC3       PU11;
typedef GpioOutputC3 PU11Output;
typedef GpioInputC3  PU11Input;
typedef GpioC4       PR1;
typedef GpioOutputC4 PR1Output;
typedef GpioInputC4  PR1Input;
typedef GpioC5       PU10;
typedef GpioOutputC5 PU10Output;
typedef GpioInputC5  PU10Input;
typedef GpioC6       PR2;
typedef GpioOutputC6 PR2Output;
typedef GpioInputC6  PR2Input;
typedef GpioC7       PU9;
typedef GpioOutputC7 PU9Output;
typedef GpioInputC7  PU9Input;

// Port D
typedef GpioD0       PU8;
typedef GpioOutputD0 PU8Output;
typedef GpioInputD0  PU8Input;
typedef GpioD1       PU7;
typedef GpioOutputD1 PU7Output;
typedef GpioInputD1  PU7Input;
typedef GpioD2       PU6;
typedef GpioOutputD2 PU6Output;
typedef GpioInputD2  PU6Input;
typedef GpioD3       PU5;
typedef GpioOutputD3 PU5Output;
typedef GpioInputD3  PU5Input;
typedef GpioD4       PU4;
typedef GpioOutputD4 PU4Output;
typedef GpioInputD4  PU4Input;
typedef GpioD5       PU3;
typedef GpioOutputD5 PU3Output;
typedef GpioInputD5  PU3Input;
typedef GpioD6       PU2;
typedef GpioOutputD6 PU2Output;
typedef GpioInputD6  PU2Input;
typedef GpioD7       PU1;
typedef GpioOutputD7 PU1Output;
typedef GpioInputD7  PU1Input;

// Special Names
typedef PR6Output StatusLed;

#endif	// XPCC_MCHCK_HPP
