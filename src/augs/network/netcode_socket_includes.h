#pragma once
#include "augs/network/netcode_sockets.h"

#if NETCODE_PLATFORM == NETCODE_PLATFORM_WINDOWS

    #define NOMINMAX
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <ws2ipdef.h>
    #include <iphlpapi.h>
    #pragma comment( lib, "WS2_32.lib" )
    #pragma comment( lib, "IPHLPAPI.lib" )

    #ifdef SetPort
    #undef SetPort
    #endif // #ifdef SetPort

    #include <iphlpapi.h>
    #pragma comment( lib, "IPHLPAPI.lib" )
    
#elif NETCODE_PLATFORM == NETCODE_PLATFORM_MAC || NETCODE_PLATFORM == NETCODE_PLATFORM_UNIX

    #include <netdb.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>

#else

    #error netcode - unknown platform!

#endif
