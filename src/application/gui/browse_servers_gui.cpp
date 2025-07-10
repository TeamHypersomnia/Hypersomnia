#include "augs/misc/future.h"

#include "application/gui/browse_servers_gui.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "3rdparty/include_httplib.h"
#include "augs/templates/thread_templates.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/setups/client/client_connect_string.h"
#include "augs/log.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "augs/misc/date_time.h"
#include "application/nat/nat_detection_settings.h"
#include "application/masterserver/masterserver_requests.h"
#include "application/masterserver/gameserver_command_readwrite.h"
#include "augs/misc/httplib_utils.h"
#include "augs/misc/to_hex_str.h"
#include "augs/string/typesafe_sscanf.h"
#include "hypersomnia_version.h"

#include "application/network/address_utils.h"
#include "augs/templates/main_thread_queue.h"
#include "augs/misc/async_get.h"

#include "augs/misc/profanity_filter.h"

#if PLATFORM_WEB
#include "application/gui/calculate_server_distance.hpp"
#endif

#if BUILD_NATIVE_SOCKETS
#include "augs/network/netcode_utils.h"
#include "augs/network/netcode_sockets.h"

constexpr auto ping_retry_interval = 1;
constexpr auto server_entry_timeout = 5;

constexpr auto reping_interval = 10;
constexpr auto max_packets_per_frame_v = 64;
#endif

#define LOG_BROWSER 1

