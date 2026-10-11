#include <cstddef>
#include <cstdint>
#if PLATFORM_UNIX
#include <csignal>
#endif
#include <shared_mutex>
#include "application/masterserver/masterserver.h"
#include "3rdparty/include_httplib.h"
#include "augs/log.h"

#include "application/config_json_table.h"
#include "augs/network/netcode_sockets.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/memory_stream.h"
#include "application/masterserver/server_heartbeat.h"
#include "augs/templates/thread_templates.h"
#include "augs/misc/date_time.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/network/netcode_socket_raii.h"
#include "application/network/address_utils.h"
#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/to_bytes.h"
#include "application/masterserver/masterserver_requests.h"
#include "application/masterserver/official_servers_registry.h"
#include "application/network/secure_connect.h"
#include "augs/misc/to_hex_str.h"
#include "application/masterserver/netcode_address_hash.h"
#include "augs/string/parse_url.h"
#include "application/detail_file_paths.h"
#include "application/setups/server/webhooks.h"
#include "application/masterserver/server_list_entry_json.h"
#include "rapidjson/prettywriter.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/network/netcode_utils.h"
#include "augs/misc/httplib_utils.h"

#include "application/masterserver/masterserver_client.h"
#include "application/masterserver/signalling_server.h"
#include "application/network/network_adapters.h"
#include "augs/string/typesafe_sscanf.h"

extern std::mutex rtc_errors_lk;
extern bool track_rtc_errors;
extern std::vector<std::string> rtc_errors;

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

/*
	A server's domain is shown on the server list and joined by players, so it has to be a plain host name -
	and only the official servers may have one under hypersomnia.io, or a community server could pass for one.
*/

static bool is_acceptable_domain(const std::string& domain, const bool is_official) {
	if (domain.empty() || domain.size() > 253 || domain.front() == '.' || domain.back() == '.' || domain.front() == '-') {
		return false;
	}

	for (const auto c : domain) {
		const bool allowed = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-';

		if (!allowed) {
			return false;
		}
	}

	if (domain.find("..") != std::string::npos) {
		return false;
	}

	auto lowercase = domain;

	for (auto& c : lowercase) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}

	const bool under_hypersomnia_io = lowercase == "hypersomnia.io" || ends_with(lowercase, ".hypersomnia.io");

	return is_official || !under_hypersomnia_io;
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

