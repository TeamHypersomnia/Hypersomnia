/// \file
/// \brief Contains TelnetTransport , used to supports the telnet transport protocol.  Insecure
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_TelnetTransport==1 && _RAKNET_SUPPORT_TCPInterface==1

#ifndef __TELNET_TRANSPORT
#define __TELNET_TRANSPORT

#include "TransportInterface.h"
#include "DS_List.h"
#include "Export.h"

namespace RakNet
{
/// Forward declarations
class TCPInterface;
struct TelnetClient;

/// \brief Use TelnetTransport to easily allow windows telnet to connect to your ConsoleServer
/// \details To run Windows telnet, go to your start menu, click run, and in the edit box type "telnet <IP>" where <IP> is the ip address.<BR>
/// of your ConsoleServer (most likely the same IP as your game).<BR>
/// This implementation always echos commands.
class RAK_DLL_EXPORT TelnetTransport : public TransportInterface
{
public:
	// GetInstance() and DestroyInstance(instance*)
	STATIC_FACTORY_DECLARATIONS(TelnetTransport)

	TelnetTransport();
	virtual ~TelnetTransport();
	bool Start(unsigned short port, bool serverMode);
	void Stop(void);
	void Send( SystemAddress systemAddress, const char *data, ... );
	void CloseConnection( SystemAddress systemAddress );
	Packet* Receive( void );
	void DeallocatePacket( Packet *packet );
	SystemAddress HasNewIncomingConnection(void);
	SystemAddress HasLostConnection(void);
	CommandParserInterface* GetCommandParser(void);
	void SetSendSuffix(const char *suffix);
	void SetSendPrefix(const char *prefix);
protected:

	struct TelnetClient
	{
		SystemAddress systemAddress;
		char textInput[REMOTE_MAX_TEXT_INPUT];
		char lastSentTextInput[REMOTE_MAX_TEXT_INPUT];
		unsigned cursorPosition;
	};

	TCPInterface *tcpInterface;
	void AutoAllocate(void);
	bool ReassembleLine(TelnetTransport::TelnetClient* telnetClient, unsigned char c);

	// Crap this sucks but because windows telnet won't send line at a time, I have to reconstruct the lines at the server per player
	DataStructures::List<TelnetClient*> remoteClients;

	char *sendSuffix, *sendPrefix;

};

} // namespace RakNet

#endif

#endif // _RAKNET_SUPPORT_*
