#if PLATFORM_UNIX
#include <csignal>
#endif

#include "application/masterserver/masterserver.h"
#include "3rdparty/cpp-httplib/httplib.h"
#include "augs/log.h"

#include "augs/network/netcode_sockets.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/memory_stream.h"

#if PLATFORM_UNIX
extern volatile std::sig_atomic_t signal_status;
#endif

struct masterserver_client {
	double time_of_last_heartbeat;
	server_heartbeat last_heartbeat;
};

bool operator==(const netcode_address_t& a, const netcode_address_t& b) {
	auto aa = a;
	auto bb = b;
	return 1 == netcode_address_equal(&aa, &bb);
}

namespace std {
	template <>
	struct hash<netcode_address_t> {
		size_t operator()(const netcode_address_t& k) const {
			if (k.type == NETCODE_ADDRESS_IPV4) {
				uint32_t ip;
				std::memcpy(&ip, &k.data.ipv4, sizeof(ip));
				return augs::hash_multiple(ip, k.port);
			}

			uint64_t ip_parts[2];
			std::memcpy(&ip_parts, &k.data.ipv6, sizeof(ip_parts));
			return augs::hash_multiple(ip_parts[0], ip_parts[1], k.port);
		}
	};
}

double yojimbo_time();
void yojimbo_sleep(double);

void perform_masterserver(const config_lua_table& cfg) {
	const auto& settings = cfg.masterserver;

	char buffer[NETCODE_MAX_PACKET_BYTES];

	struct netcode_address_t addr;
	struct netcode_socket_t socket;

	std::unordered_map<netcode_address_t, masterserver_client> clients;

	netcode_parse_address(settings.ip.c_str(), &addr);
	addr.port = settings.port;

	const auto bufsize = 4 * 1024 * 1024;
	netcode_socket_create(&socket, &addr, bufsize, bufsize);

	while (true) {
#if PLATFORM_UNIX
		if (signal_status != 0) {
			const auto sig = signal_status;

			LOG("%x received.", strsignal(sig));

			if(
				sig == SIGINT
				|| sig == SIGSTOP
				|| sig == SIGTERM
			) {
				LOG("Gracefully shutting down.");
				break;
			}
		}
#endif

		const auto current_time = yojimbo_time();

		struct netcode_address_t from;
		const auto packet_bytes = netcode_socket_receive_packet(&socket, &from, buffer, NETCODE_MAX_PACKET_BYTES);

		if (packet_bytes > 0) {
			const auto buf = augs::pointer_to_buffer{ reinterpret_cast<std::byte*>(buffer), static_cast<std::size_t>(packet_bytes) };
			auto ss = augs::cptr_memory_stream(buf);

			try {
				auto& client = clients[from];
				augs::read_bytes(ss, client.last_heartbeat);

				client.time_of_last_heartbeat = current_time;
			}
			catch (...) {

			}
		}

		const auto timeout_secs = settings.server_entry_timeout_secs;

		auto erase_if_dead = [&](auto& client) {
			return current_time - client.second.time_of_last_heartbeat >= timeout_secs;
		};

		erase_if(clients, erase_if_dead);

		yojimbo_sleep(16.0 / 1000);
	}
}
