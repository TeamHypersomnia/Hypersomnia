#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_PacketLogger==1
#include "PacketFileLogger.h"
#include "GetTime.h"

using namespace RakNet;

PacketFileLogger::PacketFileLogger()
{
	packetLogFile=0;
}
PacketFileLogger::~PacketFileLogger()
{
	if (packetLogFile)
	{
		fflush(packetLogFile);
		fclose(packetLogFile);
	}
}
void PacketFileLogger::StartLog(const char *filenamePrefix)
{
	// Open file for writing
	char filename[256];
	if (filenamePrefix)
		sprintf(filename, "%s_%i.csv", filenamePrefix, (int) RakNet::GetTimeMS());
	else
		sprintf(filename, "PacketLog_%i.csv", (int) RakNet::GetTimeMS());
	packetLogFile = fopen(filename, "wt");
	LogHeader();
	if (packetLogFile)
	{
		fflush(packetLogFile);
	}
}

void PacketFileLogger::WriteLog(const char *str)
{
	if (packetLogFile)
	{
		fprintf(packetLogFile, "%s\n", str);
		fflush(packetLogFile);
	}
}

#endif // _RAKNET_SUPPORT_*
