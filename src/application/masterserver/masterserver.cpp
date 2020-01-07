#if PLATFORM_UNIX
#include <csignal>
#endif
#include <shared_mutex>

#include "application/masterserver/masterserver.h"
#include "3rdparty/cpp-httplib/httplib.h"
#include "augs/log.h"

#include "augs/network/netcode_sockets.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/memory_stream.h"
#include "application/masterserver/server_heartbeat.h"
#include "augs/templates/introspection_utils/introspective_equal.h"
#include "augs/templates/thread_templates.h"

std::string ToString(const netcode_address_t&);

#if PLATFORM_UNIX
extern volatile std::sig_atomic_t signal_status;
#endif

#define LOG_MASTERSERVER !NDEBUG

template <class... Args>
void MSR_LOG(Args&&... args) {
#if LOG_MASTERSERVER
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_MASTERSERVER
#define MSR_LOG_NVPS LOG_NVPS
#else
#define MSR_LOG_NVPS MSR_LOG
#endif

struct masterserver_client {
	double time_of_last_heartbeat;
	server_heartbeat last_heartbeat;
};

bool operator!=(const server_heartbeat& a, const server_heartbeat& b) {
	return !augs::introspective_equal(a, b);
}

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

	struct netcode_socket_t socket;

	{
		struct netcode_address_t addr;
		netcode_parse_address(settings.ip.c_str(), &addr);
		addr.port = settings.port;

		const auto bufsize = 4 * 1024 * 1024;

		if (netcode_socket_create(&socket, &addr, bufsize, bufsize) != NETCODE_SOCKET_ERROR_NONE) {
			LOG("netcode_socket_create failed. Returning.");
			return;
		}
	}

	char buffer[NETCODE_MAX_PACKET_BYTES];

	std::unordered_map<netcode_address_t, masterserver_client> server_list;

	std::vector<std::byte> serialized_list;
	std::shared_mutex serialized_list_mutex;

	auto reserialize_list = [&]() {
		MSR_LOG("Reserializing the server list.");

		std::lock_guard<std::shared_mutex> lock(serialized_list_mutex);

		serialized_list.clear();

		auto ss = augs::ref_memory_stream(serialized_list);

		for (auto& server : server_list) {
			augs::write_bytes(ss, server.first);
			augs::write_bytes(ss, server.second.last_heartbeat);
		}
	};

	httplib::Server http;

	using namespace httplib;

	auto make_list_streamer_lambda = [&]() {
    	std::shared_lock<std::shared_mutex> lock(serialized_list_mutex);

		return [data=serialized_list](uint64_t offset, uint64_t length, DataSink sink) {
			sink(reinterpret_cast<const char*>(&data[offset]), length);
		};
	};

	http.Get("/server_list_binary", [&](const Request&, Response& res) {
		if (serialized_list.size() > 0) {
			MSR_LOG("List request arrived. Sending list of size: %x", serialized_list.size());

			res.set_content_provider(
				serialized_list.size(),
				make_list_streamer_lambda()
			);
		}
	});

	const auto http_port = 8080;

	LOG("Hosting a HTTP masterserver at port: %x", http_port);

	auto remove_from_list = [&](const auto& by_external_addr) {
		server_list.erase(by_external_addr);
		reserialize_list();
	};

	auto listening_thread = std::thread([&http]() {
		http.listen("127.0.0.1", http_port);
		LOG("The HTTP listening thread has quit.");
	});

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

		if (packet_bytes == 1) {
			if (const auto entry = mapped_or_nullptr(server_list, from)) {
				LOG("The server at %x (%x) has sent a goodbye.", ::ToString(from), entry->last_heartbeat.server_name);
				remove_from_list(from);
			}
		}
		else if (packet_bytes > 0) {
			MSR_LOG("Received packet bytes: %x", packet_bytes);

			const auto buf = augs::cpointer_to_buffer{ reinterpret_cast<const std::byte*>(buffer), static_cast<std::size_t>(packet_bytes) };
			auto ss = augs::cptr_memory_stream(buf);

			try {
				auto it = server_list.try_emplace(from);

				const bool is_new_server = it.second;
				auto& server_entry = (*it.first).second;

				const auto heartbeat_before = server_entry.last_heartbeat;
				augs::read_bytes(ss, server_entry.last_heartbeat);
				server_entry.last_heartbeat.validate();

				server_entry.time_of_last_heartbeat = current_time;

				const bool heartbeats_mismatch = heartbeat_before != server_entry.last_heartbeat;

				MSR_LOG_NVPS(is_new_server, heartbeats_mismatch);

				if (is_new_server || heartbeats_mismatch) {
					reserialize_list();
				}
			}
			catch (...) {
				if (const auto entry = mapped_or_nullptr(server_list, from)) {
					LOG("The server at %x (%x) has sent invalid data.", ::ToString(from), entry->last_heartbeat.server_name);
					remove_from_list(from);
				}
			}
		}

		const auto timeout_secs = settings.server_entry_timeout_secs;

		auto erase_if_dead = [&](auto& server_entry) {
			const bool timed_out = current_time - server_entry.second.time_of_last_heartbeat >= timeout_secs;

			if (timed_out) {
				LOG("The server at %x (%x) has timed out.", ::ToString(server_entry.first), server_entry.second.last_heartbeat.server_name);
			}

			return timed_out;
		};

		const auto previous_size = server_list.size();
		erase_if(server_list, erase_if_dead);

		if (previous_size != server_list.size()) {
			reserialize_list();
		}

		yojimbo_sleep(16.0 / 1000);
	}

	LOG("Stopping the HTTP masterserver.");
	http.stop();
	LOG("Joining the HTTP listening thread.");
	listening_thread.join();
}
