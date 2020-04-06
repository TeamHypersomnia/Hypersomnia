#pragma once
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "augs/network/port_type.h"
#include "augs/log.h"

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

void netcode_socket_destroy( struct netcode_socket_t * socket );
int netcode_socket_create( struct netcode_socket_t * s, struct netcode_address_t * address, int send_buffer_size, int receive_buffer_size );
int netcode_socket_receive_packet( struct netcode_socket_t * socket, struct netcode_address_t * from, void * packet_data, int max_packet_size );
void netcode_socket_send_packet( struct netcode_socket_t * socket, struct netcode_address_t * to, void * packet_data, int packet_bytes );

#include "augs/ensure_rel.h"
#include "augs/templates/exception_templates.h"

struct netcode_socket_raii_error : error_with_typesafe_sprintf {
	using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
};

struct netcode_socket_raii {
	netcode_socket_t socket;

private:
	bool initialized = false;

	void bind_to(netcode_address_t address) {
		const auto buf_size = 4 * 1024 * 1024;

		LOG("netcode_socket_raii ctor");

		if (const auto result = netcode_socket_create(&socket, &address, buf_size, buf_size); result != NETCODE_SOCKET_ERROR_NONE) {
			throw netcode_socket_raii_error("netcode_socket_create failed with code: %x", result);
		}

		initialized = true;
	}

	void reset() {
		initialized = false;
		socket = {};
	}

	void destroy() {
		if (initialized) {
			LOG("netcode_socket_raii dtor");
			netcode_socket_destroy(&socket);

			reset();
		}
	}
public:

	netcode_socket_raii(const netcode_socket_raii&) = delete;
	netcode_socket_raii& operator=(const netcode_socket_raii&) = delete;

	netcode_socket_raii(netcode_socket_raii&& b) 
		: socket(b.socket), initialized(b.initialized) 
	{
		b.reset();
	}

	netcode_socket_raii& operator=(netcode_socket_raii&& b)  {
		destroy();

		socket = b.socket;
		initialized = b.initialized;

		b.reset();

		return *this;
	}

	netcode_socket_raii(netcode_address_t address) {
		bind_to(address);
	}

	netcode_socket_raii(port_type port) {
		netcode_address_t address;
		const auto result = netcode_parse_address("0.0.0.0", &address);
		ensure_eq(NETCODE_OK, result);

		address.port = port;

		bind_to(address);
	}

	netcode_socket_raii() {
		netcode_address_t address;
		const auto result = netcode_parse_address("0.0.0.0", &address);
		ensure_eq(NETCODE_OK, result);
		bind_to(address);
	}

	~netcode_socket_raii() {
		destroy();
	}
};
