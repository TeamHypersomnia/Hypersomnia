#include <cstddef>
#include <cstdint>
#if PLATFORM_UNIX
#include <csignal>
#endif
#include <shared_mutex>
#include "application/masterserver/masterserver.h"
#include "3rdparty/include_httplib.h"
#include "augs/log.h"
#include "augs/misc/date_time.h"

#include "application/config_json_table.h"
#include "augs/network/netcode_sockets.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/memory_stream.h"
#include "application/masterserver/server_heartbeat.h"
#include "augs/templates/thread_templates.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/network/netcode_socket_raii.h"
#include "application/network/address_utils.h"
#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/to_bytes.h"
#include "application/masterserver/masterserver_requests.h"
#include "application/masterserver/official_servers_registry.h"
#include "application/masterserver/server_request_signing.h"
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
#include "augs/misc/token_bucket.h"

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("Masterserver signalling message tokens") {
	using peer_type = signalling_server::signalling_peer;

	auto peer = peer_type();
	const auto client_burst = static_cast<int>(peer_type::client_message_burst);

	for (int i = 0; i < client_burst; ++i) {
		REQUIRE(peer.take_message_token(0.0));
	}

	REQUIRE(!peer.take_message_token(0.0));

	/*
		A server gets a higher allowance - but a finite one.
	*/

	peer.time_last_heartbeat = 1.0;
	REQUIRE(peer.is_server());

	const auto refilled = static_cast<int>(peer_type::server_messages_per_sec);

	for (int i = 0; i < refilled; ++i) {
		REQUIRE(peer.take_message_token(1.0));
	}

	REQUIRE(!peer.take_message_token(1.0));
}
#endif

extern std::mutex rtc_errors_lk;
extern bool track_rtc_errors;
extern std::vector<std::string> rtc_errors;

std::string to_lowercase(std::string s);
std::string ToString(const netcode_address_t&);

/*
	Anyone can send tell_me_my_address and its answer goes to the packet's source address,
	so spoofed sources could bounce a flood of answers off the masterserver at a victim.

	Limited per network (augs::network_of) - not per ip:port, or a new port for every packet
	would get past it - to far more than the NAT detection of one game ever asks for.
	The table of networks is cleared wholesale when full, so a flood from ever new spoofed sources
	can not grow it - but also could reset the victim's limit, hence the limit on all answers at once.
*/

struct tell_me_my_address_limiter {
	static constexpr std::size_t max_remembered_networks = 8192;

	static constexpr double network_answers_per_sec = 50.0;
	static constexpr double network_burst = 100.0;

	static constexpr double total_answers_per_sec = 2000.0;
	static constexpr double total_burst = 4000.0;

	std::unordered_map<netcode_address_t, augs::token_bucket> by_network;
	augs::token_bucket total;