work_result perform_masterserver(const config_json_table& cfg) try {
	using namespace httplib;

	const auto& settings = cfg.masterserver;

	auto udp_command_sockets = std::vector<netcode_socket_raii>();

	const auto num_sockets = settings.num_udp_command_ports;

	LOG("Creating %x masterserver sockets for UDP commands.", num_sockets);

	augs::timer since_launch;

	auto official_servers = official_servers_registry(settings.official_servers_dir);
	augs::timer since_last_official_servers_reload;

	LOG("Loaded %x official servers from %x.", official_servers.size(), settings.official_servers_dir);

	auto keep_official_servers_up_to_date = [&]() {
		if (since_last_official_servers_reload.get<std::chrono::seconds>() >= 60.0) {
			since_last_official_servers_reload.reset();
			official_servers.reload();

			return true;
		}

		return false;
	};

	/*
		The official servers clients can get secure connect tokens for, by their address and by their WebRTC alias.
		Rebuilt on the main thread whenever the list changes, read by the HTTP threads.
	*/

	struct secure_connect_target {
		std::string address;
		std::string server_id;
		augs::hmac_sha256_type netcode_key;
	};

	std::mutex secure_connect_targets_mutex;
	std::unordered_map<std::string, secure_connect_target> secure_connect_targets;

	std::mutex token_requests_mutex;
	std::unordered_map<std::string, unsigned> token_requests_this_minute;
	augs::timer since_token_requests_reset;

	for (int i = 0; i < num_sockets; ++i) {
		const auto new_port = settings.first_udp_command_port + i;

		if (auto new_local_address = ::find_netcode_addr(settings.ip)) {
			new_local_address->port = new_port;
			udp_command_sockets.emplace_back(*new_local_address);
		}
		else {
			LOG("There was a problem binding masterserver to %x:%x! Quitting.", settings.ip, new_port);
			return work_result::FAILURE;
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
	(void)find_socket_by_port;

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

	auto push_new_server_webhook = [&](const netcode_address_t& from, const server_heartbeat& data, const bool is_web) {
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
				[ip_str, data, discord_webhook_url, is_web]() -> std::string {
					auto client = httplib_utils::make_client(discord_webhook_url);

					const auto game_mode_name = std::string(data.game_mode);

					auto items = discord_webhooks::form_new_community_server(
						"Server list",
						data.server_name,
						ip_str,
						data.current_arena,
						game_mode_name,
						data.server_slots,
						nat_type_to_string(data.nat.type),
						data.is_editor_playtesting_server,
						is_web
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
				[ip_str, data, telegram_webhook_url, telegram_channel_id, is_web]() -> std::string {
					auto client = httplib_utils::make_client(telegram_webhook_url);

					auto items = telegram_webhooks::form_new_community_server(
						telegram_channel_id,
						data.server_name,
						ip_str,
						data.is_editor_playtesting_server,
						is_web
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

	auto previous_peers = signalling_server::peer_map();

	auto rebuild_secure_connect_targets = [&]() {
		std::scoped_lock lk(secure_connect_targets_mutex);

		secure_connect_targets.clear();

		for (const auto& server : server_list) {
			const auto& entry = server.second;

			if (!entry.meta.is_official) {
				continue;
			}

			if (const auto official = official_servers.find_by_server_id(entry.official_server_id)) {
				const auto address = ::ToString(server.first);
				const auto target = secure_connect_target { address, official->server_id, official->keys.netcode_key };

				secure_connect_targets[address] = target;

				if (!entry.meta.webrtc_id.empty()) {
					secure_connect_targets[std::string(entry.meta.webrtc_id)] = target;
				}
			}
		}
	};

	auto reserialize_list = [&]() {
		// MSR_LOG("Reserializing the server list.");

		rebuild_secure_connect_targets();

		const auto peer_map = signalling.get_new_peers_map();

		std::lock_guard<std::shared_mutex> lock(serialized_list_mutex);

		serialized_list.clear();

		auto ss = augs::ref_memory_stream(serialized_list);

		for (auto& server : server_list) {
			const auto address = server.first;

			if (!server.second.last_heartbeat.show_on_server_list) {
				continue;
			}

			augs::write_bytes(ss, address);
			augs::write_bytes(ss, server.second.meta);
			augs::write_bytes(ss, server.second.last_heartbeat);
		}

		for (auto& server : peer_map) {
			if (!server.second.is_server()) {
				continue;
			}

			if (!server.second.last_heartbeat.show_on_server_list) {
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

			bool push_webhook = false;

			if (!found_in(previous_peers, server.first)) {
				push_webhook = true;
			}
			else {
				if (!previous_peers[server.first].is_server()) {
					push_webhook = true;
				}
			}

			if (push_webhook) {
				push_new_server_webhook(
					ip_address,
					server.second.last_heartbeat,
					true
				);
			}
		}

		previous_peers = peer_map;

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

			next.is_official = meta.is_official;
			next.domain = meta.domain;
			next.is_ranked = meta.is_official_server() && data.is_ranked_server();
			next.is_web_server = meta.type == server_type::WEB;
			next.name = data.server_name;
			next.ip = ::ToString(ip);
			next.webrtc_id = meta.webrtc_id;

			if (!next.webrtc_id.empty()) {
				next.browser_connect_string = next.webrtc_id;
			}
			else {
				next.browser_connect_string = next.ip;
			}

			next.time_hosted = meta.time_hosted;
			next.time_last_heartbeat = time_last_heartbeat;
			next.arena = data.current_arena;
			next.game_mode = data.game_mode;

			next.num_spectating = data.get_num_spectators();
			next.num_playing = data.num_online - next.num_spectating;
			next.num_online_humans = data.num_online_humans;

			next.slots = data.server_slots;

			next.nat = data.nat.type;

			if (data.internal_network_address.has_value()) {
				next.internal_network_address = ::ToString(*data.internal_network_address);
			}

			if (data.is_editor_playtesting_server) {
				next.is_editor_playtesting_server = data.is_editor_playtesting_server;
			}

			next.score_resistance = data.score_resistance;
			next.score_metropolis = data.score_metropolis;

			/*
				Exact decimals in the JSON, e.g. 0.8 - not the float nearest to it.
			*/

			next.speed = data.speed_hundredths / 100.0;
			next.bullet_speed = data.bullet_speed_hundredths / 100.0;
			next.zoom = data.zoom_hundredths / 100.0;

			next.players_resistance = data.players_resistance;
			next.players_metropolis = data.players_metropolis;
			next.players_spectating = data.players_spectating;

			next.site_displayed_address = [&next, &ip]() {
				if (!next.domain.empty()) {
					return typesafe_sprintf("%x:%x", next.domain, ip.port);
				}

				if (next.is_web_server) {
					return next.webrtc_id;
				}

				return next.ip;
			}();

			augs::write_json(writer, next);
		};

		for (const auto& server : server_list) {
			if (!server.second.last_heartbeat.show_on_server_list) {
				continue;
			}

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

			if (!server.second.last_heartbeat.show_on_server_list) {
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

				/*
					The dump does not say which official server an entry was -
					its next signed heartbeat makes it official again.
				*/

				entry.meta.is_official = false;

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

	auto push_alert_webhook = [&](std::string err) {
		if (auto telegram_webhook_url = parsed_url(cfg.server_private.telegram_alerts_webhook_url); telegram_webhook_url.valid()) {
			MSR_LOG("Posting a telegram webhook job");

			auto telegram_channel_id = cfg.server_private.telegram_alerts_channel_id;

			push_webhook_job(
				[err, telegram_webhook_url, telegram_channel_id]() -> std::string {
					auto client = httplib_utils::make_client(telegram_webhook_url);

					auto items = telegram_webhooks::form_error_report(
						telegram_channel_id,
						err
					);

					const auto location = telegram_webhook_url.location + "/sendMessage";

					client->Post(location.c_str(), items);

					return "";
				}
			);
		}
		else {
			if (cfg.server_private.telegram_alerts_webhook_url.size() > 0) {
				MSR_LOG("Telegram logs webhook url was invalid.");
			}
		}
	};

	auto define_http_server = [&](auto& server, const bool encrypted) {
		server.Get("/server_list_binary", [&](const Request&, Response& res) {
			std::shared_lock<std::shared_mutex> lock(serialized_list_mutex);

			set_cors(res);

			if (serialized_list.size() > 0) {
				// MSR_LOG("List request arrived. Sending list of size: %x", serialized_list.size());

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
				// MSR_LOG("JSON list request arrived. Sending list of size: %x", serialized_list_json.size());

				res.set_content_provider(
					serialized_list_json.size(),
					"application/json",
					make_json_list_streamer_lambda()
				);
			}
		});

		/*
			A secure connect token for one official server, by its address or WebRTC alias.
			Served over HTTPS (nginx in front), so the session keys inside reach only the client asking.
		*/

		server.Get("/connect_token", [&, encrypted](const Request& req, Response& res) {
			set_cors(res);

			const bool from_this_machine = 
				req.remote_addr == "127.0.0.1" 
				|| req.remote_addr == "::1" 
				|| req.remote_addr == "::ffff:127.0.0.1"
			;

			/*
				The token carries the session keys in the clear, so over plain HTTP it goes only to this machine -
				nginx, which serves it over HTTPS - never straight to anyone who asks on the public port.
			*/

			if (!encrypted && !from_this_machine && !settings.connect_tokens_over_plain_http) {
				res.status = 403;
				return;
			}

			const auto requester = [&]() -> std::string {
				const auto address = from_this_machine && req.has_header("X-Real-IP") ? req.get_header_value("X-Real-IP") : req.remote_addr;

				/*
					An IPv6 client usually has a whole /64 to rotate through, so it is limited as one.
				*/

				if (address.find(':') == std::string::npos || begins_with(address, "::ffff:")) {
					return address;
				}

				const auto double_colon = address.find("::");
				const auto head = address.substr(0, double_colon);
				const auto tail = double_colon == std::string::npos ? std::string() : address.substr(double_colon + 2);

				auto split = [](const std::string& s) {
					auto out = std::vector<std::string>();
					std::size_t start = 0;

					while (!s.empty() && start <= s.size()) {
						const auto end = s.find(':', start);
						out.push_back(s.substr(start, end == std::string::npos ? std::string::npos : end - start));

						if (end == std::string::npos) {
							break;
						}

						start = end + 1;
					}

					return out;
				};

				const auto head_groups = split(head);
				const auto tail_groups = split(tail);

				auto groups = head_groups;

				const auto missing = 8 - static_cast<int>(head_groups.size() + tail_groups.size());

				for (int i = 0; i < missing; ++i) {
					groups.push_back("0");
				}

				groups.insert(groups.end(), tail_groups.begin(), tail_groups.end());

				std::string prefix;

				for (std::size_t i = 0; i < 4 && i < groups.size(); ++i) {
					prefix += groups[i] + ":";
				}

				return prefix + ":/64";
			}();

			{
				std::scoped_lock lk(token_requests_mutex);

				if (since_token_requests_reset.get<std::chrono::seconds>() >= 60.0) {
					since_token_requests_reset.reset();
					token_requests_this_minute.clear();
				}

				/*
					Also a bound on the number of requesters, so that the map stays small whatever the number of addresses.
					Once reached, only the requesters not yet counted this minute wait - the others keep their limit.
				*/

				const bool new_requester = !token_requests_this_minute.contains(requester);
				const bool too_many_requesters = new_requester && token_requests_this_minute.size() >= 100000;

				if (too_many_requesters || ++token_requests_this_minute[requester] > 30) {
					res.status = 429;
					return;
				}
			}

			const auto requested = req.get_param_value("server");

			const auto target = [&]() -> std::optional<secure_connect_target> {
				std::scoped_lock lk(secure_connect_targets_mutex);

				if (const auto found = mapped_or_nullptr(secure_connect_targets, requested)) {
					return *found;
				}

				return std::nullopt;
			}();

			if (!target.has_value()) {
				res.status = 404;
				res.set_content("{\"error\":\"Not an official server.\"}", "application/json");
				return;
			}

			const auto user_data = secure_connect::make_user_data(target->server_id);

			if (!user_data.has_value()) {
				res.status = 500;
				return;
			}

			const auto client_id = [&]() {
				thread_local auto device = std::random_device();
				return (static_cast<uint64_t>(device()) << 32) | static_cast<uint64_t>(device());
			}();

			auto token = std::array<uint8_t, NETCODE_CONNECT_TOKEN_BYTES>();
			auto user_data_copy = *user_data;
			const char* address = target->address.c_str();

			/*
				No netcode timeout, the same as the clients' own insecure tokens - the game times connections out itself.
			*/

			const auto result = netcode_generate_connect_token(
				1,
				&address,
				&address,
				secure_connect::token_lifetime_secs,
				-1,
				client_id,
				DEFAULT_GAME_PORT_V,
				target->netcode_key.data(),
				user_data_copy.data(),
				token.data()
			);

			if (result != NETCODE_OK) {
				LOG("Failed to generate a connect token for %x (%x).", target->server_id, target->address);
				res.status = 500;
				return;
			}

			const auto json = typesafe_sprintf(
				"{\"client_id\":\"%x\",\"server_id\":\"%x\",\"token\":\"%x\"}",
				client_id,
				target->server_id,
				::get_hex_representation(token.data(), token.size())
			);

			res.set_content(json, "application/json");
		});

		server.Options("/connect_token", [](const httplib::Request&, httplib::Response& res) {
			set_cors(res);
			res.status = 204;
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

	define_http_server(*http_ptr, fallback_http_ptr != nullptr);

	if (fallback_http_ptr) {
		define_http_server(*fallback_http_ptr, false);
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

	if (track_rtc_errors) {
		push_alert_webhook("Server list restarted.");
	}

	std::optional<work_result> ms_work_result;

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
						if (const auto entry = mapped_or_nullptr(server_list, from)) {
							/*
								An official server can only be removed with its own signature,
								otherwise a goodbye from a spoofed address could take it off the list.
							*/

							if (entry->meta.is_official_server()) {
								const auto signer = official_servers.authenticate(typed_request, ::official_signature_now_ms(), ::ToString(from));

								if (signer == nullptr || signer->server_id != entry->official_server_id) {
									LOG("Ignoring an unauthenticated goodbye for the official server at %x.", ::ToString(from));
									return;
								}
							}

							/*
								TODO_SECURITY: a community server's goodbye is not authenticated -
								anyone spoofing its address can take it off the list until its next heartbeat.
							*/

							LOG("The server at %x (%x) has sent a goodbye.", ::ToString(from), entry->last_heartbeat.server_name);
							remove_from_list(from);
						}
					}
					else if constexpr(std::is_same_v<R, masterserver_in::webrtc_signalling_payload>) {
						signalling.relay_message_from_native(
							typed_request,
							::ToString(from)
						);
					}
					else if constexpr(std::is_same_v<R, masterserver_in::heartbeat>) {
						if (is_banned_server_name(typed_request.server_name)) {
							return;
						}

						if (typed_request.is_valid()) {
							const auto ip_str = ::ToString(from);
							const auto signer = official_servers.authenticate(typed_request, ::official_signature_now_ms(), ip_str);

							/*
								Only the official server itself can update its entry,
								otherwise a heartbeat from a spoofed address could rename it or change its ranked status.
								A revoked or misconfigured official server thus times out and then gets listed as a community one.
							*/

							if (signer == nullptr) {
								if (const auto existing = mapped_or_nullptr(server_list, from); existing != nullptr && existing->meta.is_official_server()) {
									LOG("Ignoring an unauthenticated heartbeat from the official server at %x.", ip_str);
									return;
								}

								/*
									A signature that fails (a replay, a request delayed past a newer one, a clock off)
									must not list the server as a community one under its official name either.
								*/

								if (typed_request.signature.has_value()) {
									LOG("Ignoring a heartbeat with an invalid signature from %x.", ip_str);
									return;
								}
							}
							else {
								/*
									The secret of one official server must not take over the entry of another
									(their operators are not all the same people), and one official server is listed
									at one address only - otherwise anyone who saw its signed heartbeat could
									send it first from their own address.
								*/

								if (const auto existing = mapped_or_nullptr(server_list, from); existing != nullptr && existing->meta.is_official_server()) {
									if (existing->official_server_id != signer->server_id) {
										LOG("Ignoring a heartbeat signed by %x for the official server %x at %x.", signer->server_id, existing->official_server_id, ip_str);
										return;
									}
								}

								for (const auto& other : server_list) {
									if (other.first != from && other.second.meta.is_official_server() && other.second.official_server_id == signer->server_id) {
										LOG("Ignoring a heartbeat signed by %x from %x: it is already listed at %x.", signer->server_id, ip_str, ::ToString(other.first));
										return;
									}
								}
							}

							auto it = server_list.try_emplace(from);

							const bool is_new_server = it.second;
							auto& server_entry = (*it.first).second;

							const auto heartbeat_before = server_entry.last_heartbeat;
							const bool was_official = server_entry.meta.is_official;
							server_entry.last_heartbeat = typed_request;
							server_entry.last_heartbeat.signature.reset();
							server_entry.time_last_heartbeat = current_time;
							server_entry.official_server_id = signer != nullptr ? signer->server_id : std::string();

							server_entry.meta.is_official = signer != nullptr;
							{
								const auto domain = std::string(typed_request.domain);
								server_entry.meta.domain = ::is_acceptable_domain(domain, signer != nullptr) ? typed_request.domain : decltype(typed_request.domain)();
							}

							if (server_entry.meta.is_official_server()) {
								const auto new_webrtc_alias = ::to_webrtc_alias(
									server_entry.last_heartbeat.is_ranked_server(),
									server_entry.last_heartbeat.get_location_id(),
									server_entry.last_heartbeat.server_name
								);

								server_entry.meta.webrtc_id = new_webrtc_alias;
								signalling.set_webrtc_id_alias(new_webrtc_alias, ip_str);
							}

							const bool heartbeats_mismatch = 
								heartbeat_before != server_entry.last_heartbeat
								|| was_official != server_entry.meta.is_official
							;

							if (is_new_server) {
								server_entry.meta.time_hosted = current_time;

								if (typed_request.show_on_server_list) {
									if (!typed_request.suppress_new_community_server_webhook) {
										MSR_LOG("New server sent a heartbeat from %x. Sending a notification.", ip_str);
										push_new_server_webhook(from, typed_request, false);
									}
									else {
										MSR_LOG("New server sent a heartbeat from %x. Skipping notification due to a flag.", ip_str);
									}
								}
								else {
									MSR_LOG("New server sent a heartbeat from %x. Skipping notification due show_on_server_list = false.", ip_str);
								}
							}

							// MSR_LOG("Packet (%x bytes) from %x. New: %x. Heartbeat changed: %x.", packet_bytes, ::ToString(from), is_new_server, heartbeats_mismatch);

							if (is_new_server || heartbeats_mismatch) {
								reserialize_list();
							}
						}
						else {
							// MSR_LOG("Invalid request (%x bytes) from %x.", packet_bytes, ::ToString(from));
						}
					}
					else if constexpr(std::is_same_v<R, masterserver_in::tell_me_my_address>) {
						masterserver_out::tell_me_my_address response;
						response.address = from;
						response.session_timestamp = typed_request.session_timestamp;

						MSR_LOG("TELL_ME_MY_ADDRESS arrived from: %x", ::ToString(from));
						send_back(response);
					}
				};

				const auto request = augs::from_bytes<masterserver_request>(packet_buffer, packet_bytes);
				std::visit(handle, request);
			}
			/*
				An official server is only ever removed by its signed goodbye or a timeout -
				anyone can send garbage with its address spoofed as the source.
			*/

			catch (const std::exception& err) {
				if (const auto entry = mapped_or_nullptr(server_list, from)) {
					LOG("The server at %x (%x) has sent invalid data: %x", ::ToString(from), entry->last_heartbeat.server_name, err.what());

					if (!entry->meta.is_official) {
						remove_from_list(from);
					}
				}
			}
			catch (...) {
				if (const auto entry = mapped_or_nullptr(server_list, from)) {
					LOG("The server at %x (%x) has sent invalid data: unknown error.", ::ToString(from), entry->last_heartbeat.server_name);

					if (!entry->meta.is_official) {
						remove_from_list(from);
					}
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

		if (keep_official_servers_up_to_date()) {
			/*
				A server whose secret was removed or changed is no longer official right away,
				instead of until it times out.
			*/

			for (auto& server : server_list) {
				auto& entry = server.second;

				if (entry.meta.is_official && official_servers.find_by_server_id(entry.official_server_id) == nullptr) {
					LOG("The server at %x is no longer official.", ::ToString(server.first));

					entry.meta.is_official = false;
					entry.official_server_id.clear();
				}
			}

			reserialize_list();
		}

		signalling.send_signalling_packets(send_to_gameserver);
		signalling.process_timeouts_if_its_time();

		if (signalling.should_reserialize()) {
			reserialize_list();
		}

		if (track_rtc_errors) {
			std::scoped_lock lk(rtc_errors_lk);

			if (!rtc_errors.empty()) {
				for (const auto& err : rtc_errors) {
					push_alert_webhook(err);
				}

				rtc_errors.clear();

				ms_work_result = work_result::RELAUNCH_MASTERSERVER;
				break;
			}
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

	if (ms_work_result.has_value()) {
		return *ms_work_result;
	}

	return work_result::SUCCESS;
}
catch (const netcode_socket_raii_error& err) {
	LOG(err.what());
	return work_result::FAILURE;
}
