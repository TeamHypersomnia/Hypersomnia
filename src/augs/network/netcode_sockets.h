#pragma once
#include "3rdparty/yojimbo/netcode.io/netcode.h"

#define NETCODE_SOCKET_ERROR_NONE                               0
#define NETCODE_PLATFORM_WINDOWS    1
#define NETCODE_PLATFORM_MAC        2
#define NETCODE_PLATFORM_UNIX       3
#define NETCODE_MAX_PACKET_BYTES 1300

#if defined(_WIN32)
#define NETCODE_PLATFORM NETCODE_PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define NETCODE_PLATFORM NETCODE_PLATFORM_MAC
#else
#define NETCODE_PLATFORM NETCODE_PLATFORM_UNIX
#endif

#if NETCODE_PLATFORM == NETCODE_PLATFORM_WINDOWS
typedef uint64_t netcode_socket_handle_t;
#else // #if NETCODE_PLATFORM == NETCODE_PLATFORM_WINDOWS
typedef int netcode_socket_handle_t;
#endif // #if NETCODE_PLATFORM == NETCODe_PLATFORM_WINDOWS

struct netcode_socket_t
{
	struct netcode_address_t address;
	netcode_socket_handle_t handle;
};

int netcode_socket_create( struct netcode_socket_t * s, struct netcode_address_t * address, int send_buffer_size, int receive_buffer_size );
int netcode_socket_receive_packet( struct netcode_socket_t * socket, struct netcode_address_t * from, void * packet_data, int max_packet_size );
void netcode_socket_send_packet( struct netcode_socket_t * socket, struct netcode_address_t * to, void * packet_data, int packet_bytes );
