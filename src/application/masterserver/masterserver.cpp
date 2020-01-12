#if PLATFORM_UNIX
#include <csignal>
#endif
#include <shared_mutex>

#include "application/masterserver/masterserver.h"
#include "3rdparty/cpp-httplib/httplib.h"
#include "augs/log.h"

#include "application/config_lua_table.h"
#include "augs/network/netcode_sockets.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/memory_stream.h"
#include "application/masterserver/server_heartbeat.h"
#include "augs/templates/introspection_utils/introspective_equal.h"
#include "augs/templates/thread_templates.h"
#include "augs/misc/time_utils.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/network/resolve_address.h"
#include "augs/readwrite/byte_file.h"

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

struct masterserver_client_meta {
	double appeared_when;

	masterserver_client_meta() {
		appeared_when = augs::date_time::secs_since_epoch();
	}
};

struct masterserver_client {
	double time_of_last_heartbeat;

	masterserver_client_meta meta;
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

bool operator!=(const netcode_address_t& a, const netcode_address_t& b) {
	auto aa = a;
	auto bb = b;
	return 0 == netcode_address_equal(&aa, &bb);
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

void perform_masterserver(const config_lua_table& cfg) try {
	using namespace httplib;

	const auto& settings = cfg.masterserver;
	const auto http_port = 8080;

	auto socket_raii = netcode_socket_raii(to_netcode_addr(settings.ip, settings.port));
	auto& socket = socket_raii.socket;

	LOG("Created masterserver socket at: %x", ::ToString(socket.address));

	std::unordered_map<netcode_address_t, masterserver_client> server_list;

	std::vector<std::byte> serialized_list;
	std::shared_mutex serialized_list_mutex;

	httplib::Server http;

	uint8_t packet_buffer[NETCODE_MAX_PACKET_BYTES];

	const auto masterserver_dump_path = augs::path_type(USER_FILES_DIR) / "masterserver.dump";

	auto reserialize_list = [&]() {
		MSR_LOG("Reserializing the server list.");

		std::lock_guard<std::shared_mutex> lock(serialized_list_mutex);

		serialized_list.clear();

		auto ss = augs::ref_memory_stream(serialized_list);

		for (auto& server : server_list) {
			const auto address = server.first;

			augs::write_bytes(ss, address);
			augs::write_bytes(ss, server.second.meta.appeared_when);
			augs::write_bytes(ss, server.second.last_heartbeat);
		}
	};

	auto dump_server_list_to_file = [&]() {
		const auto n = server_list.size();

		if (n > 0) {
			LOG("Saving %x servers to %x", n, masterserver_dump_path);
			augs::bytes_to_file(std::as_const(serialized_list), masterserver_dump_path);
		}
		else {
			LOG("The server list is empty: deleting the dump file.");
			augs::remove_file(masterserver_dump_path);
		}
	};

	auto load_server_list_from_file = [&]() {
		try {
			auto source = augs::open_binary_input_stream(masterserver_dump_path);

			LOG("%x found.\nLoading the server list from file.", masterserver_dump_path);

			const auto current_time = yojimbo_time();

			while (source.peek() != EOF) {
				netcode_address_t address;
				masterserver_client entry;

				augs::read_bytes(source, address);
				augs::read_bytes(source, entry.meta.appeared_when);
				augs::read_bytes(source, entry.last_heartbeat);

				entry.time_of_last_heartbeat = current_time;

				server_list.try_emplace(address, std::move(entry));
			}

			reserialize_list();
		}
		catch (const augs::file_open_error& err) {
			LOG("Could not load the server list file: %x.\nStarting from an empty server list. Details:\n%x", masterserver_dump_path, err.what());

			server_list.clear();
		}
		catch (const augs::stream_read_error& err) {
			LOG("Failed to read the server list from file: %x.\nStarting from an empty server list. Details:\n%x", masterserver_dump_path, err.what());

			server_list.clear();
		}
	};

	load_server_list_from_file();

	auto make_list_streamer_lambda = [&]() {
		return [data=serialized_list](uint64_t offset, uint64_t length, DataSink sink) {
			sink(reinterpret_cast<const char*>(&data[offset]), length);
		};
	};

	auto remove_from_list = [&](const auto& by_external_addr) {
		server_list.erase(by_external_addr);
		reserialize_list();
	};

	auto define_http_server = [&]() {
		http.Get("/server_list_binary", [&](const Request&, Response& res) {
			std::shared_lock<std::shared_mutex> lock(serialized_list_mutex);

			if (serialized_list.size() > 0) {
				MSR_LOG("List request arrived. Sending list of size: %x", serialized_list.size());

				res.set_content_provider(
					serialized_list.size(),
					make_list_streamer_lambda()
				);
			}
		});
	};

	define_http_server();

	LOG("Hosting a HTTP masterserver at port: %x", http_port);

	auto listening_thread = std::thread([&http]() {
		http.listen("0.0.0.0", http_port);
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
		const auto packet_bytes = netcode_socket_receive_packet(&socket, &from, packet_buffer, NETCODE_MAX_PACKET_BYTES);

		if (packet_bytes == 1) {
			if (const auto entry = mapped_or_nullptr(server_list, from)) {
				LOG("The server at %x (%x) has sent a goodbye.", ::ToString(from), entry->last_heartbeat.server_name);
				remove_from_list(from);
			}
		}
		else if (packet_bytes > 0) {
			MSR_LOG("Received packet bytes: %x", packet_bytes);

			const auto buf = augs::cpointer_to_buffer{ reinterpret_cast<const std::byte*>(packet_buffer), static_cast<std::size_t>(packet_bytes) };
			auto ss = augs::cptr_memory_stream(buf);

			try {
				masterserver_udp_command command;
				augs::read_bytes(ss, command);

				if (command == masterserver_udp_command::HEARTBEAT) {
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
				else if (command == masterserver_udp_command::NAT_PUNCH_REQUEST) {
					netcode_address_t target_server;
					augs::read_bytes(ss, target_server);

					std::byte out_buf[sizeof(masterserver_udp_command) + sizeof(netcode_address_t) + sizeof(uint64_t)];

					auto buf = augs::pointer_to_buffer{out_buf, sizeof(out_buf)};
					auto out = augs::ptr_memory_stream(buf);

					const auto sequence_dummy = uint64_t(-1);

					augs::write_bytes(out, uint8_t(NETCODE_PING_REQUEST_PACKET));
					augs::write_bytes(out, sequence_dummy);
					augs::write_bytes(out, from);

					if (const auto entry = mapped_or_nullptr(server_list, target_server)) {
						const bool is_behind_nat = entry->last_heartbeat.internal_network_address != target_server;

						if (is_behind_nat) {
							MSR_LOG("Sending NAT open request from %x to %x", ::ToString(from), ::ToString(target_server));
							netcode_socket_send_packet(&socket, &target_server, out_buf, sizeof(out_buf));
						}
					}
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

		auto process_entry_logic = [&](auto& server_entry) {
			(void)server_entry;
		};

		auto erase_if_dead = [&](auto& server_entry) {
			const bool timed_out = current_time - server_entry.second.time_of_last_heartbeat >= timeout_secs;

			if (timed_out) {
				LOG("The server at %x (%x) has timed out.", ::ToString(server_entry.first), server_entry.second.last_heartbeat.server_name);
			}
			else {
				process_entry_logic(server_entry);
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

	netcode_socket_destroy(&socket);

	dump_server_list_to_file();
}
catch (const netcode_socket_raii_error& err) {
	LOG(err.what());
}
