/// \file
/// \brief This will write all incoming and outgoing network messages to a file
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_PacketLogger==1

#ifndef __PACKET_FILE_LOGGER_H_
#define __PACKET_FILE_LOGGER_H_

#include "PacketLogger.h"
#include <stdio.h>

namespace RakNet
{

/// \ingroup PACKETLOGGER_GROUP
/// \brief Packetlogger that outputs to a file
class RAK_DLL_EXPORT  PacketFileLogger : public PacketLogger
{
public:
	PacketFileLogger();
	virtual ~PacketFileLogger();
	void StartLog(const char *filenamePrefix);
	virtual void WriteLog(const char *str);
protected:
	FILE *packetLogFile;
};

} // namespace RakNet

#endif

#endif // _RAKNET_SUPPORT_*
