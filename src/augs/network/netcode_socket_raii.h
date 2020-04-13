#pragma once
#include "augs/ensure_rel.h"
#include "augs/templates/exception_templates.h"
#include "augs/network/netcode_sockets.h"

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