template <class... Args>
void BRW_LOG(Args&&... args) {
#if LOG_BROWSER
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_MASTERSERVER
#define BRW_LOG_NVPS LOG_NVPS
#else
#define BRW_LOG_NVPS BRW_LOG
#endif

static uint32_t order_category(
	const server_list_entry& entry
) {
	const auto& a = entry.heartbeat;

	/*
		Order:

		0. Local servers
		1. Servers with players with no NAT (based on ping)
		2. Servers with players with NAT
		3. Empty servers with no NAT
		4. Empty servers with NAT
		5. Full servers with no NAT
		6. Full servers with NAT
	*/

	if (a.is_full()) {
		/* 
			Still consider it as there's always a chance it gets freed in the meantime
		*/

		if (a.nat.type == nat_type::PUBLIC_INTERNET) {
			return 1000;
		}

		return 10000;
	}

	if (entry.progress.found_on_internal_network) {
		return 0;
	}

	if (a.num_online_humans > 0 		&& a.nat.type == nat_type::PUBLIC_INTERNET) {
		/* Non-empty public, best category */
		return 1; 
	}
	else if (a.num_online_humans > 0) {
		/* Non-empty NAT */
		return 1 + static_cast<uint32_t>(a.nat.type); /* 2-5 */
	}
	else if (a.num_online_humans == 0 && a.nat.type == nat_type::PUBLIC_INTERNET) {
		/* Empty public */
		return 10 + static_cast<uint32_t>(a.nat.type); /* 10-14 */
	}
	else {
		/* Empty NAT */
		return 100 + static_cast<uint32_t>(a.nat.type); /* 100-104 */
	}
}

static bool compare_servers(
	const server_list_entry& a,
	const server_list_entry& b
) {
	{
		const auto ca = order_category(a);
		const auto cb = order_category(b);
		
		if (ca != cb) {
			/* Favor better category */
			return ca < cb;
		}
	}

	if (a.progress.ping != b.progress.ping) {
		/* Favor closer one */
		return a.progress.ping < b.progress.ping;
	}

	/* Favor more recent one */
	return a.meta.time_hosted > b.meta.time_hosted;
}

bool server_list_entry::is_set() const {
	return unlisted || heartbeat.server_name.size() > 0;
}

struct browse_servers_gui_internal {
	augs::async_response_ptr future_response;
	netcode_socket_t socket;

	bool refresh_op_in_progress() const {
		return augs::in_progress(future_response);
	}
};

browse_servers_gui_state::~browse_servers_gui_state() = default;

browse_servers_gui_state::browse_servers_gui_state(const std::string& title) 
	: base(title), data(std::make_unique<browse_servers_gui_internal>()) 
{

}

template <class T>
void update_cached_time_to_event(T& server_list) {
	main_thread_queue::execute(
		[&]() {
			for (auto& s : server_list) {
				s.heartbeat.cached_time_to_event = 
					s.heartbeat.is_ranked_server()
					? augs::date_time::get_secs_until_next_weekend_evening(s.heartbeat.get_location_id()) 
					: std::numeric_limits<double>::max()
				;
			}
		}
	);
}

static std::vector<server_list_entry> to_server_list(const augs::http_response& response, std::string& error_message) {
    using namespace httplib_utils;

    const auto status = response.status;

	if (status == -1) {
		error_message = "Couldn't connect to the server list host.";
		return {};
	}

    LOG("Server list response status: %x", status);

    if (!successful(status)) {
        const auto couldnt_download = std::string("Couldn't download the server list.\n");

        error_message = couldnt_download + "HTTP response: " + std::to_string(status);
        return {};
    }

    const auto& bytes = response.body;

    LOG("Server list response bytes: %x", bytes.size());

    auto stream = augs::make_ptr_read_stream(bytes.data(), bytes.size());

	std::vector<server_list_entry> new_server_list;

    try {
        while (stream.has_unread_bytes()) {
            server_list_entry entry;

			augs::read_bytes(stream, entry.address);
            augs::read_bytes(stream, entry.meta);
            augs::read_bytes(stream, entry.heartbeat);

            new_server_list.emplace_back(std::move(entry));
        }
    }
    catch (const augs::stream_read_error& err) {
        error_message = "There was a problem deserializing the server list:\n" + std::string(err.what()) + "\n\nTry restarting the game and updating your client!";
        new_server_list.clear();
    }

#if PLATFORM_WEB
	for (auto& n : new_server_list) {
		if (const auto ping = get_estimated_ping_to_server(n.heartbeat.get_location_id())) {
			n.progress.state = server_entry_state::PING_MEASURED;
			n.progress.ping = static_cast<uint32_t>(*ping);
		}

		if (augs::has_profanity(n.heartbeat.server_name)) {
			n.heartbeat.server_name = "(censored)";
		}
	}
#endif

    return new_server_list;
}

#if !WEB_SINGLETHREAD
void browse_servers_gui_state::open_matching_server_entry(
	const browse_servers_input in,
	const client_connect_string& server
) {
	sync_download_server_entry(in, server);

	if (const auto entry = find_entry_by_connect_string(server)) {
		open();

		select_server(*entry);
		server_details.open();

		return;
	}
	else {
		select_server(server_list_entry());

		if (const auto connected_address = ::find_netcode_addr(server)) {
			selected_server.address = *connected_address;
		}
		else {
			selected_server.meta.webrtc_id = server;
		}

		selected_server.unlisted = true;

		server_details.open();

		return;
	}
}

void browse_servers_gui_state::sync_download_server_entry(
	const browse_servers_input in,
	const client_connect_string& server
) {
	LOG("Calling sync_download_server_entry.");

	/*
		For now this will just refresh the whole list
		so find_entry later returns a valid entry.
	*/

	(void)server;
	(void)in;

	/* Todo: make async */
	auto lbd = 
		[address = in.server_list_provider]() -> std::optional<httplib_result> {
			/* 
				Right now it's the same as the one that downloads the whole list,
				but we'll need to support downloading just one entry.
			*/

			LOG("Connecting to server list at: %x", address);

			auto cli = httplib_utils::make_client(address);
			return cli->Get("/server_list_binary");
		}
	;

	auto result = lbd();

	if (result.has_value() && *result != nullptr) {
		server_list = ::to_server_list( { (*result)->status, (*result)->body } , error_message);
	}
}
#endif

bool browse_servers_gui_state::refreshed_at_least_once() const {
	return when_last_started_refreshing_server_list != 0;
}

bool browse_servers_gui_state::refresh_in_progress() const {
	return data->refresh_op_in_progress();
}

void browse_servers_gui_state::refresh_server_list(const browse_servers_input in) {
	using namespace httplib;

	if (refresh_in_progress()) {
		return;
	}

	error_message.clear();
	server_list.clear();
	auto prev_addr = selected_server.address;
	selected_server = {};
	selected_server.address = prev_addr;

	LOG("Launching future_response");

	data->future_response = augs::async_get(
		in.server_list_provider,
		"/server_list_binary"
	);

	LOG("refresh_server_list returns.");

	when_last_started_refreshing_server_list = augs::steady_secs();
}

void browse_servers_gui_state::refresh_server_pings() {
	for (auto& s : server_list) {
		s.progress = {};
	}
}

std::string get_steam_join_link(const std::string& addr);
std::string get_browser_join_link(const std::string& addr);

std::string get_steam_join_link(
	const masterserver_entry_meta& meta,
	const netcode_address_t& fallback_ip
) {
	const auto suffix = [&]() -> std::string {
		if (meta.type == server_type::WEB) {
			return meta.webrtc_id;
		}

		/* For native, prefer official, if not then fallback ip */
		if (!meta.official_url.empty()) {
			return meta.official_url;
		}

		return ::ToString(fallback_ip);
	}();

	return get_steam_join_link(suffix);
}

std::string get_browser_join_link(
	const masterserver_entry_meta& meta,
	const netcode_address_t& fallback_ip
) {
	const auto suffix = [&]() -> std::string {
		if (!meta.webrtc_id.empty()) {
			return meta.webrtc_id;
		}

		return ::ToString(fallback_ip);
	}();

	return get_browser_join_link(suffix);
}

std::string server_list_entry::get_connect_string() const {
	/*
		Cases:
		Web server will always have a WebRTC id.
		Native server can also have WebRTC id as a reserved alias - e.g. 'pl', 'us' etc. for the official servers.

		- Web connects to Web - can only use webrtc id.
		- Native connects to Web - can only use webrtc id.
		- Web connects to Native - prefer alias, then ip as webrtc id.
		- Native connects to Native - only ip. No point taking webrtc id.
	
	*/

	const auto& webrtc_id = meta.webrtc_id;

	if (meta.type == server_type::WEB) {
		return webrtc_id;
	}
#if PLATFORM_WEB
	else if (meta.type == server_type::NATIVE && !webrtc_id.empty()) {
		return webrtc_id;
	}
#endif

	return ::ToString(get_connect_address());
}

netcode_address_t server_list_entry::get_connect_address() const {
	if (progress.found_on_internal_network && heartbeat.internal_network_address.has_value()) {
		return *heartbeat.internal_network_address;
	}

	return address;
}

bool server_list_entry::is_behind_nat() const {
	return heartbeat.is_behind_nat();
}

server_list_entry* browse_servers_gui_state::find_entry(const netcode_address_t& address) {
	for (auto& s : server_list) {
		if (s.address == address) {
			return &s;
		}
	}

	return nullptr;
}

server_list_entry* browse_servers_gui_state::find_entry_by_internal_address(const netcode_address_t& address, const uint64_t ping_sequence) {
	for (auto& s : server_list) {
		if (s.progress.ping_sequence == ping_sequence) {
			if (s.heartbeat.internal_network_address == address) {
				return &s;
			}
		}
	}

	return nullptr;
}

bool is_internal(const netcode_address_t& address);

bool browse_servers_gui_state::handle_gameserver_response(const netcode_address_t& from, uint8_t* packet_buffer, std::size_t packet_bytes) {
	const auto current_time = augs::steady_secs();

	if (const auto maybe_sequence = read_ping_response(packet_buffer, packet_bytes)) {
		const auto sequence = *maybe_sequence;

		if (const auto entry = find_entry(from)) {
			BRW_LOG("Received a pingback (seq=%x) from an EXTERNAL game server at: %x", sequence, ::ToString(from));

			auto& progress = entry->progress;

			if (sequence == progress.ping_sequence) {
				BRW_LOG("Properly measured ping to %x: %x", ::ToString(from), progress.ping);

				progress.set_ping_from(current_time);
				progress.state = server_entry_state::PING_MEASURED;
			}
		}
		else if (const auto entry = find_entry_by_internal_address(from, sequence)) {
			auto& progress = entry->progress;

			BRW_LOG("Received a pingback (seq=%x) from an INTERNAL game server at: %x", sequence, ::ToString(from));

			progress.set_ping_from(current_time);
			progress.state = server_entry_state::PING_MEASURED;
			progress.found_on_internal_network = true;
		}
		else {
			BRW_LOG("WARNING! Received a pingback (seq=%x) from an UNKNOWN game server at: %x", sequence, ::ToString(from));
		}

		return true;
	}

	return false;
}

#if BUILD_NATIVE_SOCKETS
void browse_servers_gui_state::handle_incoming_udp_packets(netcode_socket_t& socket) {
	auto packet_handler = [&](const auto& from, uint8_t* buffer, const int bytes_received) {
		if (handle_gameserver_response(from, buffer, bytes_received)) {
			return;
		}
	};

	::receive_netcode_packets(socket, packet_handler);
}

void browse_servers_gui_state::animate_dot_column() {
	const auto current_time = augs::steady_secs();

	const auto num_dots = uint64_t(current_time * 3) % 3 + 1;
	loading_dots = std::string(num_dots, '.');
}

void browse_servers_gui_state::advance_ping_logic() {
	animate_dot_column();

	auto& udp_socket = server_browser_socket.socket;

	handle_incoming_udp_packets(udp_socket);
	send_pings_and_punch_requests(udp_socket);
}

void browse_servers_gui_state::send_pings_and_punch_requests(netcode_socket_t& socket) {
	const auto current_time = augs::steady_secs();

	auto interval_passed = [current_time](const auto last, const auto interval) {
		return current_time - last >= interval;
	};

	int packets_left = max_packets_per_frame_v;

	if (server_list.empty()) {
		return;
	}

	for (auto& s : server_list) {
		if (packets_left <= 0) {
			break;
		}

		if (s.meta.type == server_type::WEB) {
			continue;
		}

		auto& p = s.progress;

		using S = server_entry_state;

		auto request_ping = [&]() {
			p.when_sent_last_ping = current_time;
			p.ping_sequence = ping_sequence_counter++;

			ping_this_server(socket, s.address, p.ping_sequence);
			--packets_left;

			const auto& internal_address = s.heartbeat.internal_network_address;
			const bool maybe_reachable_internally = 
				internal_address.has_value()
				&& ::is_internal(*internal_address)
			;

			if (maybe_reachable_internally) {
				ping_this_server(socket, *internal_address, p.ping_sequence);
				--packets_left;
			}
		};

		if (p.state == S::AWAITING_RESPONSE) {
			auto& when_first_ping = p.when_sent_first_ping;
			auto& when_last_ping = p.when_sent_last_ping;

			if (try_fire_interval(ping_retry_interval, when_last_ping, current_time)) {
				if (when_first_ping < 0.0) {
					when_first_ping = current_time;
				}

				request_ping();
			}

			if (when_first_ping >= 0.0 && interval_passed(server_entry_timeout, when_first_ping)) {
				p.state = server_entry_state::GIVEN_UP;
				continue;
			}
		}

		if (p.state == S::PING_MEASURED) {
			auto& when_last_ping = p.when_sent_last_ping;

			auto reping_if_its_time = [&]() {
				if (!show) {
					/* Never reping if the window is hidden. */
					return;
				}

				if (interval_passed(reping_interval, when_last_ping)) {
					request_ping();
				}
			};

			reping_if_its_time();
		}
	}
}
#endif

constexpr auto num_columns = 7;

void browse_servers_gui_state::select_server(const server_list_entry& s) {
	selected_server = s;
	//server_details.open();
	scroll_once_to_selected = true;
}

void browse_servers_gui_state::show_server_list(
	const std::string& label,
	const std::vector<server_list_entry*>& server_list,
	const faction_view_settings& faction_view,
	const bool streamer_mode
) {
	using namespace augs::imgui;

	const auto& style = ImGui::GetStyle();

	if (server_list.empty()) {
		ImGui::NextColumn();
		text_disabled(typesafe_sprintf("No %x servers match the applied filters.", label));
		next_columns(num_columns - 1);
		return;
	}

	for (const auto& sp : server_list) {
		auto id = scoped_id(index_in(server_list, sp));
		const auto& s = *sp;
		const auto& progress = s.progress;
		const auto& d = s.heartbeat;

		const bool given_up = progress.state == server_entry_state::GIVEN_UP;
		const auto color = rgba(given_up || d.is_full() ? style.Colors[ImGuiCol_TextDisabled] : style.Colors[ImGuiCol_Text]);

		auto do_text = [&](const auto& t) {
			text_color(t, color);
		};

		const bool is_selected = selected_server.address == s.address;

		if (sp->meta.type == server_type::WEB) {
			do_text("Web");
		}
		else {
			if (progress.responding()) {
				const auto ping = progress.ping;

				if (ping > 999) {
					do_text("999>");
				}
				else {
#if PLATFORM_WEB
					do_text(typesafe_sprintf("~%x", ping));
#else
					do_text(typesafe_sprintf("%x", ping));
#endif
				}
			}
			else {
				if (given_up) {
					do_text("?");
				}
				else {
					do_text(loading_dots);
				}
			}
		}

		ImGui::NextColumn();

		const auto wave_time = augs::steady_secs() / 4.0;
		const auto wave_color = rgba::get_bright_wave(wave_time, 0.55);

		{
			if (s.is_official_server() ) {
				if (d.is_ranked_server()) {
					if (d.is_full()) {
						text_color("RANKED", color);
					}
					else {
						text_color("RANKED", wave_color);
					}

					ImGui::SameLine();
				}
				else {
					text_disabled("CASUAL");
					ImGui::SameLine();
				}
			}

			auto col_scope = scoped_style_color(ImGuiCol_Text, color);

			auto displayed_name = d.server_name;

			if (streamer_mode && s.is_community_server()) {
				displayed_name = "Community server";
			}

			if (is_selected) {
				if (scroll_once_to_selected) {
					scroll_once_to_selected = false;

					ImGui::SetScrollHereY(0.0f);
				}
			}

			if (ImGui::Selectable(displayed_name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
				selected_server = s;
			}
		}

		if (ImGui::IsItemHovered()) {
			auto tool = scoped_tooltip();

			if (d.num_online == 0) {
				text_disabled("No players online.");
			}
			else {
				text_color(typesafe_sprintf("%x players online:", d.num_online), green);

				auto do_faction_summary = [&](auto faction, auto& entries) {
					if (entries.empty()) {
						return;
					}

					ImGui::Separator();

					const auto labcol = faction_view.colors[faction].current_player_text;

					for (auto& e : entries) {
						auto nickname = e.nickname;

						if (streamer_mode || augs::has_profanity(nickname)) {
							nickname = "Player";
						}

						text_color(nickname, labcol);
					}
				};

				do_faction_summary(faction_type::RESISTANCE, d.players_resistance);
				do_faction_summary(faction_type::METROPOLIS, d.players_metropolis);
				do_faction_summary(faction_type::SPECTATOR, d.players_spectating);
			}
		}
		
		if (ImGui::IsItemClicked()) {
			if (ImGui::IsMouseDoubleClicked(0)) {
				LOG("Double-clicked server list entry: %x (%x). Connecting.", d.server_name, ToString(s.address));

				displayed_connecting_server_name = d.server_name;

				requested_connection = s.get_connect_string();
			}
		}

		if (ImGui::IsItemClicked(1)) {
			selected_server = s;
			server_details.open();
		}

		ImGui::NextColumn();

		const auto players_text = typesafe_sprintf("%x/%x", d.num_online, d.max_online());

		if (d.any_human() == 0) {
			text_disabled(players_text);
		}
		else {
			do_text(players_text);
		}

		ImGui::NextColumn();

		if (s.is_official_server() ) {
			if (d.is_ranked_server()) {
				const auto location_id = d.get_location_id();

#if PLATFORM_WEB
				if (const auto result = augs::date_time::format_time_until_weekend_evening(d.cached_time_to_event)) {
#else
				if (const auto result = augs::date_time::format_time_until_weekend_evening(location_id)) {
#endif
					if (const bool ongoing = *result == "") {
						text_color("2X EXP", wave_color);
					}
					else {
						text_disabled(*result);
					}
				}
			}
			else {
				text_disabled("None");
			}
		}

		ImGui::NextColumn();

		{
			auto displayed_arena = std::string(d.current_arena);

			if (streamer_mode && s.is_community_server()) {
				displayed_arena = "Community arena";
			}

			do_text(displayed_arena);
		}

		ImGui::NextColumn();

		if (d.game_mode.size() > 0) {
			auto displayed_mode = std::string(d.game_mode);

			if (streamer_mode && s.is_community_server()) {
				displayed_mode = "Game mode";
			}

			do_text(displayed_mode);
		}
		else {
			do_text("<unknown>");
		}

		ImGui::NextColumn();

		const auto secs_ago = augs::date_time::secs_since_epoch() - s.meta.time_hosted;
		text_disabled(augs::date_time::format_countdown_letters(secs_ago));

		ImGui::NextColumn();
	}

	scroll_once_to_selected = false;
}

bool browse_servers_gui_state::perform(const browse_servers_input in) {
	using namespace httplib_utils;

	if (augs::is_ready(data->future_response)) {
		server_list = ::to_server_list(augs::get_once(data->future_response), error_message);
		when_last_updated_time_to_events = 0;
	}

	if (selected_server.unlisted) {
		server_details.perform(selected_server, in.faction_view, in.streamer_mode);
	}

	if (!show) {
		return false;
	}

	using namespace augs::imgui;

	{
		const auto current_time = augs::steady_secs();

		if (current_time - when_last_updated_time_to_events > 1.0) {
			when_last_updated_time_to_events = current_time;

			update_cached_time_to_event(server_list);
		}
	}

	center_flag = ImGuiCond_Always;

	if (ImGui::GetIO().DisplaySize.x <= 1200) {
		centered_size_mult = vec2(0.97f, 0.96f);
	}
	else {
		centered_size_mult = vec2(0.85f, 0.8f);
	}

	auto imgui_window = make_scoped_window(ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	if (!imgui_window) {
		return false;
	}

	thread_local ImGuiTextFilter filter;

	filter_with_hint(filter, "##HierarchyFilter", "Filter server names/arenas...");
	local_server_list.clear();
	official_server_list.clear();
	community_server_list.clear();

	bool has_local_servers = false;
	bool has_official_servers = false;
	bool has_community_servers = false;

	requested_connection = std::nullopt;

	for (auto& s : server_list) {
		const auto& name = s.heartbeat.server_name;
		const auto& arena = s.heartbeat.current_arena;
		const auto effective_name = std::string(name) + " " + std::string(arena);

		const bool is_internal = s.progress.found_on_internal_network;

		auto push_if_passes = [&](auto& target_list) {
			const bool is_internal = s.progress.found_on_internal_network;

			if (!is_internal) {
				if (!allow_ranked_servers && s.heartbeat.is_ranked_server()) {
					return;
				}
			}

			if (only_responding) {
				if (!s.progress.responding()) {
					return;
				}
			}

			if (at_least_players.is_enabled) {
				if (s.heartbeat.num_online_humans < at_least_players.value) {
					return;
				}
			}

			if (at_most_ping.is_enabled) {
				if (s.progress.ping > at_most_ping.value) {
					return;
				}
			}

			if (!show_incompatible) {
				const auto client_version = hypersomnia_version().get_version_string();
				const auto server_version = s.heartbeat.server_version;

				int client_major = 0;
				int client_minor = 0;
				int client_patch = 0;
				int server_major = 0;
				int server_minor = 0;
				int server_patch = 0;

				if (
					typesafe_sscanf(client_version, "%x.%x.%x", client_major, client_minor, client_patch) &&
					typesafe_sscanf(server_version, "%x.%x.%x", server_major, server_minor, server_patch)
				) {
					if (client_major != server_major || client_minor != server_minor) {
						return;
					}
				}
			}

			if (!filter.PassFilter(effective_name.c_str())) {
				return;
			}

			target_list.push_back(&s);
		};

		if (is_internal) {
			has_local_servers = true;
			push_if_passes(local_server_list);
		}
		else {
			if (const bool is_official = s.is_official_server()) {
				has_official_servers = true;
				push_if_passes(official_server_list);
			}
			else {
				has_community_servers = true;
				push_if_passes(community_server_list);
			}
		}
	}

	{
		using T = const server_list_entry*;

		auto by_ping = [](const T& a, const T& b) {
			return a->progress.ping < b->progress.ping;
		};

		auto by_name = [](const T& a, const T& b) {
			return a->heartbeat.server_name < b->heartbeat.server_name;
		};

		auto by_mode = [](const T& a, const T& b) {
			return a->heartbeat.game_mode < b->heartbeat.game_mode;
		};

		auto by_time_to_event = [](const T& a, const T& b) {
			return a->heartbeat.cached_time_to_event < b->heartbeat.cached_time_to_event;
		};

		auto by_players = [](const T& a, const T& b) {
			if (a->heartbeat.num_online_humans == b->heartbeat.num_online_humans) {
				return a->heartbeat.num_online < b->heartbeat.num_online;
			}

			return a->heartbeat.num_online_humans < b->heartbeat.num_online_humans;
		};

		auto by_arena = [](const T& a, const T& b) {
			return a->heartbeat.current_arena < b->heartbeat.current_arena;
		};

		auto by_appeared = [](const T& a, const T& b) {
			return a->meta.time_hosted > b->meta.time_hosted;
		};

		auto make_comparator = [&](auto op1, auto op2, auto op3) {
			bool asc = ascending;

			return [asc, op1, op2, op3](const T& a, const T& b) {
				const auto a_less_b = op1(a, b);
				const auto b_less_a = op1(b, a);

				if (!a_less_b && !b_less_a) {
					const auto _2_a_less_b = op2(a, b);
					const auto _2_b_less_a = op2(b, a);

					if (!_2_a_less_b && !_2_b_less_a) {
						return op3(a, b);
					}

					return _2_a_less_b;
				}
				
				return asc ? a_less_b : b_less_a;
			};
		};

		auto sort_entries = [&]() {
			const auto c = sort_by_column;

			auto sort_by = [&](auto... cmps) {
				sort_range(local_server_list, make_comparator(cmps...));
				sort_range(official_server_list, make_comparator(cmps...));
				sort_range(community_server_list, make_comparator(cmps...));
			};

			switch (c) {
				case 0: 
					return sort_by(by_ping, by_appeared, by_name);

				case 1: 
					return sort_by(by_name, by_ping, by_appeared);

				case 2: 
					return sort_by(by_players, by_ping, by_appeared);

				case 3: 
					return sort_by(by_time_to_event, by_ping, by_appeared);

				case 4: 
					return sort_by(by_arena, by_ping, by_appeared);

				case 5: 
					return sort_by(by_mode, by_ping, by_appeared);

				case 6: 
					return sort_by(by_appeared, by_ping, by_players);
					
				default:
					return sort_by(by_ping, by_appeared, by_name);
			}
		};

		sort_entries();
	}

	ImGui::Separator();

	if (when_last_started_refreshing_server_list == 0) {
		refresh_server_list(in);
	}

	auto do_list_view = [&](bool disable_content_view) {
		const auto max_sname = 35;

		const auto avail_x = ImGui::GetContentRegionAvail().x;

		float col_1 = ImGui::CalcTextSize("9999999").x;
		float col_3 = ImGui::CalcTextSize("99h 99m 9").x;
		float col_4 = ImGui::CalcTextSize("Players  9").x;
		float col_5 = ImGui::CalcTextSize("9").x * (max_arena_name_length_v / 2);
		float col_6 = ImGui::CalcTextSize("Capture the flag 9").x;
		float col_7 = ImGui::CalcTextSize("9").x * 25;

		float total_width = col_1 + col_3 + col_4 + col_5 + col_6 + col_7;

		float col_2 = std::max(ImGui::CalcTextSize("9").x * (max_sname - 5) - 4, avail_x - total_width);
		total_width += col_2;

		auto child = scoped_child("list view", ImVec2(total_width, 2 * -(ImGui::GetFrameHeightWithSpacing() + 4)), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (disable_content_view) {
			return;
		}

		ImGui::Columns(num_columns);

		ImGui::SetColumnWidth(0, col_1);
		ImGui::SetColumnWidth(1, col_2);
		ImGui::SetColumnWidth(2, col_3);
		ImGui::SetColumnWidth(3, col_4);
		ImGui::SetColumnWidth(4, col_5);
		ImGui::SetColumnWidth(5, col_6);
		ImGui::SetColumnWidth(6, col_7);

		int current_col = 0;

		auto do_column = [&](std::string label, std::optional<rgba> col = std::nullopt) {
			const bool is_current = current_col == sort_by_column;

			if (is_current) {
				label += ascending ? "▲" : "▼";
			}

			const auto& style = ImGui::GetStyle();

			auto final_color = rgba(is_current ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled]);

			if (col.has_value()) {
				final_color = *col;
			}

			auto col_scope = scoped_style_color(ImGuiCol_Text, final_color);

			if (ImGui::Selectable(label.c_str())) {
				if (is_current) {
					ascending = !ascending;
				}
				else 
				{
					sort_by_column = current_col;
					ascending = true;
				}
			}

			++current_col;
			ImGui::NextColumn();
		};

		auto do_column_labels = [&](const auto& with_label, const auto& col) {
			do_column("Ping");
			do_column(with_label, col);
			do_column("Players");
			do_column("Event");
			do_column("Arena");
			do_column("Game mode");
			do_column("Uptime");

			ImGui::Separator();
		};

		const auto num_locals = local_server_list.size();
		const auto num_officials = official_server_list.size();
		const auto num_communities = community_server_list.size();

		const auto local_servers_label = typesafe_sprintf("Local servers (%x)", num_locals);
		const auto official_servers_label = typesafe_sprintf("Official servers (%x)", num_officials);
		const auto community_servers_label = typesafe_sprintf("Community servers (%x)", num_communities);

		auto separate_with_label_only = [&](const auto& label, const auto& color) {
			text_disabled("\n\n");

			ImGui::Separator();
			ImGui::NextColumn();
			text_color(label, color);
			next_columns(num_columns - 1);
			ImGui::Separator();
		};

		if (has_local_servers) {
			do_column_labels(local_servers_label, green);
			show_server_list("local", local_server_list, in.faction_view, in.streamer_mode);
		}

		if (!has_local_servers) {
			do_column_labels(official_servers_label, yellow);
		}
		else {
			separate_with_label_only(official_servers_label, yellow);
		}

		if (has_official_servers) {
			show_server_list("official", official_server_list, in.faction_view, in.streamer_mode);
		}
		else {
			ImGui::NextColumn();
			text_disabled("No official servers are online at the time.");
			next_columns(num_columns - 1);
		}

		separate_with_label_only(community_servers_label, orange);

		if (has_community_servers) {
			show_server_list("community", community_server_list, in.faction_view, in.streamer_mode);
		}
		else {
			ImGui::NextColumn();
			text_disabled("No community servers are online at the time.");
			next_columns(num_columns - 1);
		}
	};

	{
		bool disable_content_view = false;

		if (error_message.size() > 0) {
			text_color(error_message, red);
			disable_content_view = true;
		}

		if (refresh_in_progress()) {
			text_disabled("Downloading the server list...");
			disable_content_view = true;
		}

		do_list_view(disable_content_view);
	}

	{
		auto child = scoped_child("buttons view");

		ImGui::Separator();

		checkbox("Only responding", only_responding);
		ImGui::SameLine();

		checkbox("Show incompatible", show_incompatible);
		ImGui::SameLine();

		checkbox("At least", at_least_players.is_enabled);

		{
			auto& val = at_least_players.value;
			auto scoped = maybe_disabled_cols({}, !at_least_players.is_enabled);
			auto width = scoped_item_width(ImGui::CalcTextSize("9").x * 4);

			ImGui::SameLine();
			auto input = std::to_string(val);
			input_text(val == 1 ? "human player###playernumbox" : "human players###playernumbox", input);
			val = std::clamp(std::atoi(input.c_str()), 1, int(max_incoming_connections_v));
		}


		ImGui::SameLine();

		checkbox("At most", at_most_ping.is_enabled);
		
		{
			auto& val = at_most_ping.value;
			auto scoped = maybe_disabled_cols({}, !at_most_ping.is_enabled);
			auto width = scoped_item_width(ImGui::CalcTextSize("9").x * 5);

			ImGui::SameLine();
			auto input = std::to_string(val);
			input_text(val == 1 ? "ping###pingnumbox" : "ping###pingnumbox", input);
			val = std::clamp(std::atoi(input.c_str()), 0, int(999));
		}

		{
			auto scope = maybe_disabled_cols({}, !selected_server.is_set());

			if (ImGui::Button("Connect")) {
				auto& s = selected_server;

				LOG("Chosen server list entry: %x (%x). Connecting.", ToString(s.address), s.heartbeat.server_name);

				displayed_connecting_server_name = s.heartbeat.server_name;

				requested_connection = s.get_connect_string();
			}

			ImGui::SameLine();

			if (ImGui::Button("Server details")) {
				server_details.open();
			}

			ImGui::SameLine();
		}

		ImGui::SameLine();

		{
			auto scope = maybe_disabled_cols({}, refresh_in_progress());

			if (ImGui::Button("Refresh ping")) {
				refresh_server_pings();
			}

			ImGui::SameLine();

			if (ImGui::Button("Refresh list")) {
				refresh_server_list(in);
			}
		}
	}
	
	if (server_details.show) {
		if (!selected_server.is_set()) {
			for (auto& s : server_list) {
				if (s.address == selected_server.address) {
					selected_server = s;
				}
			}
		}
	}

	if (const bool listed = !selected_server.unlisted) {
		if (server_details.perform(selected_server, in.faction_view, in.streamer_mode)) {
			refresh_server_list(in);
		}
	}

	if (requested_connection.has_value()) {
		in.client_connect = *requested_connection;
		in.displayed_connecting_server_name = displayed_connecting_server_name;

		return true;
	}

	return false;
}

std::optional<server_list_entry> browse_servers_gui_state::find_best_server(const bool find_ranked) const {
	auto filtered = server_list;

	erase_if(
		filtered,
		[&](auto& f) {
			if (const bool casual = !find_ranked) {
				const bool is_ranked = f.is_official_server() && f.heartbeat.is_ranked_server();

				return is_ranked;
			}

			const bool good = f.is_official_server() && f.heartbeat.is_still_joinable_ranked();

			return !good;
		}
	);

	if (filtered.empty()) {
		return std::nullopt;
	}

	return minimum_of(filtered, compare_servers);
}

const server_list_entry* browse_servers_gui_state::find_entry_by_connect_string(const client_connect_string& in) const {
	LOG("Finding the server entry by: %x", in);
	LOG("Number of servers in the browser: %x", server_list.size());

	if (const auto connected_address = ::find_netcode_addr(in)) {
		LOG("%x is an IP address.", in);

		for (auto& s : server_list) {
			if (s.address == *connected_address) {
				return &s;
			}
		}

		if (is_internal(*connected_address)) {
			for (auto& s : server_list) {
				if (s.heartbeat.internal_network_address == *connected_address) {
					return &s;
				}
			}
		}
	}
	else {
		LOG("%x is either a webrtc id or an official domain address.", in);

		for (auto& s : server_list) {
			if (s.meta.webrtc_id == in) {
				return &s;
			}

			if (s.meta.official_url == in) {
				return &s;
			}
		}
	}

	return nullptr;
}

void server_details_gui_state::perform_online_players(
	const server_list_entry& entry,
	const faction_view_settings& faction_view,
	const bool streamer_mode
) {
	using namespace augs::imgui;

	auto heartbeat = entry.heartbeat;

	//input_text("Server version", heartbeat.server_version, ImGuiInputTextFlags_ReadOnly);
	if (heartbeat.num_online == 0) {
		text_disabled("No players online.");
		return;
	}

#if 0
	ImGui::Separator();
	text("Faction scores");
	ImGui::Separator();

	auto do_faction_score = [&](const auto faction, const auto score) {
		const auto labcol = faction_view.colors[faction].current_player_text;

		text_color(typesafe_sprintf("%x: %x", format_enum(faction), score), labcol);
	};

	do_faction_score(faction_type::RESISTANCE, heartbeat.score_resistance);
	do_faction_score(faction_type::METROPOLIS, heartbeat.score_metropolis);
#endif

	ImGui::Separator();

	auto longest_nname = std::size_t(0);

	auto get_nickname = [&](const auto& e) -> std::string {
		if (streamer_mode) {
			return "Player";
		}

		return e.nickname;
	};

	for (auto& e : heartbeat.players_resistance) { 
		longest_nname = std::max(longest_nname, get_nickname(e).length());
	}
	for (auto& e : heartbeat.players_metropolis) { 
		longest_nname = std::max(longest_nname, get_nickname(e).length());
	}
	for (auto& e : heartbeat.players_spectating) { 
		longest_nname = std::max(longest_nname, get_nickname(e).length());
	}

	text_color(typesafe_sprintf("Players online: %x", heartbeat.num_online), green);

	ImGui::Columns(3);
	//ImGui::SetColumnWidth(0, ImGui::CalcTextSize("9999999").x);
	ImGui::SetColumnWidth(0, ImGui::CalcTextSize("9").x * (longest_nname + 5));
	ImGui::SetColumnWidth(1, ImGui::CalcTextSize("Score99").x);
	ImGui::SetColumnWidth(2, ImGui::CalcTextSize("Deaths90").x);

#if 0
	text_disabled("Nickname");
	ImGui::NextColumn();

	//text_disabled("Score");
	ImGui::NextColumn();

	//text_disabled("Deaths");
	ImGui::NextColumn();
#endif

	bool once = true;

	auto do_faction = [&](const auto faction, const auto& entries, const auto score) {
		(void)score;
		if (entries.empty()) {
			return;
		}

		const auto labcol = faction_view.colors[faction].current_player_text;
		const auto col = faction_view.colors[faction].current_player_text;
		(void)labcol;

		ImGui::Separator();

		if (faction == faction_type::SPECTATOR) {
			text_disabled("Spectators");

			ImGui::NextColumn();
			ImGui::NextColumn();
			ImGui::NextColumn();

			ImGui::Separator();
		}
		else {
			text_disabled(typesafe_sprintf("%x", format_enum(faction)));

			ImGui::NextColumn();
			if (once) {
				text_disabled("Score");
			}
			//text_disabled(typesafe_sprintf("%x", score));

			ImGui::NextColumn();
			if (once) {
				text_disabled("Deaths");
			}
			ImGui::NextColumn();

			ImGui::Separator();

			once = false;
		}


		for (auto& e : entries) {
			text_color(get_nickname(e), col);
			ImGui::NextColumn();

			if (faction != faction_type::SPECTATOR) {
				text(std::to_string(e.score));
			}

			ImGui::NextColumn();

			if (faction != faction_type::SPECTATOR) {
				text(std::to_string(e.deaths));
			}

			ImGui::NextColumn();
		}
	};

	do_faction(faction_type::RESISTANCE, heartbeat.players_resistance, heartbeat.score_resistance);
	do_faction(faction_type::METROPOLIS, heartbeat.players_metropolis, heartbeat.score_metropolis);
	do_faction(faction_type::SPECTATOR, heartbeat.players_spectating, 0);
}

bool server_details_gui_state::perform(
	const server_list_entry& entry,
	const faction_view_settings& faction_view,
	const bool streamer_mode
) {
	using namespace augs::imgui;
	//ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiCond_FirstUseEver);
	auto window = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);

	const auto rdonly_flags = ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll;

	if (!window) {
		return false;
	}

	if (!entry.is_set()) {
		text_disabled("No server selected.");
		return false;
	}

	const bool listed = !entry.unlisted;

	auto steam_link = ::get_steam_join_link(entry.meta, entry.address);

	auto sid = scoped_id(steam_link.c_str());

	if (listed) {
		if (ImGui::Button("Refresh")) {
			return true;
		}
	}

	ImGui::Separator();

	auto heartbeat = entry.heartbeat;
	const auto& internal_addr = heartbeat.internal_network_address;

	auto external_address = ::ToString(entry.address);
	auto internal_address = internal_addr ? ::ToString(*internal_addr) : std::string("Unknown yet");

	text_disabled("Join:");

	auto do_steam_join_link = [&](bool acquire) {
		auto link = steam_link;

		if (acquire) {
			acquire_keyboard_once();
		}

		input_text("##Steam join link", link, rdonly_flags);

		ImGui::SameLine();

		if (ImGui::Button("Copy##SteJoin")) {
			ImGui::SetClipboardText(link.c_str());
		}

		ImGui::SameLine();

		text_color("Steam join link", green);
	};

	auto do_browser_join_link = [&](bool acquire) {
		auto link = ::get_browser_join_link(entry.meta, entry.address);

		if (acquire) {
			acquire_keyboard_once();
		}

		input_text("##Browser join link", link, rdonly_flags);

		ImGui::SameLine();

		if (ImGui::Button("Copy##BroJoin")) {
			ImGui::SetClipboardText(link.c_str());
		}

		ImGui::SameLine();
		text_color("Browser join link", green);
	};

	if (listed) {
		do_browser_join_link(true);
		do_steam_join_link(false);
	}
	else {
		/*
			Browser can't get to an unlisted server
			as it requires a webrtc handshake with the server list as the middleman.
		*/

		do_steam_join_link(true);
	}

	text_disabled("Details:");

	ImGui::Separator();

	bool do_show_ips = true;

	if (streamer_mode) {
		checkbox("Show IPs", show_ips);
		do_show_ips = show_ips;
	}

	if (!do_show_ips) {
		if (ImGui::Button("Copy External IP address to clipboard")) {
			ImGui::SetClipboardText(external_address.c_str());
		}

		if (ImGui::Button("Copy Internal IP address to clipboard")) {
			ImGui::SetClipboardText(internal_address.c_str());
		}
	}
	else {
		input_text("##IP address", external_address, rdonly_flags);

		ImGui::SameLine();

		if (ImGui::Button("Copy##ExtIp")) {
			ImGui::SetClipboardText(external_address.c_str());
		}

		ImGui::SameLine();

		text("IP address");

		if (external_address != internal_address && heartbeat.internal_network_address.has_value()) {
			input_text("##IP address (internal)", internal_address, rdonly_flags);

			ImGui::SameLine();

			if (ImGui::Button("Copy##IntIp")) {
				ImGui::SetClipboardText(internal_address.c_str());
			}

			ImGui::SameLine();

			text("IP address (internal)");
		}
	}

	auto official_url = entry.meta.official_url;

	if (!official_url.empty()) {
		input_text("##Official Domain", official_url, rdonly_flags);

		ImGui::SameLine();

		if (ImGui::Button("Copy##OffDom")) {
			ImGui::SetClipboardText(official_url.c_str());
		}

		ImGui::SameLine();

		text("Official Domain");
	}

	{
		auto webrtc_id = entry.meta.webrtc_id;

		if (!webrtc_id.empty()) {
			input_text("##WebRTC id", webrtc_id, rdonly_flags);

			ImGui::SameLine();

			if (ImGui::Button("Copy##WebId")) {
				ImGui::SetClipboardText(webrtc_id.c_str());
			}

			ImGui::SameLine();

			text("WebRTC id");
		}
	}

	if (!listed) {
		ImGui::Separator();
		text_color("Server is unlisted.\nLimited information available.", orange);

		return false;
	}

	auto name = heartbeat.server_name;
	auto arena = heartbeat.current_arena;
	auto version = heartbeat.server_version;

	if (streamer_mode && entry.is_community_server()) {
		name = "Community server";
		arena = "Community arena";
	}

	input_text("Server name", name, rdonly_flags);

	input_text("Arena", arena, rdonly_flags);

	auto nat = nat_type_to_string(heartbeat.nat.type); 

	input_text("NAT type", nat, rdonly_flags);
	auto delta = std::to_string(heartbeat.nat.port_delta);

	if (heartbeat.nat.type != nat_type::PUBLIC_INTERNET) {
		input_text("Port delta", delta, rdonly_flags);
	}

	input_text("Server version", version, rdonly_flags);

	perform_online_players(entry, faction_view, streamer_mode);
	return false;
}

void browse_servers_gui_state::reping_all_servers() {
	for (auto& s : server_list) {
		s.progress = {};
	}
}

