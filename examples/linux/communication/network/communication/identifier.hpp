// ----------------------------------------------------------------------------
/*
 * WARNING: This file is generated automatically, do not edit!
 * Please modify the corresponding XML file instead.
 */
// ----------------------------------------------------------------------------

#ifndef	ROBOT__IDENTIFIER_HPP
#define	ROBOT__IDENTIFIER_HPP

namespace robot
{
	namespace domain
	{
		enum Identifier
		{
		};
				
		inline const char* 
		enumToString(Identifier e)
		{
			switch (e)
			{
				default: return "__UNKNOWN__";
			}
		}
	}
	
	namespace component
	{
		enum Identifier
		{
			SENDER = 1,
			RECEIVER = 2,
		};
				
		inline const char* 
		enumToString(Identifier e)
		{
			switch (e)
			{
				case SENDER: return "SENDER";
				case RECEIVER: return "RECEIVER";
				default: return "__UNKNOWN__";
			}
		}
	}
	
	namespace action
	{
		enum Identifier
		{
			SET_POSITION = 1,
			GET_POSITION = 2,
		};
				
		inline const char* 
		enumToString(Identifier e)
		{
			switch (e)
			{
				case SET_POSITION: return "SET_POSITION";
				case GET_POSITION: return "GET_POSITION";
				default: return "__UNKNOWN__";
			}
		}
	}
		
	namespace event
	{
		enum Identifier
		{
		};
		
		inline const char* 
		enumToString(Identifier e)
		{
			switch (e)
			{
				default: return "__UNKNOWN__";
			}
		}
	}
}

#endif	// ROBOT__IDENTIFIER_HPP
