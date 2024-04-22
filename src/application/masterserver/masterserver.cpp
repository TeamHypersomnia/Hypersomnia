#if PLATFORM_UNIX
#include <csignal>
#endif
#include <shared_mutex>
#include "application/masterserver/masterserver.h"
#include "3rdparty/include_httplib.h"
#include "augs/log.h"

#include "application/config_lua_table.h"
#include "augs/network/netcode_sockets.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/memory_stream.h"
#include "application/masterserver/server_heartbeat.h"
#include "augs/templates/thread_templates.h"
#include "augs/misc/date_time.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/network/netcode_socket_raii.h"
#include "application/network/resolve_address.h"
#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/to_bytes.h"
#include "application/masterserver/masterserver_requests.h"
#include "application/masterserver/netcode_address_hash.h"
#include "augs/string/parse_url.h"
#include "application/detail_file_paths.h"
#include "application/setups/server/webhooks.h"
#include "application/masterserver/server_list_entry_json.h"
#include "3rdparty/rapidjson/include/rapidjson/prettywriter.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/network/netcode_utils.h"
#include "augs/misc/httplib_utils.h"

#include "application/masterserver/masterserver_client.h"
#include "application/masterserver/signalling_server.h"
#include "application/network/network_adapters.h"
#include "augs/string/typesafe_sscanf.h"

std::string to_lowercase(std::string s);
std::string ToString(const netcode_address_t&);

#if PLATFORM_UNIX
extern std::atomic<int> signal_status;
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


static void set_cors(httplib::Response& res) {
	res.set_header("Access-Control-Allow-Origin", "*"); // Allows any domain
	res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS"); // Allowed methods
	res.set_header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization");
};

using ip_to_host = std::unordered_map<
	std::string,
	std::string
>;

std::string to_string_no_port(netcode_address_t n) {
	n.port = 0;
	return ::ToString(n);
}

static std::string find_official_url(
	const ip_to_host& map,
	const netcode_address_t& ip
) {
	if (const auto host = mapped_or_nullptr(map, to_string_no_port(ip))) {
		return typesafe_sprintf("%x:%x", *host, ip.port);
	}

	return "";
}

static ip_to_host resolve_all(
	const std::vector<std::string>& official_hosts
) {
	LOG("Resolving all official servers.");

	ip_to_host out;

	for (const auto& host : official_hosts) {
		if (auto result = hostname_to_netcode_address_t(host)) {
			out[::to_string_no_port(*result)] = host;
		}
	}

	return out;
}

static auto to_webrtc_alias(
	const bool is_ranked,
	std::string location_id,
	const std::string& full_name
) {
	if (location_id == "us-central") {
		location_id = "us";
	}

	if (is_ranked) {
		location_id = "ranked-" + location_id;
	}

	std::string rest;
	int index = 0;

	if (typesafe_sscanf(full_name, "%x#%x", rest, index)) {
		location_id += typesafe_sprintf(":%x", index);
	}

	return location_id;
}

