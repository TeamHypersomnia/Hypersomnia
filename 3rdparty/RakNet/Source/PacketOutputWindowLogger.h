/// \file
/// \brief This will write all incoming and outgoing network messages to a file
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_PacketLogger==1

#ifndef __PACKET_OUTPUT_WINDOW_LOGGER_H_
#define __PACKET_OUTPUT_WINDOW_LOGGER_H_

#include "PacketLogger.h"

namespace RakNet
{

/// \ingroup PACKETLOGGER_GROUP
/// \brief Packetlogger that outputs to the output window in the debugger. Windows only.
class RAK_DLL_EXPORT  PacketOutputWindowLogger : public PacketLogger
{
public:
	PacketOutputWindowLogger();
	virtual ~PacketOutputWindowLogger();
	virtual void WriteLog(const char *str);
protected:
};

} // namespace RakNet

#endif

#endif // _RAKNET_SUPPORT_*
