/// \file GetTime.h
/// \brief Returns the value from QueryPerformanceCounter.  This is the function RakNet uses to represent time. This time won't match the time returned by GetTimeCount(). See http://www.jenkinssoftware.com/forum/index.php?topic=2798.0
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#ifndef __GET_TIME_H
#define __GET_TIME_H

#include "Export.h"
#include "RakNetTime.h" // For RakNet::TimeMS

namespace RakNet
{
	/// Same as GetTimeMS
	/// Holds the time in either a 32 or 64 bit variable, depending on __GET_TIME_64BIT
	RakNet::Time RAK_DLL_EXPORT GetTime( void );

	/// Return the time as 32 bit
	/// \note The maximum delta between returned calls is 1 second - however, RakNet calls this constantly anyway. See NormalizeTime() in the cpp.
	RakNet::TimeMS RAK_DLL_EXPORT GetTimeMS( void );
	
	/// Return the time as 64 bit
	/// \note The maximum delta between returned calls is 1 second - however, RakNet calls this constantly anyway. See NormalizeTime() in the cpp.
	RakNet::TimeUS RAK_DLL_EXPORT GetTimeUS( void );

	/// a > b?
	extern RAK_DLL_EXPORT bool GreaterThan(RakNet::Time a, RakNet::Time b);
	/// a < b?
	extern RAK_DLL_EXPORT bool LessThan(RakNet::Time a, RakNet::Time b);
}

#endif