void perform_masterserver(const config_lua_table& cfg) try {
	using namespace httplib;

	const auto& settings = cfg.masterserver;

	auto udp_command_sockets = std::vector<netcode_socket_raii>();

	const auto num_sockets = settings.num_udp_command_ports;

	LOG("Creating %x masterserver sockets for UDP commands.", num_sockets);

	augs::timer since_launch;

	augs::timer since_last_official_hosts_refresh;
	std::future<ip_to_host> future_ip_to_host;

	const auto official_hosts = settings.official_hosts;
	auto ip_noport_to_official = resolve_all(official_hosts);

	auto keep_official_hosts_up_to_date = [&]() {
		if (valid_and_is_ready(future_ip_to_host)) {
			ip_noport_to_official = future_ip_to_host.get();
		}

		if (since_last_official_hosts_refresh.get<std::chrono::hours>() >= 1.0) {
			if (is_free_slot(future_ip_to_host)) {
				since_last_official_hosts_refresh.reset();

				future_ip_to_host = launch_async(
					[official_hosts]() {
						return resolve_all(official_hosts);
					}
				);
			}
		}
	};

	auto find_official_url = [&](const netcode_address_t& n) {
		return ::find_official_url(ip_noport_to_official, n);
	};

	for (int i = 0; i < num_sockets; ++i) {
		const auto new_port = settings.first_udp_command_port + i;

		if (auto new_local_address = ::find_netcode_addr(settings.ip)) {
			new_local_address->port = new_port;
			udp_command_sockets.emplace_back(*new_local_address);
		}
		else {
			LOG("There was a problem binding masterserver to %x:%x! Quitting.", settings.ip, new_port);
			return;
		}

		LOG("Created masterserver socket at: %x", ::ToString(udp_command_sockets.back().socket.address));
	}

	auto find_socket_by_port = [&](const port_type port) -> netcode_socket_raii* {
		const auto first = settings.first_udp_command_port;
		const auto index = static_cast<std::size_t>(port - first);

		if (index < udp_command_sockets.size()) {
			return std::addressof(udp_command_sockets[index]);
		}

		return nullptr;
	};

	auto banlist_to_set = [](const auto& path) {
		std::pair<std::unordered_set<std::string>, std::unordered_set<std::string>> out;

		try {
			auto content = augs::file_to_string(path);
			auto s = std::stringstream(content);

			for (std::string line; std::getline(s, line); ) {
				auto space = line.find(' ');
				out.first.insert(line.substr(0, space));

				if (space != std::string::npos) {
					auto server_name = line.substr(space + 1);
					out.second.insert(::to_lowercase(server_name));
				}
			}
		}
		catch (...) {

		}

		return out;
	};

	auto banlist_notifications 	= banlist_to_set(USER_DIR / "masterserver_banlist_notifications.txt");
	auto banlist_servers 		= banlist_to_set(USER_DIR / "masterserver_banlist_servers.txt");

	auto is_banned_notifications = [&](netcode_address_t t) {
		t.port = 0;
		return found_in(banlist_notifications.first, ::ToString(t));
	};

	auto is_banned_server = [&](netcode_address_t t) {
		t.port = 0;
		return found_in(banlist_servers.first, ::ToString(t));
	};

	auto is_banned_notifications_name = [&](const std::string& t) {
		return found_in(banlist_notifications.second, ::to_lowercase(t));
	};

	auto is_banned_server_name = [&](const std::string& t) {
		return found_in(banlist_servers.second, ::to_lowercase(t));
	};

	auto signalling = signalling_server(
		settings.signalling_peer_timeout_secs,
		settings.signalling_server_bind_address,
		settings.signalling_server_port,
		settings.signalling_ssl_cert_path,
		settings.signalling_ssl_private_key_path
	);

	std::unordered_map<netcode_address_t, masterserver_client> server_list;

	std::vector<std::byte> serialized_list;
	std::string serialized_list_json = "[]";

	std::shared_mutex serialized_list_mutex;

	std::unique_ptr<httplib::Server> http_ptr;
	std::unique_ptr<httplib::Server> fallback_http_ptr;

	const auto& cert_path = settings.ssl_cert_path;
	const auto& key_path = settings.ssl_private_key_path;

	if (!cert_path.empty() && !key_path.empty()) {
		LOG("Starting HTTPS server.");

		const auto cert = augs::string_windows_friendly(cert_path);
		const auto key = augs::string_windows_friendly(key_path);

		http_ptr = std::make_unique<httplib::SSLServer>(cert.c_str(), key.c_str());

		if (settings.fallback_http_server_list_port != 0) {
			fallback_http_ptr = std::make_unique<httplib::Server>();
		}
	}
	else {
		LOG("Starting HTTP server. Cert or key file unspecified.");

		http_ptr = std::make_unique<httplib::Server>();
	}

	const auto masterserver_dump_path = USER_DIR / "masterserver.dump";

	auto reserialize_list = [&]() {
		MSR_LOG("Reserializing the server list.");

		const auto peer_map = signalling.get_new_peers_map();

		std::lock_guard<std::shared_mutex> lock(serialized_list_mutex);

		serialized_list.clear();

		auto ss = augs::ref_memory_stream(serialized_list);

		for (auto& server : server_list) {
			const auto address = server.first;

			augs::write_bytes(ss, address);
			augs::write_bytes(ss, server.second.meta);
			augs::write_bytes(ss, server.second.last_heartbeat);
		}

		for (auto& server : peer_map) {
			if (!server.second.is_server()) {
				continue;
			}

			const auto ip_address = server.second.ip_informational;

			masterserver_entry_meta meta;
			meta.time_hosted = server.second.time_hosted;
			meta.type = server_type::WEB;
			meta.webrtc_id = webrtc_id_type(server.first);

			augs::write_bytes(ss, ip_address);
			augs::write_bytes(ss, meta);
			augs::write_bytes(ss, server.second.last_heartbeat);
		}

		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

		writer.StartArray();

		auto write_json_entry = [&](
			const server_heartbeat& data,
			const masterserver_entry_meta meta,
			const double time_last_heartbeat,
			const netcode_address_t& ip
		) {
			server_list_entry_json next;

			next.server_version = data.server_version;

			next.official_url = meta.official_url;
			next.is_ranked = meta.is_official_server() && data.is_ranked_server();
			next.is_web_server = meta.type == server_type::WEB;
			next.name = data.server_name;
			next.ip = ::ToString(ip);
			next.webrtc_id = meta.webrtc_id;

			next.time_hosted = meta.time_hosted;
			next.time_last_heartbeat = time_last_heartbeat;
			next.arena = data.current_arena;
			next.game_mode = data.game_mode;

			next.num_spectating = data.get_num_spectators();
			next.num_playing = data.num_online - next.num_spectating;

			next.slots = data.max_online;

			next.nat = data.nat.type;

			if (data.internal_network_address.has_value()) {
				next.internal_network_address = ::ToString(*data.internal_network_address);
			}

			if (data.is_editor_playtesting_server) {
				next.is_editor_playtesting_server = data.is_editor_playtesting_server;
			}

			next.score_resistance = data.score_resistance;
			next.score_metropolis = data.score_metropolis;

			next.players_resistance = data.players_resistance;
			next.players_metropolis = data.players_metropolis;
			next.players_spectating = data.players_spectating;

			next.site_displayed_address = [&next]() {
				if (!next.official_url.empty()) {
					return next.official_url;
				}

				if (next.is_web_server) {
					return next.webrtc_id;
				}

				return next.ip;
			}();

			augs::write_json(writer, next);
		};

		for (const auto& server : server_list) {
			write_json_entry(
				server.second.last_heartbeat,
				server.second.meta,
				server.second.time_last_heartbeat,
				server.first
			);
		}

		for (const auto& server : peer_map) {
			if (!server.second.is_server()) {
				continue;
			}

			masterserver_entry_meta meta;
			meta.time_hosted = server.second.time_hosted;
			meta.type = server_type::WEB;
			meta.webrtc_id = webrtc_id_type(server.first);
			
			write_json_entry(
				server.second.last_heartbeat,
				meta,
				server.second.time_last_heartbeat,
				server.second.ip_informational
			);
		}

		writer.EndArray();

		serialized_list_json = s.GetString();
	};

	auto dump_server_list_to_file = [&]() {
		if (!serialized_list.empty()) {
			LOG("Saving servers to %x", masterserver_dump_path);
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

			const auto current_time = augs::date_time::secs_since_epoch();

			while (source.peek() != EOF) {
				const auto address = augs::read_bytes<netcode_address_t>(source);

				masterserver_client entry;

				webrtc_id_type dummy;
				augs::read_bytes(source, dummy);
				augs::read_bytes(source, entry.meta);
				augs::read_bytes(source, entry.last_heartbeat);

				entry.time_last_heartbeat = current_time;

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
		return [data=serialized_list](uint64_t offset, uint64_t length, DataSink& sink) {
			return sink.write(reinterpret_cast<const char*>(&data[offset]), length);
		};
	};

	auto make_json_list_streamer_lambda = [&]() {
		return [data=serialized_list_json](uint64_t offset, uint64_t length, DataSink& sink) {
			return sink.write(reinterpret_cast<const char*>(&data[offset]), length);
		};
	};

	auto remove_from_list = [&](const auto& by_external_addr) {
		server_list.erase(by_external_addr);
		reserialize_list();
	};

	struct webhook_job {
		std::unique_ptr<std::future<std::string>> job;
	};

	std::vector<webhook_job> pending_jobs;

	auto finalize_webhook_jobs = [&]() {
		auto finalize = [&](auto& webhook_job) {
			return is_ready(*webhook_job.job);
		};

		erase_if(pending_jobs, finalize);
	};

	auto push_webhook_job = [&](auto&& f) {
		auto ptr = std::make_unique<std::future<std::string>>(
			std::async(std::launch::async, std::forward<decltype(f)>(f))
		);

		pending_jobs.emplace_back(webhook_job{ std::move(ptr) });
	};

	auto push_new_server_webhook = [&](const netcode_address_t& from, const server_heartbeat& data) {
		const auto ip_str = ::ToString(from);

		if (is_banned_notifications(from)) {
			return;
		}

		if (is_banned_notifications_name(data.server_name)) {
			return;
		}

		const auto passed = since_launch.get<std::chrono::seconds>();
		const auto mute_for_secs = settings.suppress_community_server_webhooks_after_launch_for_secs;

		if (passed < mute_for_secs) {
			MSR_LOG("Suppressing notifications for %x as masterserver has only just launched (%x/%x).", passed, mute_for_secs);
			return;
		}

		if (auto discord_webhook_url = parsed_url(cfg.server_private.discord_webhook_url); discord_webhook_url.valid()) {
			MSR_LOG("Posting a discord webhook job");

			push_webhook_job(
				[ip_str, data, discord_webhook_url]() -> std::string {
					const auto ca_path = CA_CERT_PATH;
					auto client = httplib_utils::make_client(discord_webhook_url);

					const auto game_mode_name = std::string(data.game_mode);

					auto items = discord_webhooks::form_new_community_server(
						"Server list",
						data.server_name,
						ip_str,
						data.current_arena,
						game_mode_name,
						data.max_online,
						nat_type_to_string(data.nat.type),
						data.is_editor_playtesting_server
					);

					MSR_LOG("Sending a discord notification for %x", data.server_name);

					client->Post(discord_webhook_url.location.c_str(), items);

					return "";
				}
			);
		}
		else {
			if (cfg.server_private.discord_webhook_url.size() > 0) {
				MSR_LOG("Discord webhook url was invalid.");
			}
		}

		if (auto telegram_webhook_url = parsed_url(cfg.server_private.telegram_webhook_url); telegram_webhook_url.valid()) {
			MSR_LOG("Posting a telegram webhook job");

			auto telegram_channel_id = cfg.server_private.telegram_channel_id;

			push_webhook_job(
				[ip_str, data, telegram_webhook_url, telegram_channel_id]() -> std::string {
					const auto ca_path = CA_CERT_PATH;
					auto client = httplib_utils::make_client(telegram_webhook_url);

					auto items = telegram_webhooks::form_new_community_server(
						telegram_channel_id,
						data.server_name,
						ip_str,
						data.is_editor_playtesting_server
					);

					const auto location = telegram_webhook_url.location + "/sendMessage";

					MSR_LOG("Sending a telegram notification for %x", data.server_name);

					client->Post(location.c_str(), items);

					return "";
				}
			);
		}
		else {
			if (cfg.server_private.telegram_webhook_url.size() > 0) {
				MSR_LOG("Telegram webhook url was invalid.");
			}
		}
	};

	auto define_http_server = [&](auto& server) {
		server.Get("/server_list_binary", [&](const Request&, Response& res) {
			std::shared_lock<std::shared_mutex> lock(serialized_list_mutex);

			set_cors(res);

			if (serialized_list.size() > 0) {
				MSR_LOG("List request arrived. Sending list of size: %x", serialized_list.size());

				res.set_content_provider(
					serialized_list.size(),
					"application/octet-stream",
					make_list_streamer_lambda()
				);
			}
		});

		server.Get("/server_list_json", [&](const Request&, Response& res) {
			std::shared_lock<std::shared_mutex> lock(serialized_list_mutex);

			set_cors(res);

			if (serialized_list_json.size() > 0) {
				MSR_LOG("JSON list request arrived. Sending list of size: %x", serialized_list_json.size());

				res.set_content_provider(
					serialized_list_json.size(),
					"application/json",
					make_json_list_streamer_lambda()
				);
			}
		});

		server.Options("/server_list_binary", [](const httplib::Request&, httplib::Response& res) {
			set_cors(res);
			res.status = 204;
		});

		server.Options("/server_list_json", [](const httplib::Request&, httplib::Response& res) {
			set_cors(res);
			res.status = 204;
		});
	};

	define_http_server(*http_ptr);

	if (fallback_http_ptr) {
		define_http_server(*fallback_http_ptr);
	}

	LOG("Hosting a server list at port: %x (HTTP)", settings.server_list_port);

	auto listening_thread = std::thread([&http_ptr, in_settings=settings]() {
		http_ptr->listen(in_settings.ip.c_str(), in_settings.server_list_port);
		LOG("The HTTP listening thread has quit.");
	});

	std::optional<std::thread> fallback_listening_thread;

	if (fallback_http_ptr) {
		LOG("Hosting a FALLBACK HTTP server list at port: %x (HTTP)", settings.fallback_http_server_list_port);

		fallback_listening_thread = std::thread([&fallback_http_ptr, in_settings=settings]() {
			fallback_http_ptr->listen(in_settings.ip.c_str(), in_settings.fallback_http_server_list_port);
			LOG("The fallback HTTP listening thread has quit.");
		});
	}

	while (true) {
#if PLATFORM_UNIX
		if (signal_status != 0) {
			const auto sig = signal_status.load();

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

		const auto current_time = augs::date_time::secs_since_epoch();

		finalize_webhook_jobs();

		auto send_to_gameserver = [&udp_command_sockets](const auto& typed_command, netcode_address_t server_address) {
			auto& socket_readable_by_gameserver_address = udp_command_sockets[0];

			auto bytes = make_gameserver_command_bytes(typed_command);
			netcode_socket_send_packet(&socket_readable_by_gameserver_address.socket, &server_address, bytes.data(), bytes.size());
			LOG("Sending %x b to %x", bytes.size(), ::ToString(server_address));
		};

		auto process_socket_messages = [&](
			netcode_socket_t socket,
			const netcode_address_t from,
			const uint8_t* const packet_buffer,
			const int packet_bytes
		) {
			if (is_banned_server(from)) {
				return;
			}

			try {
				auto send_to_with_socket = [&](auto which_socket, auto to, const auto& typed_response) {
					auto bytes = augs::to_bytes(masterserver_response(typed_response));
					netcode_socket_send_packet(&which_socket, &to, bytes.data(), bytes.size());
				};

				auto send_to = [&](auto to, const auto& typed_response) {
					send_to_with_socket(socket, to, typed_response);
				};

				auto send_back = [&](const auto& typed_response) {
					send_to(from, typed_response);
				};

				auto handle = [&](const auto& typed_request) {
					using R = remove_cref<decltype(typed_request)>;

					if constexpr(std::is_same_v<R, masterserver_in::goodbye>) {
						// TODO_SECURITY: has to include a ticket

						if (const auto entry = mapped_or_nullptr(server_list, from)) {
							LOG("The server at %x (%x) has sent a goodbye.", ::ToString(from), entry->last_heartbeat.server_name);
							remove_from_list(from);
						}
					}
					else if constexpr(std::is_same_v<R, masterserver_in::webrtc_signalling_payload>) {
						signalling.relay_message(
							typed_request,
							::ToString(from)
						);
					}
					else if constexpr(std::is_same_v<R, masterserver_in::heartbeat>) {
						if (is_banned_server_name(typed_request.server_name)) {
							return;
						}

						if (typed_request.is_valid()) {
							auto it = server_list.try_emplace(from);

							const bool is_new_server = it.second;
							auto& server_entry = (*it.first).second;

							const auto heartbeat_before = server_entry.last_heartbeat;
							server_entry.last_heartbeat = typed_request;
							server_entry.time_last_heartbeat = current_time;
							server_entry.meta.time_hosted = current_time;
							server_entry.meta.official_url = find_official_url(from);

							const auto ip_str = ::ToString(from);

							if (server_entry.meta.is_official_server()) {
								const auto new_webrtc_alias = ::to_webrtc_alias(
									server_entry.last_heartbeat.is_ranked_server(),
									server_entry.last_heartbeat.get_location_id(),
									server_entry.last_heartbeat.server_name
								);

								server_entry.meta.webrtc_id = new_webrtc_alias;
								signalling.set_webrtc_id_alias(new_webrtc_alias, ip_str);
							}

							const bool heartbeats_mismatch = heartbeat_before != server_entry.last_heartbeat;

							if (is_new_server) {
								if (!typed_request.suppress_new_community_server_webhook) {
									MSR_LOG("New server sent a heartbeat from %x. Sending a notification.", ip_str);
									push_new_server_webhook(from, typed_request);
								}
								else {
									MSR_LOG("New server sent a heartbeat from %x. Skipping notification due to a flag.", ip_str);
								}
							}

							MSR_LOG("Packet (%x bytes) from %x. New: %x. Heartbeat changed: %x.", packet_bytes, ::ToString(from), is_new_server, heartbeats_mismatch);

							if (is_new_server || heartbeats_mismatch) {
								reserialize_list();
							}
						}
					}
					else if constexpr(std::is_same_v<R, masterserver_in::tell_me_my_address>) {
						masterserver_out::tell_me_my_address response;
						response.address = from;
						response.session_timestamp = typed_request.session_timestamp;

						MSR_LOG("TELL_ME_MY_ADDRESS arrived from: %x", ::ToString(from));
						send_back(response);
					}
					else if constexpr(std::is_same_v<R, masterserver_in::stun_result_info>) {
						/* Just relay this */

						const auto session_guid = typed_request.session_guid;

						auto response = masterserver_out::stun_result_info();
						response.session_guid = session_guid;

						{
							const auto resolved_port = typed_request.resolved_external_port;

							if (resolved_port == 0) {
								/* Let masterserver resolve it */
								const auto masterserver_visible_port = from.port;
								response.resolved_external_port = masterserver_visible_port;
							}
							else {
								/* Result of server STUNning on its own */
								response.resolved_external_port = resolved_port;
							}
						}

						const auto& recipient = typed_request.client_origin;
						const auto client_used_probe = recipient.probe;

						MSR_LOG("Received stun_result_info from a gameserver (session guid: %f, resolved port: %x). Relaying this to: %x (original probe port: %x)", session_guid, response.resolved_external_port, ::ToString(recipient.address), client_used_probe);

						if (const auto socket = find_socket_by_port(client_used_probe)) {
							send_to_with_socket(socket->socket, recipient.address, response);
						}
						else {
							MSR_LOG("Invalid client_used_probe: %x", client_used_probe); 
						}
					}
					else if constexpr(std::is_same_v<R, masterserver_in::nat_traversal_step>) {
						auto punched_server = typed_request.target_server;

						const auto address_string = ::ToString(punched_server);
						MSR_LOG("A request arrived from %x to traverse %x", ::ToString(from), address_string);

						const bool should_send_request = [&]() {
							if (address_string == "NONE" || punched_server.type == NETCODE_ADDRESS_NONE) {
								MSR_LOG("The target address was invalid. Dropping the request.");
								return false;
							}

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

						if (should_send_request) {
							auto step_request = masterserver_out::nat_traversal_step();

							step_request.client_origin = { from, socket.address.port };
							step_request.payload = typed_request.payload;

							send_to_gameserver(step_request, punched_server);
						}
					}
				};

				const auto request = augs::from_bytes<masterserver_request>(packet_buffer, packet_bytes);
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
			::receive_netcode_packets<true>(s.socket, process_socket_messages);
		}

		const auto timeout_secs = settings.server_entry_timeout_secs;

		auto process_entry_logic = [&](auto& server_entry) {
			(void)server_entry;
		};

		auto erase_if_dead = [&](auto& server_entry) {
			const bool timed_out = current_time - server_entry.second.time_last_heartbeat >= timeout_secs;

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

		keep_official_hosts_up_to_date();

		signalling.send_signalling_packets(send_to_gameserver);
		signalling.process_timeouts_if_its_time();

		if (signalling.should_reserialize()) {
			reserialize_list();
		}

		augs::sleep(settings.sleep_ms / 1000);
	}

	LOG("Stopping the HTTP masterserver.");
	http_ptr->stop();

	LOG("Joining the HTTP listening thread.");
	listening_thread.join();

	if (fallback_http_ptr) {
		fallback_http_ptr->stop();
	}

	if (fallback_listening_thread.has_value()) {
		LOG("Joining the FALLBACK HTTP listening thread.");
		fallback_listening_thread->join();
	}

	dump_server_list_to_file();
}
catch (const netcode_socket_raii_error& err) {
	LOG(err.what());
}
