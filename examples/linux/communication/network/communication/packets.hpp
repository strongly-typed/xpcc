// ----------------------------------------------------------------------------
/*
 * WARNING: This file is generated automatically, do not edit!
 * Please modify the corresponding XML file instead.
 */
// ----------------------------------------------------------------------------

#ifndef	ROBOT__PACKETS_HPP
#define	ROBOT__PACKETS_HPP

#include <stdint.h>

namespace robot
{
	namespace packet
	{
		/** On x86 systems the bool-type has 4 bytes and on AVRs it has 1 byte.
		 * This type is defined to make it posible to send bools from
		 * everywhere. */
		typedef uint8_t Bool;
	
		/** Position of the Robot on the Playground. */
		struct Position
		{
			Position();
			
			Position(int16_t x, int16_t y);
			
			int16_t x;
			int16_t y;
		} __attribute__((packed));
	} // namespace packet
} // namespace robot

#endif	// ROBOT__PACKETS_HPP
