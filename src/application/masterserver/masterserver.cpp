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
#include "augs/readwrite/to_bytes.h"
#include "application/masterserver/netcode_ping_request.h"

std::string ToString(const netcode_address_t&);

#if PLATFORM_UNIX
extern volatile std::sig_atomic_t signal_status;
#endif

#define LOG_MASTERSERVER 1

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

	auto udp_command_sockets = std::vector<netcode_socket_raii>();

	const auto num_sockets = settings.num_udp_command_ports;

	LOG("Creating %x masterserver sockets for UDP commands.", num_sockets);

	for (int i = 0; i < num_sockets; ++i) {
		const auto new_port = settings.first_udp_command_port + i;

		if (const auto new_local_address = to_netcode_addr(settings.ip, new_port)) {
			udp_command_sockets.emplace_back(*new_local_address);
		}
		else {
			LOG("There was a problem binding masterserver to %x:%x! Quitting.", settings.ip, new_port);
			return;
		}

		LOG("Created masterserver socket at: %x", ::ToString(udp_command_sockets.back().socket.address));
	}

	std::unordered_map<netcode_address_t, masterserver_client> server_list;

	std::vector<std::byte> serialized_list;
	std::shared_mutex serialized_list_mutex;

	httplib::Server http;

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
				const auto address = augs::read_bytes<netcode_address_t>(source);

				masterserver_client entry;
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

	LOG("Hosting a server list at port: %x (HTTP)", settings.server_list_port);

	auto listening_thread = std::thread([&http, in_settings=settings]() {
		http.listen(in_settings.ip.c_str(), in_settings.server_list_port);
		LOG("The HTTP listening thread has quit.");
	});

	uint8_t packet_buffer[NETCODE_MAX_PACKET_BYTES];

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

		auto process_socket_messages = [&](auto& socket) {
			netcode_address_t from;
			const auto packet_bytes = netcode_socket_receive_packet(&socket, &from, packet_buffer, NETCODE_MAX_PACKET_BYTES);

			if (packet_bytes < 1) {
				return;
			}

			MSR_LOG("Received packet bytes: %x", packet_bytes);

			try {
				auto send_back = [&](const auto& typed_response) {
					auto bytes = augs::to_bytes(masterserver_response(typed_response));
					netcode_socket_send_packet(&socket, &from, bytes.data(), bytes.size());
				};

				auto handle = [&](const auto& typed_request) {
					using R = remove_cref<decltype(typed_request)>;
					namespace IN = masterserver_in;
					namespace OUT = masterserver_out;

					if constexpr(std::is_same_v<R, IN::goodbye>) {
						if (const auto entry = mapped_or_nullptr(server_list, from)) {
							LOG("The server at %x (%x) has sent a goodbye.", ::ToString(from), entry->last_heartbeat.server_name);
							remove_from_list(from);
						}
					}
					else if constexpr(std::is_same_v<R, IN::heartbeat>) {
						auto it = server_list.try_emplace(from);

						const bool is_new_server = it.second;
						auto& server_entry = (*it.first).second;

						const auto heartbeat_before = server_entry.last_heartbeat;
						server_entry.last_heartbeat = typed_request;
						server_entry.time_of_last_heartbeat = current_time;

						const bool heartbeats_mismatch = heartbeat_before != server_entry.last_heartbeat;

						MSR_LOG_NVPS(is_new_server, heartbeats_mismatch);

						if (is_new_server || heartbeats_mismatch) {
							reserialize_list();
						}
					}
					else if constexpr(std::is_same_v<R, IN::tell_me_my_address>) {
						OUT::tell_me_my_address response;
						response.address = from;

						MSR_LOG("TELL_ME_MY_ADDRESS arrived from: %x", ::ToString(from));
						send_back(response);
					}
					else if constexpr(std::is_same_v<R, IN::punch_this_server>) {
						auto punched_server = typed_request.address;
						const auto& pingback_address = from;

						const bool should_send_request = [&]() {
							if (const auto entry = mapped_or_nullptr(server_list, punched_server)) {
								MSR_LOG("Found the requested server.");

								const bool is_behind_nat = entry->last_heartbeat.is_behind_nat();

								if (is_behind_nat) {
									MSR_LOG("The requested server is behind NAT. Deciding to send the request.");
									return true;
								}

								MSR_LOG("The requested server is not behind NAT. Ignoring the request.");
								return false;
							}

							MSR_LOG("The requested server was not found.");
							return false;
						}();

						MSR_LOG("A request arrived from %x to punch %x", ::ToString(from), ::ToString(punched_server));

						if (should_send_request || LOG_MASTERSERVER) {
							auto punching_ping_bytes = make_ping_request_message_bytes(
								pingback_address
							);

							netcode_socket_send_packet(&socket, &punched_server, punching_ping_bytes.data(), punching_ping_bytes.size());
						}
					}
				};

				const auto buf = augs::cpointer_to_buffer { 
					reinterpret_cast<const std::byte*>(packet_buffer), 
					static_cast<std::size_t>(packet_bytes) 
				};

				auto ss = augs::cptr_memory_stream(buf);

				const auto request = augs::read_bytes<masterserver_request>(ss);
				std::visit(handle, request);
			}
			catch (...) {
				if (const auto entry = mapped_or_nullptr(server_list, from)) {
					LOG("The server at %x (%x) has sent invalid data.", ::ToString(from), entry->last_heartbeat.server_name);
					remove_from_list(from);
				}
			}
		};

		for (auto& s : udp_command_sockets) {
			process_socket_messages(s.socket);
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

		yojimbo_sleep(settings.sleep_ms / 1000);
	}

	LOG("Stopping the HTTP masterserver.");
	http.stop();
	LOG("Joining the HTTP listening thread.");
	listening_thread.join();

	dump_server_list_to_file();
}
catch (const netcode_socket_raii_error& err) {
	LOG(err.what());
}