	bool try_answer(const netcode_address_t& from, const double now) {
		if (by_network.size() >= max_remembered_networks) {
			by_network.clear();
		}

		if (!by_network[augs::network_of(from)].take(now, network_answers_per_sec, network_burst)) {
			return false;
		}

		return total.take(now, total_answers_per_sec, total_burst);
	}
};

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("Masterserver tell_me_my_address limiter") {
	auto address = [](const uint8_t last_octet, const uint16_t port) {
		auto out = netcode_address_t();
		out.type = NETCODE_ADDRESS_IPV4;
		out.data.ipv4[0] = 10;
		out.data.ipv4[1] = 0;
		out.data.ipv4[2] = 0;
		out.data.ipv4[3] = last_octet;
		out.port = port;
		return out;
	};

	using limiter_type = tell_me_my_address_limiter;

	SECTION("A network gets its burst, then its rate") {
		auto limiter = limiter_type();
		const auto burst = static_cast<int>(limiter_type::network_burst);

		for (int i = 0; i < burst; ++i) {
			REQUIRE(limiter.try_answer(address(1, 1000), 0.0));
		}

		REQUIRE(!limiter.try_answer(address(1, 1000), 0.0));

		/*
			Another port of the same network shares its limit.
		*/

		REQUIRE(!limiter.try_answer(address(1, 2000), 0.0));

		/*
			Another network does not.
		*/

		REQUIRE(limiter.try_answer(address(2, 1000), 0.0));

		/*
			A second later, the rate's worth of answers again.
		*/

		const auto refilled = static_cast<int>(limiter_type::network_answers_per_sec);

		for (int i = 0; i < refilled; ++i) {
			REQUIRE(limiter.try_answer(address(1, 1000), 1.0));
		}

		REQUIRE(!limiter.try_answer(address(1, 1000), 1.0));
	}

	SECTION("All answers at once are limited too") {
		auto limiter = limiter_type();
		const auto total_burst = static_cast<int>(limiter_type::total_burst);

		auto answered = 0;

		for (int i = 0; i < total_burst + 1000; ++i) {
			auto from = address(static_cast<uint8_t>(i % 256), 1000);
			from.data.ipv4[2] = static_cast<uint8_t>(i / 256);

			answered += limiter.try_answer(from, 0.0) ? 1 : 0;
		}

		REQUIRE(answered == total_burst);
	}
}
#endif

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

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("Nicknames and server names reject control characters") {
	REQUIRE(::is_nickname_valid_characters(std::string("Pythagoras")));
	REQUIRE(::is_nickname_valid_characters(std::string("z\xc5\xbc\xc3\xb3\xc5\x82w")));

	REQUIRE(!::is_nickname_valid_characters(std::string("   ")));
	REQUIRE(!::is_nickname_valid_characters(std::string("a\tb")));
	REQUIRE(!::is_nickname_valid_characters(std::string("a\nb")));
	REQUIRE(!::is_nickname_valid_characters(std::string("a\x1b[31mb")));
	REQUIRE(!::is_nickname_valid_characters(std::string("a\x7f")));
	REQUIRE(!::is_nickname_valid_characters(std::string("a\0b", 3)));
}
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
	if (domain.empty() || domain.size() > 253 || domain.front() == '.' || domain.back() == '.') {
		return false;
	}

	for (const auto c : domain) {
		const bool allowed = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-';

		if (!allowed) {
			return false;
		}
	}

	/*
		Every label (the parts between dots): 1 to 63 characters, neither beginning nor ending with a hyphen.
	*/

	for (std::size_t label_begin = 0; label_begin < domain.size();) {
		const auto dot = domain.find('.', label_begin);
		const auto label_end = dot == std::string::npos ? domain.size() : dot;
		const auto label_length = label_end - label_begin;

		if (label_length == 0 || label_length > 63 || domain[label_begin] == '-' || domain[label_end - 1] == '-') {
			return false;
		}

		label_begin = label_end + 1;
	}

	/*
		Not an IP passed for a domain (e.g. another server's, also written in hex or as one number) - a domain has a dot
		and its top-level part begins with a letter.
	*/

	const auto last_dot = domain.rfind('.');

	const bool top_level_begins_with_letter =
		last_dot != std::string::npos
		&& last_dot + 1 < domain.size()
		&& std::isalpha(static_cast<unsigned char>(domain[last_dot + 1]))
	;

	if (!top_level_begins_with_letter) {
		return false;
	}

	auto lowercase = domain;

	for (auto& c : lowercase) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}

	const bool under_hypersomnia_io = lowercase == "hypersomnia.io" || ends_with(lowercase, ".hypersomnia.io");

	return is_official || !under_hypersomnia_io;
}

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("Masterserver acceptable domains") {
	REQUIRE(::is_acceptable_domain("example.com", false));
	REQUIRE(::is_acceptable_domain("my-server.example.com", false));
	REQUIRE(::is_acceptable_domain(std::string(63, 'a') + ".com", false));

	REQUIRE(!::is_acceptable_domain("", false));
	REQUIRE(!::is_acceptable_domain(".example.com", false));
	REQUIRE(!::is_acceptable_domain("example.com.", false));
	REQUIRE(!::is_acceptable_domain("example..com", false));
	REQUIRE(!::is_acceptable_domain("-example.com", false));
	REQUIRE(!::is_acceptable_domain("example.com-", false));
	REQUIRE(!::is_acceptable_domain("example-.com", false));
	REQUIRE(!::is_acceptable_domain("sub.-example.com", false));
	REQUIRE(!::is_acceptable_domain(std::string(64, 'a') + ".com", false));
	REQUIRE(!::is_acceptable_domain("exa mple.com", false));
	REQUIRE(!::is_acceptable_domain("1.2.3.4", false));
	REQUIRE(!::is_acceptable_domain("localhost", false));

	REQUIRE(!::is_acceptable_domain("ranked.hypersomnia.io", false));
	REQUIRE(::is_acceptable_domain("ranked.hypersomnia.io", true));
}
#endif

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
		See server_request_signature.h.
	*/

	const auto cookies = masterserver_cookies();

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

	signalling.max_web_peers = settings.max_web_peers;

	signalling.heartbeat_rejected = [&](const server_heartbeat& hb) {
		return is_banned_server_name(hb.server_name);
	};

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

	/*
		At most a few "new server" notifications an hour, whatever comes - a flood of new servers must not spam the channels.
		And at most a few a day from one address (an IPv6 /64 counts as one) - a server that keeps restarting
		would otherwise be announced as new every time it comes back.
	*/

	auto new_server_notifications = std::vector<double>();

	constexpr std::size_t max_new_server_notifications_per_address_per_day = 4;
	constexpr double secs_per_day = 24.0 * 3600.0;

	auto new_server_notifications_by_address = std::unordered_map<netcode_address_t, std::vector<double>>();

	auto push_new_server_webhook = [&](const netcode_address_t& from, const server_heartbeat& data, const bool is_web) {
		const auto ip_str = ::ToString(from);

		if (is_banned_notifications(from) || is_banned_notifications_name(data.server_name)) {
			return;
		}

		/*
			Muted ones don't count - every listed server comes back as new right after a masterserver restart.
		*/

		const auto passed = since_launch.get<std::chrono::seconds>();
		const auto mute_for_secs = settings.suppress_community_server_webhooks_after_launch_for_secs;

		if (passed < mute_for_secs) {
			MSR_LOG("Suppressing notifications for %x as masterserver has only just launched (%x/%x).", ip_str, passed, mute_for_secs);
			return;
		}

		{
			const auto now = augs::steady_secs();
			erase_if(new_server_notifications, [now](const double t) { return now - t > 3600.0; });

			if (new_server_notifications.size() >= settings.max_new_server_notifications_per_hour) {
				MSR_LOG("Too many new server notifications this hour - skipping one for %x.", ip_str);
				return;
			}

			/*
				Addresses with nothing left in the last day are forgotten - the map does not grow without end.
				An unknown address (a web server's peer) never equals another one, so only the hourly limit holds for it.
			*/

			erase_if(new_server_notifications_by_address, [now](const auto& entry) {
				return entry.second.empty() || now - entry.second.back() > secs_per_day;
			});

			if (from.type != NETCODE_ADDRESS_NONE) {
				auto& of_address = new_server_notifications_by_address[augs::network_of(from)];
				erase_if(of_address, [now](const double t) { return now - t > secs_per_day; });

				if (of_address.size() >= max_new_server_notifications_per_address_per_day) {
					MSR_LOG("Too many new server notifications from %x today - skipping one.", ip_str);
					return;
				}

				of_address.push_back(now);
			}

			new_server_notifications.push_back(now);
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

	auto previous_web_servers = signalling_server::servers_map();

	/*
		See tell_me_my_address_limiter.
	*/

	auto address_answers = tell_me_my_address_limiter();

	/*
		A browser joins an official server by its alias (e.g. /game/ranked-pl:1) - only an official server has one.
	*/

	auto update_webrtc_alias = [&](masterserver_client& entry) {
		if (entry.meta.is_official_server()) {
			entry.meta.webrtc_id = ::to_webrtc_alias(
				entry.last_heartbeat.is_ranked_server(),
				entry.official_server_id,
				entry.last_heartbeat.server_name
			);
		}
		else {
			entry.meta.webrtc_id = {};
		}
	};

	/*
		Official by its key (official_servers.d) or not: the id, the flag the clients get and the alias, always together.
	*/

	auto set_official = [&](masterserver_client& entry, const std::string* const official_id) {
		entry.official_server_id = official_id != nullptr ? *official_id : std::string();
		entry.meta.is_official = official_id != nullptr;
		update_webrtc_alias(entry);
	};

	/*
		The aliases as the list has them now, each to one server - two official servers with the same name get it
		in turn no more: the first keeps it and the clash is logged (once a minute at most).
	*/

	uint64_t last_logged_alias_clash = 0;

	auto rebuild_webrtc_aliases = [&]() {
		auto aliases = std::unordered_map<std::string, std::string>();

		for (const auto& server : server_list) {
			const auto& alias = server.second.meta.webrtc_id;

			if (!server.second.meta.is_official_server() || alias.empty()) {
				continue;
			}

			const auto address = ::ToString(server.first);

			if (const auto taken = aliases.try_emplace(std::string(alias), address); !taken.second && taken.first->second != address) {
				if (::official_log_allowed(last_logged_alias_clash)) {
					LOG("Two official servers want the alias %x (%x and %x) - check their names.", alias, taken.first->second, address);
				}
			}
		}

		signalling.set_webrtc_id_aliases(std::move(aliases));
	};

	auto reserialize_list = [&]() {
		// MSR_LOG("Reserializing the server list.");

		rebuild_webrtc_aliases();

		const auto web_servers = signalling.get_new_servers_map();

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

		for (auto& server : web_servers) {
			if (!server.second.last_heartbeat.show_on_server_list) {
				continue;
			}

			const auto ip_address = server.second.ip_informational;


			masterserver_entry_meta meta;
			meta.time_hosted = server.second.time_hosted;
			meta.type = server_type::WEB;
			meta.webrtc_id = webrtc_id_type(server.first);
			meta.server_public_key = server.second.server_public_key;

			augs::write_bytes(ss, ip_address);
			augs::write_bytes(ss, meta);
			augs::write_bytes(ss, server.second.last_heartbeat);

			if (!found_in(previous_web_servers, server.first)) {
				push_new_server_webhook(
					ip_address,
					server.second.last_heartbeat,
					true
				);
			}
		}

		previous_web_servers = web_servers;

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
			next.require_password = data.require_password;
			next.listed_players_only = data.listed_players_only;
			next.name = data.server_name;
			next.instance_label = data.instance_label;
			next.ip = ::ToString(ip);

			/*
				A tournament's live page, served by the server itself over TCP - at its own address, so it can only
				ever lead to that server.
			*/

			if (data.tournament_page_port != 0) {
				auto host = ip;
				host.port = data.tournament_page_port;
				next.tournament_page = "http://" + ::ToString(host) + "/";
			}
			next.webrtc_id = meta.webrtc_id;

			{
				const bool has_key = std::any_of(meta.server_public_key.begin(), meta.server_public_key.end(), [](const auto b) { return b != 0; });

				if (has_key) {
					next.server_public_key = augs::bytes_to_hex(meta.server_public_key);
				}
			}

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

		for (const auto& server : web_servers) {
			if (!server.second.last_heartbeat.show_on_server_list) {
				continue;
			}

			masterserver_entry_meta meta;
			meta.time_hosted = server.second.time_hosted;
			meta.type = server_type::WEB;
			meta.webrtc_id = webrtc_id_type(server.first);
			meta.server_public_key = server.second.server_public_key;
			
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

				augs::read_bytes(source, entry.meta);
				augs::read_bytes(source, entry.last_heartbeat);

				/*
					The list has the servers in browsers too - they come back through the signalling, not as native entries.
				*/

				if (entry.meta.type == server_type::WEB) {
					continue;
				}

				entry.time_last_heartbeat = current_time;

				/*
					Official again by its key at once (the registry is read already) - its alias stays. Its domain comes
					back with its next heartbeat, checked anew.
				*/

				set_official(entry, official_servers.find(entry.meta.server_public_key));
				entry.meta.domain = {};

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

	auto define_http_server = [&](auto& server) {
		/*
			/server_list - the list as the game reads it; /server_list_binary - the same, for games before 4.3.0.
		*/

		auto send_binary_list = [&](const Request&, Response& res) {
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
		};

		server.Get("/server_list", send_binary_list);
		server.Get("/server_list_binary", send_binary_list);

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

		for (const auto path : { "/server_list", "/server_list_binary" }) {
			server.Options(path, [](const httplib::Request&, httplib::Response& res) {
				set_cors(res);
				res.status = 204;
			});
		}

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
		/*
			Plain HTTP only for this machine - hypersomnia.io reads the list from here. Everyone else gets HTTPS.
		*/

		LOG("Hosting a FALLBACK HTTP server list at 127.0.0.1:%x (HTTP)", settings.fallback_http_server_list_port);

		fallback_listening_thread = std::thread([&fallback_http_ptr, in_settings=settings]() {
			fallback_http_ptr->listen("127.0.0.1", in_settings.fallback_http_server_list_port);
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
								Only the server itself takes itself off the list: signed with the key it is listed with,
								and with a fresh cookie for its address - so neither a spoofed nor a recorded goodbye works.
							*/

							const bool authentic = 
								::server_request_signature_valid(typed_request)
								&& typed_request.signature->server_public_key == entry->meta.server_public_key
								&& cookies.valid(typed_request.signature->cookie, from)
							;

							if (!authentic) {
								static uint64_t last_logged_ms = 0;

								if (::official_log_allowed(last_logged_ms)) {
									LOG("Ignoring an unauthenticated goodbye for the server at %x.", ::ToString(from));
								}

								return;
							}

							LOG("The server at %x (%x) has sent a goodbye.", ::ToString(from), entry->last_heartbeat.server_name);
							remove_from_list(from);
						}
					}
					else if constexpr(std::is_same_v<R, masterserver_in::webrtc_signalling_payload>) {
						/*
							Only from a server on the list - a spoofed source could otherwise make offers in a listed
							server's name.
						*/

						if (!found_in(server_list, from)) {
							return;
						}

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

							/*
								Signed by the key the server is listed with - see server_request_signature.h.
								Without a fresh cookie for this address the heartbeat changes nothing:
								it gets a cookie back, and the server sends again with it.
								The cheap cookie check goes first - the signature is verified only for a heartbeat that can change the list.
							*/

							if (!typed_request.signature.has_value()) {
								return;
							}

							const auto& public_key = typed_request.signature->server_public_key;
							const auto official_id = official_servers.find(public_key);

							auto ack = masterserver_out::heartbeat_ack();
							ack.cookie = cookies.make(from);
							ack.nonce = typed_request.ack_nonce;

							if (!cookies.valid(typed_request.signature->cookie, from)) {
								send_back(ack);
								return;
							}

							if (!::server_request_signature_valid(typed_request)) {
								static uint64_t last_logged_ms = 0;

								if (::official_log_allowed(last_logged_ms)) {
									LOG("Ignoring a badly signed heartbeat from %x.", ip_str);
								}

								/*
									Not accepted - the server says so in its log.
								*/

								ack.refusal = masterserver_out::heartbeat_refusal::BAD_SIGNATURE;
								send_back(ack);
								return;
							}

							/*
								Each instance of a host (one key) by its own label (see server_instance_label.h): one in a
								wrong format, or one another address of this key is listed with already (two processes
								on one host, or a server.key copied to another machine) is not listed - the older one
								stays, until its goodbye or its timeout.
							*/

							std::optional<netcode_address_t> moved_from;

							{
								const auto label = std::string(typed_request.instance_label);

								if (!::is_instance_label(label)) {
									ack.refusal = masterserver_out::heartbeat_refusal::BAD_INSTANCE_LABEL;
									send_back(ack);
									return;
								}

								/*
									An entry of this instance at another address silent for longer than a few
									heartbeats (one every 10 s by default) is this server's own old address (its address
									changed - IPv4 and IPv6, a NAT mapping, a restart on another port): replaced, not
									waited out until it times out (server_entry_timeout_secs).
								*/

								const auto moved_after_silence_secs = 35.0;

								for (const auto& other : server_list) {
									const bool same_instance =
										!(other.first == from)
										&& other.second.meta.server_public_key == public_key
										&& std::string(other.second.last_heartbeat.instance_label) == label
									;

									if (same_instance && current_time - other.second.time_last_heartbeat > moved_after_silence_secs) {
										moved_from = other.first;
										continue;
									}

									if (same_instance) {
										static uint64_t last_logged_ms = 0;

										if (::official_log_allowed(last_logged_ms)) {
											LOG("Ignoring a heartbeat from %x: its key's %x is listed at %x already.", ip_str, label, ::ToString(other.first));
										}

										ack.refusal = masterserver_out::heartbeat_refusal::INSTANCE_LABEL_TAKEN;
										ack.taken_by = other.first;
										send_back(ack);
										return;
									}
								}

							}

							/*
								See masterserver_settings: at most max_servers_per_address from one address (e.g. the instances of one host)
								and max_servers in all, so that nobody fills the list. The official servers are always listed.
							*/

							if (!found_in(server_list, from) && official_id == nullptr) {
								std::size_t from_this_ip = 0;

								for (const auto& other : server_list) {
									if (augs::network_of(other.first) == augs::network_of(from)) {
										++from_this_ip;
									}
								}

								if (from_this_ip >= settings.max_servers_per_address || server_list.size() >= settings.max_servers) {
									static uint64_t last_logged_ms = 0;

									if (::official_log_allowed(last_logged_ms)) {
										LOG("Ignoring a new server at %x: already %x servers from this address, or %x in all.", ip_str, settings.max_servers_per_address, settings.max_servers);
									}

									ack.refusal = masterserver_out::heartbeat_refusal::LIST_FULL;
									send_back(ack);
									return;
								}
							}

							/*
								Moved: hosted since the old entry was, which goes only once the new one is taken.
							*/

							const auto moved_time_hosted = [&]() -> std::optional<double> {
								if (!moved_from.has_value()) {
									return std::nullopt;
								}

								const auto old = server_list.find(*moved_from);

								if (old == server_list.end()) {
									return std::nullopt;
								}

								const auto hosted = old->second.meta.time_hosted;

								LOG(
									"The %x of key %x moved from %x to %x.",
									std::string(typed_request.instance_label),
									augs::bytes_to_hex(public_key).substr(0, 8),
									::ToString(*moved_from),
									ip_str
								);

								server_list.erase(old);
								return hosted;
							}();

							ack.accepted = true;
							send_back(ack);

							auto it = server_list.try_emplace(from);

							const bool is_new_server = it.second;
							auto& server_entry = (*it.first).second;

							const auto heartbeat_before = server_entry.last_heartbeat;
							const bool was_official = server_entry.meta.is_official;
							const auto key_before = server_entry.meta.server_public_key;

							server_entry.last_heartbeat = typed_request;
							server_entry.last_heartbeat.signature.reset();
							server_entry.last_heartbeat.ack_nonce = {};
							server_entry.time_last_heartbeat = current_time;

							/*
								Another key at the same address (e.g. a reinstalled server) simply replaces the entry.
							*/

							server_entry.meta.server_public_key = public_key;
							set_official(server_entry, official_id);

							{
								const auto domain = std::string(typed_request.domain);
								server_entry.meta.domain = ::is_acceptable_domain(domain, official_id != nullptr) ? typed_request.domain : decltype(typed_request.domain)();
							}

							const bool heartbeats_mismatch = 
								heartbeat_before != server_entry.last_heartbeat
								|| was_official != server_entry.meta.is_official
								|| key_before != server_entry.meta.server_public_key
							;

							if (is_new_server) {
								server_entry.meta.time_hosted = moved_time_hosted.value_or(current_time);

								if (moved_time_hosted.has_value()) {
									MSR_LOG("A listed server moved to %x - no notification.", ip_str);
								}
								else if (typed_request.show_on_server_list) {
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
					}
					else if constexpr(std::is_same_v<R, masterserver_in::tell_me_my_address>) {
						if (!address_answers.try_answer(from, augs::steady_secs())) {
							static uint64_t last_logged_ms = 0;

							if (::official_log_allowed(last_logged_ms)) {
								LOG("Too many tell_me_my_address requests from %x - ignoring some.", ::ToString(from));
							}

							return;
						}

						masterserver_out::tell_me_my_address response;
						response.address = from;
						response.session_timestamp = typed_request.session_timestamp;

						send_back(response);
					}
				};

				const auto request = augs::from_bytes<masterserver_request>(packet_buffer, packet_bytes);
				std::visit(handle, request);
			}
			/*
				A server is only ever removed by its signed goodbye or a timeout -
				anyone can send garbage with its address spoofed as the source.
			*/

			catch (const std::exception& err) {
				if (const auto entry = mapped_or_nullptr(server_list, from)) {
					LOG("The server at %x (%x) has sent invalid data: %x", ::ToString(from), entry->last_heartbeat.server_name, err.what());
				}
			}
			catch (...) {
				if (const auto entry = mapped_or_nullptr(server_list, from)) {
					LOG("The server at %x (%x) has sent invalid data: unknown error.", ::ToString(from), entry->last_heartbeat.server_name);
				}
			}
		};

		for (auto& s : udp_command_sockets) {
			::receive_netcode_packets<true>(s.socket, process_socket_messages);
		}

		const auto timeout_secs = settings.server_entry_timeout_secs;

		auto erase_if_dead = [&](auto& server_entry) {
			const bool timed_out = current_time - server_entry.second.time_last_heartbeat >= timeout_secs;

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

		if (keep_official_servers_up_to_date()) {
			/*
				A server whose key was removed is no longer official right away, instead of until it times out -
				and one whose key was just added is official right away.
			*/

			for (auto& server : server_list) {
				auto& entry = server.second;
				const auto official_id = official_servers.find(entry.meta.server_public_key);
				const bool now_official = official_id != nullptr;

				const auto now_id = now_official ? *official_id : std::string();

				if (entry.meta.is_official != now_official || entry.official_server_id != now_id) {
					LOG("The server at %x is %x.", ::ToString(server.first), now_official ? "official as " + now_id : std::string("no longer official"));

					set_official(entry, official_id);
				}
			}

			reserialize_list();
		}

		/*
			Only to servers on the list - otherwise anyone with a WebSocket could make the masterserver send packets anywhere.
		*/

		signalling.send_signalling_packets([&](const auto& payload, const netcode_address_t& target) {
			if (found_in(server_list, target)) {
				send_to_gameserver(payload, target);
			}
		});
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
