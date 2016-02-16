// coding: utf-8
/* Copyright (c) 2016, strongly-typed
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_DS3231_HPP
#	error "Don't include this file directly, use 'ds3231.hpp' instead!"
#endif

// ----------------------------------------------------------------------------
template < typename I2cMaster >
xpcc::Ds3231<I2cMaster>::Ds3231(Data &data, uint8_t address)
:	I2cDevice<I2cMaster,2>(address), data(data),
	i2cBuffer{0,0,0,0}, logicBuffer{Register(0)}
{
}

// MARK: - i2cTasks
// MARK: ping
// template < class I2cMaster >
// xpcc::ResumableResult<bool>
// xpcc::Ds3231<I2cMaster>::ping()
// {
// 	RF_BEGIN();
//
// 	if (not RF_CALL(read(Register::SECONDS, i2cBuffer[2])))
// 		RF_RETURN(false);
//
// 	RF_END_RETURN(true);
// }

// template < typename I2cMaster >
// xpcc::ResumableResult<bool>
// xpcc::Ds3231<I2cMaster>::initialize()
// {
// 	using namespace vl6180_private;
//
// 	RF_BEGIN();
//
// 	// logicBuffer[0] => success of config
// 	logicBuffer.byte[0] = true;
// 	// logicBuffer[1] => index of array
// 	logicBuffer.byte[1] = 0;
//
// 	// write the configuration
// 	for (; logicBuffer.byte[1] < 40; logicBuffer.byte[1]++)
// 	{
// 		i2cBuffer[2] = configuration[logicBuffer.byte[1]].value;
// 		logicBuffer.byte[0] &= RF_CALL( write(Register(configuration[logicBuffer.byte[1]].reg), i2cBuffer[2]) );
//
// 		// prematurely abort if something failed
// 		if (!logicBuffer.byte[0]) RF_RETURN(false);
// 	}
//
// 	RF_END_RETURN(logicBuffer.byte[0]);
// }
//
// template < typename I2cMaster >
// xpcc::ResumableResult<bool>
// xpcc::Vl6180<I2cMaster>::readSensor(bool isDistance)
// {
// 	// for this relatively complicated sequence, see the datasheet page 17
// 	RF_BEGIN();
//
// 	// Write 0x01 to the SYSRANGE__START or SYSALS__START register.
// 	// The start bit auto-clears after completion.
// 	logicBuffer.reg = isDistance ? Register::SYSRANGE__START : Register::SYSALS__START;
// 	if ( RF_CALL(write(logicBuffer.reg, uint8_t(Start::StartStop))) )
// 	{
// 		// Measurement will take 7.5ms + convergence time (< ~10ms) for ranging
// 		// or the analog integration time for ALS
// 		timeout.restart(isDistance ? 10 : data.time);
// 		RF_WAIT_UNTIL(timeout.isExpired());
//
// 		// When the measurement is completed, the interrupt source of ALS or range
// 		// in RESULT__INTERRUPT_STATUS_GPIO will set to New Sample Ready.
// 		logicBuffer.byte[0] = 3;	// 3ms initial wait
// 		while(true)
// 		{
// 			// read the byte
// 			if ( !RF_CALL(read(Register::RESULT__INTERRUPT_STATUS_GPIO, i2cBuffer[2])) )
// 				RF_RETURN(false);
//
// 			// break if the correct interrupt source is set
// 			{
// 				InterruptSource source;
// 				if (isDistance) source = RangeInterruptStatus_t::get(InterruptStatus_t(i2cBuffer[2]));
// 				else source = ALS_InterruptStatus_t::get(InterruptStatus_t(i2cBuffer[2]));
//
// 				if (source == InterruptSource::NewSampleReady)
// 					break;
// 			}
//
// 			// otherwise wait 2ms longer on every try
// 			timeout.restart(logicBuffer.byte[0]);
// 			logicBuffer.byte[0] += 2;
//
// 			// 168ms timeout
// 			if (logicBuffer.byte[0] > 25)
// 				RF_RETURN(false);
//
// 			RF_WAIT_UNTIL(timeout.isExpired());
// 		}
//
// 		// The range result is read from RESULT__RANGE_VAL or RESULT__ALS_VAL.
// 		logicBuffer.reg = isDistance ? Register::RESULT__RANGE_VAL : Register::RESULT__ALS_VAL;
// 		if ( RF_CALL(read(logicBuffer.reg, i2cBuffer+2, isDistance ? 1 : 2)) )
// 		{
// 			if (isDistance)
// 				data.data[0] = i2cBuffer[2];
// 			else {
// 				data.data[1] = i2cBuffer[2];
// 				data.data[2] = i2cBuffer[3];
// 			}
//
// 			// Interrupt status flags are cleared by writing a 0x07 to SYSTEM__INTERRUPT_CLEAR.
// 			if ( RF_CALL(write(Register::SYSTEM__INTERRUPT_CLEAR, (InterruptClear::Range | InterruptClear::ALS | InterruptClear::Error).value)) )
// 			{
// 				// Bit 0 of RESULT__RANGE_STATUS or RESULT__ALS_STATUS indicates when either sensor is ready for the next operation.
// 				logicBuffer.reg = isDistance ? Register::RESULT__RANGE_STATUS : Register::RESULT__ALS_STATUS;
// 				i2cBuffer[3] = 0;
// 				while(true)
// 				{
// 					// read the byte
// 					if ( !RF_CALL(read(logicBuffer.reg, i2cBuffer[2])) )
// 						RF_RETURN(false);
//
// 					// Error codes are indicated in bits [7:4] of the status registers
// 					if ( i2cBuffer[2] & uint8_t(RangeStatus::DeviceReady) )
// 					{
// 						if (isDistance) rangeError = RangeErrorCode_t::get(RangeStatus_t(i2cBuffer[2]));
// 						else alsError = ALS_ErrorCode_t::get(ALS_Status_t(i2cBuffer[2]));
// 						break;
// 					}
//
// 					// otherwise wait 4ms and try again
// 					timeout.restart(4);
// 					RF_WAIT_UNTIL(timeout.isExpired());
//
// 					if (i2cBuffer[3]++ > 15)
// 						RF_RETURN(false);
// 				}
//
// 				// the sequence was executed successfully
// 				RF_RETURN(true);
// 			}
// 		}
// 	}
//
// 	RF_END_RETURN(false);
// }
//
// template < typename I2cMaster >
// xpcc::ResumableResult<bool>
// xpcc::Vl6180<I2cMaster>::setDeviceAddress(uint8_t address)
// {
// 	RF_BEGIN();
//
// 	if ( RF_CALL(write(Register::I2C_SLAVE__DEVICE_ADDRESS, (address & 0x7f))) )
// 	{
// 		this->setAddress(address);
// 		RF_RETURN(true);
// 	}
//
// 	RF_END_RETURN(false);
// }
//
// template < typename I2cMaster >
// xpcc::ResumableResult<bool>
// xpcc::Vl6180<I2cMaster>::setGain(AnalogGain gain)
// {
// 	RF_BEGIN();
//
// 	if ( RF_CALL(write(Register::SYSALS__ANALOGUE_GAIN, uint8_t(gain))) )
// 	{
// 		data.gain = uint8_t(gain);
// 		RF_RETURN(true);
// 	}
//
// 	RF_END_RETURN(false);
// }
//
// template < typename I2cMaster >
// xpcc::ResumableResult<bool>
// xpcc::Vl6180<I2cMaster>::setIntegrationTime(uint16_t time)
// {
// 	RF_BEGIN();
//
// 	// 0 is 1ms, we need to substract 1 UNLESS the time is 0!
// 	if (time > 0) time -= 1;
// 	if (time > 0x1ff) time = 0x1ff;
// 	i2cBuffer[2] = time >> 8;
// 	i2cBuffer[3] = time;
//
// 	if ( RF_CALL(write(Register::SYSALS__INTEGRATION_PERIOD, i2cBuffer[2], 2)) )
// 	{
// 		// data.time must never be zero!
// 		data.time = time + 1;
// 		RF_RETURN(true);
// 	}
//
// 	RF_END_RETURN(false);
// }
//
// // ----------------------------------------------------------------------------
// // MARK: update register
// template < typename I2cMaster >
// xpcc::ResumableResult<bool>
// xpcc::Vl6180<I2cMaster>::updateControlRegister(Register reg, Control_t setMask, Control_t clearMask)
// {
// 	RF_BEGIN();
//
// 	if (clearMask.value != 0xff)
// 		if (!RF_CALL(read(reg, i2cBuffer[2])))
// 			RF_RETURN(false);
//
// 	i2cBuffer[2] = (i2cBuffer[2] & ~clearMask.value) | setMask.value;
//
// 	RF_END_RETURN_CALL(write(reg, i2cBuffer[2]));
// }
//
// // MARK: write multilength register
// template < class I2cMaster >
// xpcc::ResumableResult<bool>
// xpcc::Vl6180<I2cMaster>::write(Register reg, uint8_t value, uint8_t length)
// {
// 	RF_BEGIN();
//
// 	i2cBuffer[0] = uint16_t(reg) >> 8;
// 	i2cBuffer[1] = uint8_t(reg);
// 	i2cBuffer[2] = value;
//
// 	this->transaction.configureWrite(i2cBuffer, length+2);
//
// 	RF_END_RETURN_CALL( this->runTransaction() );
// }
//
// MARK: read multilength register
template < class I2cMaster >
xpcc::ResumableResult<bool>
xpcc::Ds3231<I2cMaster>::read(Register reg, uint8_t *buffer, uint8_t length)
{
	RF_BEGIN();

	i2cBuffer[0] = uint16_t(reg) >> 8;
	i2cBuffer[1] = uint8_t(reg);

	this->transaction.configureWriteRead(i2cBuffer, 2, buffer, length);

	RF_END_RETURN_CALL( this->runTransaction() );
}
