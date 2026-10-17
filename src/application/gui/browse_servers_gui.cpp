#include <cstddef>
#include <cstdint>
#include "augs/string/hex.h"
#include "augs/misc/future.h"
#include "view/format_multiplier.h"

#include "application/setups/client/connect_target.h"
#include "application/main/favorite_servers.h"
#include "application/gui/browse_servers_gui.h"
#include "augs/misc/imgui/simple_popup.h"
#include "augs/misc/imgui/long_input_text_with_dots.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_status_icons.h"
#include "3rdparty/include_httplib.h"
#include "augs/templates/thread_templates.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/setups/client/client_connect_string.h"
#include "augs/log.h"
#include "augs/misc/imgui/maybe_different.h"
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
#include "augs/filesystem/file.h"
#include "application/gui/pretty_tabs.h"

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
				const auto at = s.heartbeat.next_event_at;

				s.heartbeat.cached_time_to_event =
					at < 0.0 ? std::numeric_limits<double>::max() :
					at == 0.0 ? 0.0 :
					std::max(1.0, at - augs::steady_secs())
				;
			}
		}
	);
}

static std::vector<server_list_entry> to_server_list(const augs::http_response& response, std::string& error_message) {
    using namespace httplib_utils;

    const auto status = response.status;

	/*
		The web build gets 0 for anything that did not arrive (offline, CORS).
	*/

	if (status <= 0) {
#if PLATFORM_WEB
		/*
			The browser joins through the same host's signalling - by address too.
		*/

		error_message = "Couldn't connect to the server list host.";
#else
		error_message = "Couldn't connect to the server list host.\nYou can still join a server by its address (Connect to server).";
#endif
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

			{
				const auto secs = entry.heartbeat.secs_until_next_event;
				entry.heartbeat.next_event_at = secs < 0 ? -1.0 : secs == 0 ? 0.0 : augs::steady_secs() + secs;
			}

            new_server_list.emplace_back(std::move(entry));
        }
    }
    catch (const augs::stream_read_error& err) {
        error_message = "There was a problem deserializing the server list:\n" + std::string(err.what()) + "\n\nTry restarting the game and updating your client!";
        new_server_list.clear();
    }

#if PLATFORM_WEB
	for (auto& n : new_server_list) {
		if (const auto ping = get_estimated_ping_to_server(std::string(n.heartbeat.time_zone))) {
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

/*
	The cached list: the bytes of /server_list_binary as they came.
*/

static void save_cached_list(const augs::path_type& path, const std::string& bytes) {
	try {
		augs::create_directories_for(path);

		auto tmp = path;
		tmp += ".tmp";

		{
			auto out = augs::open_binary_output_stream(tmp);
			out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		}

		std::filesystem::rename(tmp, path);
	}
	catch (const std::exception& e) {
		LOG("Could not cache the server list at %x: %x", path, e.what());
	}
}


bool browse_servers_gui_state::refreshed_at_least_once() const {
	return when_last_started_refreshing_server_list != 0;
}

void browse_servers_gui_state::ask_password(const server_list_entry& entry) {
	pending_password_entry = entry;
	password_prompt_input.clear();
}

std::optional<std::pair<std::string, server_list_entry>> browse_servers_gui_state::perform_password_prompt() {
	using namespace augs::imgui;

	auto result = std::optional<std::pair<std::string, server_list_entry>>();

	const bool show_password_popup = pending_password_entry.has_value();

	if (show_password_popup) {
		center_next_window(ImGuiCond_Appearing);
	}

	if (auto popup = cond_scoped_modal_popup(
		show_password_popup,
		"Server password required",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize
	)) {
		text("Server '%x' requires a password.", pending_password_entry->heartbeat.server_name);

		/*
			Don't use ImGuiInputTextFlags_EnterReturnsTrue here: the input_text wrapper
			only writes back into password_prompt_input when ImGui::InputText returns true,
			which with that flag happens only on Enter. We want every keystroke captured.
		*/
		const auto flags = ImGuiInputTextFlags_Password;

		if (ImGui::IsWindowAppearing()) {
			ImGui::SetKeyboardFocusHere();
		}

		/*
			At most what a handshake carries - a longer one would never get in.
		*/

		input_text<65>("Password##server_password_prompt", password_prompt_input, flags);

		const bool enter_pressed =
			ImGui::IsItemFocused()
			&& (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
		;

		const bool can_connect = !password_prompt_input.empty();

		const bool connect_clicked = [&]() {
			auto scope = maybe_disabled_cols(!can_connect);
			return ImGui::Button("Connect!");
		}();

		if (can_connect && (connect_clicked || enter_pressed)) {
			result = std::make_pair(std::exchange(password_prompt_input, std::string()), *pending_password_entry);
			pending_password_entry.reset();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		const bool escape_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape);

		if (ImGui::Button("Cancel") || escape_pressed) {
			pending_password_entry.reset();
			password_prompt_input.clear();
			ImGui::CloseCurrentPopup();
		}
	}

	return result;
}

bool browse_servers_gui_state::refresh_in_progress() const {
	/*
		Until the answer is read into the list (advance_list) - a join waiting for the list must not look
		at it between the answer arriving and being read.
	*/

	return data->future_response != nullptr;
}

void browse_servers_gui_state::load_cached_list_once(const browse_servers_input& in) {
	if (std::exchange(cache_loaded, true)) {
		return;
	}

	if (!augs::exists(in.list_cache_path)) {
		return;
	}

	try {
		auto file = augs::open_binary_input_stream(in.list_cache_path);
		auto bytes = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

		auto cache_error = std::string();
		auto cached = ::to_server_list({ 200, std::move(bytes) }, cache_error);

		if (cache_error.empty()) {
			LOG("Loaded %x servers from the cached server list.", cached.size());

			/*
				Its time to the next event was as of when it was downloaded - not known now.
			*/

			for (auto& entry : cached) {
				entry.heartbeat.next_event_at = -1.0;
			}

			cached_list = std::move(cached);
		}
		else {
			LOG("The cached server list does not read: %x", cache_error);
		}
	}
	catch (const std::exception& e) {
		LOG("Could not read the cached server list: %x", e.what());
	}
}

void browse_servers_gui_state::refresh_server_list(const browse_servers_input in) {
	using namespace httplib;

	load_cached_list_once(in);

	if (refresh_in_progress()) {
		return;
	}

	/*
		The list stays until a new one replaces it - a join meanwhile (e.g. Retry after a key mismatch) still finds
		its server's key there.
	*/

	error_message.clear();

	if (selected_server.is_set()) {
		reselect_after_refresh = selected_server;
	}

#if BUILD_NATIVE_SOCKETS
	/*
		The favorites' domains resolved anew - one that did not resolve may now.
	*/

	favorite_resolved.clear();
#endif

	selected_server = {};

	LOG("Launching future_response");

	data->future_response = augs::async_get(
		in.server_list_provider,
		"/server_list_binary"
	);

	LOG("refresh_server_list returns.");

	when_last_started_refreshing_server_list = augs::steady_secs();
}

void browse_servers_gui_state::refresh_server_pings() {
	reping_all_servers();
}

std::string get_steam_join_link(const std::string& addr);
std::string get_browser_join_link(const std::string& addr);

std::string server_list_entry::get_my_connect_string() const {
	/*
		Cases:
		Web server will always have a WebRTC id.
		Native server can also have WebRTC id as a reserved alias - e.g. 'pl', 'us' etc. for the official servers.

		- Web connects to Web - can only use webrtc id.
		- Native connects to Web - can only use webrtc id.
		- Web connects to Native - prefer alias, then ip as webrtc id.
		- Native connects to Native:
			- Target behind NAT: use IP as WebRTC id.
			- Target without NAT: use public IP directly.
	
	*/

#if PLATFORM_WEB
	return get_web_connect_string();
#else
	const bool for_sending = false;
	return get_native_connect_string(for_sending);
#endif
}

std::string server_list_entry::get_join_string() const {
	if (favorite_only) {
		return connect_target { favorite_address, get_public_key() }.to_string();
	}

	return connect_target { get_my_connect_string(), get_public_key() }.to_string();
}

/*
	Where a favorite is remembered to be - only a hint for when the server list does not have it.
*/

static std::string favorite_address_of(const server_list_entry& s) {
	if (s.favorite_only) {
		return s.favorite_address;
	}

	return s.get_invite_address();
}

static std::string favorite_name_of(const server_list_entry& s) {
	return std::string(s.heartbeat.server_name);
}

std::string server_list_entry::get_invite_address() const {
	if (meta.type == server_type::WEB) {
		return std::string(meta.webrtc_id);
	}

	const auto ip = ::ToString(address);
	return is_behind_nat() ? "rtc:" + ip : connect_target::without_default_port(ip);
}

std::string server_list_entry::get_web_connect_string() const {
	if (!meta.webrtc_id.empty()) {
		return meta.webrtc_id;
	}

	const auto external_ip_as_webrtc_id = ::ToString(address);
	return external_ip_as_webrtc_id;
}

std::string server_list_entry::get_native_connect_string(const bool for_sending) const {
	const auto& webrtc_id = meta.webrtc_id;
	const auto ip_address = ::ToString(get_ip_address(for_sending));

	if (meta.type == server_type::WEB) {
		return webrtc_id;
	}

	/* Native-to-native case */

	if (for_sending) {
		if (is_official_server()) {
			if (const auto domain = get_domain_with_port(); !domain.empty()) {
				return domain;
			}
		}
	}
	else {
		if (is_internal_network()) {
			return ip_address;
		}
	}

	if (is_behind_nat()) {
		return std::string("rtc:") + ip_address;
	}

	return ip_address;
}

std::string server_list_entry::get_domain_with_port() const {
	if (meta.domain.empty()) {
		return "";
	}

	return typesafe_sprintf("%x:%x", std::string(meta.domain), address.port);
}

bool server_list_entry::is_internal_network() const {
	return progress.found_on_internal_network && heartbeat.internal_network_address.has_value();
}

netcode_address_t server_list_entry::get_ip_address(bool for_sending) const {
	if (!for_sending) {
		if (is_internal_network()) {
			return *heartbeat.internal_network_address;
		}
	}

	return address;
}

bool server_list_entry::is_behind_nat() const {
	return heartbeat.is_behind_nat();
}

/*
	The one pinged with this sequence at this address - a favorite added by hand may be at the address of a listed
	server, and each gets its own answer.
*/

server_list_entry* browse_servers_gui_state::find_entry(const netcode_address_t& address, const uint64_t ping_sequence) {
	for (auto& s : displayed_list()) {
		if (s.address == address && s.progress.ping_sequence == ping_sequence) {
			return &s;
		}
	}

	for (auto& s : favorite_only_list) {
		if (s.address == address && s.progress.ping_sequence == ping_sequence) {
			return &s;
		}
	}

	return nullptr;
}

server_list_entry* browse_servers_gui_state::find_entry_by_internal_address(const netcode_address_t& address, const uint64_t ping_sequence) {
	for (auto& s : displayed_list()) {
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

		if (const auto entry = find_entry(from, sequence)) {
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

	auto ping_entry = [&](server_list_entry& s) {
		if (packets_left <= 0) {
			return;
		}

		if (s.meta.type == server_type::WEB) {
			return;
		}

		if (s.favorite_only && (s.address.type == NETCODE_ADDRESS_NONE || !s.other_server_at_address.empty())) {
			return;
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
				return;
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

		/*
			A favorite the list does not have gets no new list to try it again - one that did not answer is tried
			again now and then (it may be up by now).
		*/

		if (p.state == S::GIVEN_UP && s.favorite_only && s.other_server_at_address.empty() && show) {
			if (interval_passed(reping_interval, p.when_sent_first_ping)) {
				p = {};
			}
		}
	};

	for (auto& s : displayed_list()) {
		ping_entry(s);
	}

	for (auto& s : favorite_only_list) {
		ping_entry(s);
	}
}
#endif

/*
	The first column is the favorite star - no header, never sorted by.
*/

#if WEB_LOWEND
constexpr auto num_columns = 7;
#else
constexpr auto num_columns = 8;
#endif

/*
	A text in the name column of a row of its own (e.g. a section's label).
*/

static void name_column_row(const std::function<void()> content) {
	using namespace augs::imgui;

	next_columns(2);
	content();
	next_columns(num_columns - 2);
}



void browse_servers_gui_state::select_server(const server_list_entry& s) {
	selected_server = s;
	//server_details.open();
	scroll_once_to_selected = true;
}

void browse_servers_gui_state::show_server_list(
	const std::string& label,
	const std::vector<server_list_entry*>& server_list,
	const browse_servers_input& in
) {
	using namespace augs::imgui;

	const auto& style = ImGui::GetStyle();
	const auto& faction_view = in.faction_view;
	const auto streamer_mode = in.streamer_mode;

	if (server_list.empty()) {
		name_column_row([&]() {
			text_disabled(typesafe_sprintf("No %x servers match the applied filters.", label));
		});

		return;
	}

	for (const auto& sp : server_list) {
		auto id = scoped_id(index_in(server_list, sp));
		const auto& s = *sp;
		const auto& progress = s.progress;
		const auto& d = s.heartbeat;

		const bool given_up = progress.state == server_entry_state::GIVEN_UP;
		/*
			A favorite the list does not have knows no slots - never "full".
		*/

		const bool full = !s.favorite_only && d.is_full();
		const auto color = rgba(given_up || full ? style.Colors[ImGuiCol_TextDisabled] : style.Colors[ImGuiCol_Text]);

		auto do_text = [&](const auto& t) {
			text_color(t, color);
		};

		const bool is_selected = selected_server.same_server(s);

		/*
			The star: gold for a favorite, an outline otherwise - a click adds or removes it, never selects the row.
		*/

		/*
			A favorite added by hand (unlisted) is that address and key - see favorite_servers.h.
		*/

		const auto key = s.get_public_key();
		const bool manual_favorite = s.favorite_only && s.unlisted;

		const bool is_favorite = key.has_value() && (
			manual_favorite ?
			in.favorites.contains_manual(*key, s.favorite_address) :
			in.favorites.contains(*key, s.get_instance_label())
		);
		bool star_hovered = false;

		if (key.has_value() && favorites_kept) {
			const auto star_size = ImGui::GetTextLineHeight();
			const auto star_pos = ImGui::GetCursorScreenPos();

			/*
				The whole star column of the row, the gaps between the rows halved - never a pixel of the row's own tooltip
				between two stars.
			*/

			const auto& spacing = ImGui::GetStyle().ItemSpacing;
			const auto column_left = ImGui::GetWindowPos().x + ImGui::GetColumnOffset() - ImGui::GetScrollX();
			const auto hover_min = ImVec2(column_left, star_pos.y - spacing.y / 2 - 0.5f);
			const auto hover_max = ImVec2(column_left + ImGui::GetColumnWidth(), star_pos.y + star_size + spacing.y / 2 + 0.5f);

			star_hovered =
				ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)
				&& ImGui::IsMouseHoveringRect(hover_min, hover_max, false)
			;

			const auto star_color = [&]() {
				if (is_favorite) {
					return rgba(255, 200, 0, 255);
				}

				return rgba(star_hovered ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled]);
			}();

			const auto center = ImVec2(star_pos.x + star_size / 2, star_pos.y + star_size / 2);
			star_shape(ImGui::GetWindowDrawList(), center, star_size * 0.45f, ImGui::GetColorU32(ImVec4(star_color)), is_favorite);

			if (star_hovered && ImGui::IsMouseClicked(0)) {
				if (is_favorite) {
					if (manual_favorite) {
						forget_unlisted = std::make_pair(*key, s.favorite_address);
						forget_unlisted_name = std::string(s.heartbeat.server_name);
					}
					else {
						in.favorites.remove(*key, s.get_instance_label());
					}
				}
				else if (manual_favorite) {
					in.favorites.add_manual(*key, s.favorite_address, std::string(s.heartbeat.server_name));
				}
				else {
					in.favorites.set(
						*key,
						s.get_instance_label(),
						::favorite_address_of(s),
						::favorite_name_of(s),
						s.is_official_server()
					);
				}
			}

			ImGui::Dummy(ImVec2(star_size, star_size));
		}

		ImGui::NextColumn();

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
			/*
				No CASUAL or RANKED for a favorite the list does not have now - not known without it.
			*/

			if (!s.favorite_only && (s.is_official_server() || s.is_internal_network())) {
				if (d.is_ranked_server()) {
					const auto [label, stage_color] = [&]() -> std::pair<const char*, rgba> {
						switch (d.get_ranked_type()) {
							case ranked_server_type::TOURNAMENT_BYES:        return { "BYES",       wave_color };
							case ranked_server_type::TOURNAMENT_MID:         return { "TOURNAMENT", wave_color };
							case ranked_server_type::TOURNAMENT_SEMIFINALS:  return { "SEMIFINALS", wave_color };
							case ranked_server_type::TOURNAMENT_FINALS:      return { "FINALS",     wave_color };
							default:                                         return { "RANKED",     wave_color };
						}
					}();

					if (d.is_full()) {
						text_color(label, color);
					}
					else {
						text_color(label, stage_color);
					}

					ImGui::SameLine();
				}
				else {
					text_disabled("CASUAL");
					ImGui::SameLine();
				}
			}

			/*
				A padlock before the name of a server with a password.
			*/

			if (d.require_password) {
				padlock(ImGui::GetTextLineHeight(), rgba(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]));
				ImGui::SameLine();
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

			if (ImGui::Selectable(displayed_name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns) && !star_hovered) {
				selected_server = s;
			}
		}

		const bool ranked_unavailable = s.is_official_server() && d.is_ranked_server() && !ranked_unavailable_reason.empty();

		if (star_hovered) {
			auto tool = scoped_tooltip();
			text(is_favorite ? "Remove from favorites" : "Add to favorites");
		}
		else if (ImGui::IsItemHovered() && s.favorite_only) {
			auto tool = scoped_tooltip();

			if (s.unlisted) {
				text_disabled("Unlisted - added by hand.");
			}

			if (!s.other_server_at_address.empty()) {
				text_color(typesafe_sprintf("Another server is at its address now: %x", s.other_server_at_address), orange);
				text_disabled("This one moved or got a new key - add it to your favorites anew.");
			}

			if (!streamer_mode) {
				text_disabled(typesafe_sprintf(s.unlisted ? "Address: %x" : "Last seen at: %x", s.favorite_address));
			}
		}
		else if (ImGui::IsItemHovered()) {
			auto tool = scoped_tooltip();

			if (ranked_unavailable) {
				text_color(ranked_unavailable_reason, orange);
				ImGui::Separator();
			}

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
		
		if (ImGui::IsItemClicked() && !ranked_unavailable && !star_hovered) {
			if (ImGui::IsMouseDoubleClicked(0)) {
				LOG("Double-clicked server list entry: %x (%x). Connecting.", d.server_name, ToString(s.address));

				displayed_connecting_server_name = d.server_name;
				requested_connection = s.get_join_string();
			}
		}

		if (ImGui::IsItemClicked(1)) {
			selected_server = s;
			server_details.open();
		}

		ImGui::NextColumn();

		if (s.favorite_only) {
			text_disabled("-");
			next_columns(num_columns - 3);
			continue;
		}

		const auto players_text = typesafe_sprintf("%x/%x", d.num_online, d.max_online());

		if (d.any_human() == 0) {
			text_disabled(players_text);
		}
		else {
			do_text(players_text);
		}

		ImGui::NextColumn();

		if (s.is_official_server() ) {
			if (d.next_event_at >= 0.0) {
				const auto secs_left = d.cached_time_to_event == std::numeric_limits<double>::max() ? -1.0 : d.cached_time_to_event;

				if (const auto result = augs::date_time::format_time_until_weekend_evening(secs_left)) {
					if (const bool ongoing = *result == "") {
						const auto event_name = std::string(d.next_event_name);
						text_color(event_name.empty() ? std::string("Event") : event_name, wave_color);
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

#if !WEB_LOWEND
		const auto secs_ago = augs::date_time::secs_since_epoch() - s.meta.time_hosted;
		text_disabled(augs::date_time::format_countdown_letters(secs_ago));

		ImGui::NextColumn();
#endif
	}

	scroll_once_to_selected = false;
}

void browse_servers_gui_state::advance_list(const browse_servers_input& in) {
	if (augs::is_ready(data->future_response)) {
		/*
			A failed download keeps the last list (and says why) - an empty one would expect no key of any server.
		*/

		const auto response = augs::get_once(data->future_response);
		auto new_list = ::to_server_list(response, error_message);

		if (error_message.empty()) {
			server_list = std::move(new_list);
			fresh_list_arrived = true;
			cached_list.clear();
			::save_cached_list(in.list_cache_path, response.body);
		}

		when_last_updated_time_to_events = 0;
	}

	/*
		At the start: the cached list at once, and a fresh one downloading in the background.
	*/

	if (!refreshed_at_least_once()) {
		refresh_server_list(in);
	}
}

/*
	Forgetting a favorite asked first: true once Forget is clicked, false on Cancel, none while it asks.
	Its name may be its address - not shown to a stream.
*/

static std::optional<bool> ask_to_forget(const std::string& name, const bool streamer_mode) {
	const auto question =
		streamer_mode || name.empty() ?
		std::string("Forget this server?") :
		typesafe_sprintf("Forget \"%x\"?", name)
	;

	auto popup = simple_popup { "Forget a server", question, "" };

	const auto buttons = std::vector<simple_popup::button> {
		{ "Forget", rgba(200, 80, 0, 255), rgba(25, 20, 0, 255) },
		{ "Cancel", rgba::zero, rgba::zero }
	};

	if (const auto result = popup.perform(buttons)) {
		return result == 1;
	}

	return std::nullopt;
}

bool browse_servers_gui_state::perform(const browse_servers_input in) {
	using namespace httplib_utils;

	advance_list(in);

	update_favorite_only_list(in.favorites);

	if (!show) {
		/*
			Local servers take the first look in a run only - not a later one.
		*/

		if (shown_once) {
			local_tab_precedence_pending = false;
		}

		return false;
	}

	if (!std::exchange(shown_once, true)) {
		first_shown_at = augs::steady_secs();
	}

	using namespace augs::imgui;

	{
		const auto current_time = augs::steady_secs();

		if (current_time - when_last_updated_time_to_events > 1.0) {
			when_last_updated_time_to_events = current_time;

			update_cached_time_to_event(displayed_list());
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

	/*
		What went wrong with the server list - above all the tabs, which show what they can meanwhile.
	*/

	if (error_message.size() > 0) {
		text_color(error_message, red);
	}

	if (!std::exchange(tab_loaded, true)) {
		/*
			The remembered tab - All instead of an empty Favorites.
		*/

		current_tab = in.remembered_tab;

		if (current_tab == browser_tab::FAVORITES && in.favorites.all().empty()) {
			current_tab = browser_tab::ALL;
		}
	}

	{
		/*
			Local, All, Favorites - Local only while there are local servers, Favorites not on CrazyGames
			(see favorites_kept). Only the player's click is remembered - not All shown in place of a tab not there,
			nor Local taking the first look.
		*/

		auto shown_tabs = std::vector<browser_tab>();

		if (had_local_servers) {
			shown_tabs.push_back(browser_tab::LOCAL);
		}

		shown_tabs.push_back(browser_tab::ALL);

		if (favorites_kept) {
			shown_tabs.push_back(browser_tab::FAVORITES);
		}

		if (std::find(shown_tabs.begin(), shown_tabs.end(), current_tab) == shown_tabs.end()) {
			current_tab = browser_tab::ALL;
		}

		auto labels = std::vector<const char*>();
		int index = 0;

		for (std::size_t i = 0; i < shown_tabs.size(); ++i) {
			labels.push_back(shown_tabs[i] == browser_tab::LOCAL ? "Local" : shown_tabs[i] == browser_tab::ALL ? "All" : "Favorites");

			if (shown_tabs[i] == current_tab) {
				index = static_cast<int>(i);
			}
		}

		const auto index_before = index;

		{
			auto style = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto padding = ImGui::GetStyle().FramePadding; padding.x *= 4; return padding; }());
			ImGui::TabLabels(labels.data(), static_cast<int>(labels.size()), index, nullptr);
		}

		current_tab = shown_tabs[index];

		{
			auto sep = scoped_style_color(ImGuiCol_Separator, ImGui::GetStyle().Colors[ImGuiCol_Button]);
			ImGui::Separator();
		}

		if (const bool clicked = index != index_before) {
			local_tab_precedence_pending = false;
			in.remembered_tab = current_tab;
		}
	}

	thread_local ImGuiTextFilter filter;

	filter_with_hint(filter, "##HierarchyFilter", "Filter server names/arenas, or paste a server's key...");

	/*
		A server's key pasted into the filter (with 0x or bare) shows that host's servers only.
	*/

	const auto filter_key = connect_target::key_of(filter.InputBuf);

	auto passes_filter = [&](const server_list_entry& s, const std::string& text) {
		if (filter_key.has_value()) {
			return s.get_public_key() == filter_key;
		}

		return filter.PassFilter(text.c_str());
	};
	local_server_list.clear();
	tournament_server_list.clear();
	official_server_list.clear();
	community_server_list.clear();

	bool has_local_servers = false;
	bool has_tournament_servers = false;
	bool has_official_servers = false;
	bool has_community_servers = false;

	requested_connection = std::nullopt;

	for (auto& s : displayed_list()) {
		const auto& name = s.heartbeat.server_name;
		const auto& arena = s.heartbeat.current_arena;
		const auto effective_name = std::string(name) + " " + std::string(arena);

		const bool is_internal = s.progress.found_on_internal_network;

		auto push_if_passes = [&](auto& target_list) {
			if (only_responding) {
				if (!s.progress.responding()) {
					return;
				}
			}

			if (no_password) {
				if (s.heartbeat.require_password) {
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
				if (!s.heartbeat.versions_compatible()) {
					return;
				}
			}

			if (!passes_filter(s, effective_name)) {
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
				if (s.heartbeat.is_tournament_server()) {
					has_tournament_servers = true;
					push_if_passes(tournament_server_list);
				}
				else {
					has_official_servers = true;
					push_if_passes(official_server_list);
				}
			}
			else {
				has_community_servers = true;
				push_if_passes(community_server_list);
			}
		}
	}

	unlisted_shown.clear();

	for (auto& s : favorite_only_list) {
		if (!passes_filter(s, std::string(s.heartbeat.server_name))) {
			continue;
		}

		/*
			The other filters need what only the list knows (players, a password) - Only responding does not.
		*/

		if (only_responding && !s.progress.responding()) {
			continue;
		}

		/*
			A favorite starred on the list answering on this machine or the local network is a local server - found
			even when the server list hangs or is down. One added by hand stays under Unlisted.
		*/

		if (!s.unlisted && s.progress.responding() && ::is_internal(s.address)) {
			has_local_servers = true;
			local_server_list.push_back(&s);
			continue;
		}

		/*
			Starred on the list but not on it now (or no list): on the Favorites tab where it was last seen, official
			or community, sorted along with the listed ones. The ones added by hand go under Unlisted.
		*/

		if (!s.unlisted) {
			if (current_tab == browser_tab::FAVORITES) {
				(s.meta.is_official ? official_server_list : community_server_list).push_back(&s);
			}

			continue;
		}

		unlisted_shown.push_back(&s);
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
				sort_range(tournament_server_list, make_comparator(cmps...));
				sort_range(official_server_list, make_comparator(cmps...));
				sort_range(community_server_list, make_comparator(cmps...));
				sort_range(unlisted_shown, make_comparator(cmps...));
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

	/*
		Only right after the first opening - never switching the tab under the cursor later.
	*/

	had_local_servers = has_local_servers;

	if (local_tab_precedence_pending && augs::steady_secs() - first_shown_at > 2.0) {
		local_tab_precedence_pending = false;
	}

	if (local_tab_precedence_pending && has_local_servers) {
		local_tab_precedence_pending = false;
		current_tab = browser_tab::LOCAL;
	}

	ImGui::Separator();

	if (when_last_started_refreshing_server_list == 0) {
		refresh_server_list(in);
	}

	/*
		No unlisted servers in a browser: it reaches a server only through the server list's signalling (WebRTC).
	*/

#if !PLATFORM_WEB
	/*
		The clipboard auto-pasted (see unlisted_server_fields.h) when Add an unlisted server opens, or when the game
		gets the focus back while it is open (e.g. after copying in a browser) - never while editing one.
	*/

	auto autopaste_from_clipboard = [&]() {
		if (!editing_unlisted.has_value()) {
			add_server.autopaste(ImGui::GetClipboardText());
		}
	};

	if (const auto activated = in.times_window_activated; activated != std::exchange(times_window_activated, activated)) {
		if (add_server_open) {
			autopaste_from_clipboard();
		}
	}

	/*
		A white plus where the star is, "Add an unlisted server" where the name is - one row of the table's columns.
	*/

	auto draw_add_server_row = [&]() {
		const auto size = ImGui::GetTextLineHeight();
		const auto pos = ImGui::GetCursorScreenPos();
		const auto center = ImVec2(pos.x + size / 2, pos.y + size / 2);
		const auto arm = size * 0.35f;
		const auto thickness = std::max(1.0f, size * 0.12f);
		auto* const draw_list = ImGui::GetWindowDrawList();

		draw_list->AddLine(ImVec2(center.x - arm, center.y), ImVec2(center.x + arm, center.y), IM_COL32_WHITE, thickness);
		draw_list->AddLine(ImVec2(center.x, center.y - arm), ImVec2(center.x, center.y + arm), IM_COL32_WHITE, thickness);
		ImGui::Dummy(ImVec2(size, size));

		next_columns(2);

		if (ImGui::Selectable("Add an unlisted server", false, ImGuiSelectableFlags_SpanAllColumns)) {
			add_server_open = true;
			editing_unlisted.reset();

			/*
				A new dialog takes the clipboard as it is, even one taken before.
			*/

			add_server = unlisted_server_fields();
			autopaste_from_clipboard();
		}

		next_columns(num_columns - 2);
	};
#endif

#if PLATFORM_WEB
	const bool add_row_pinned_now = false;
#else
	const bool add_row_pinned_now = current_tab == browser_tab::FAVORITES && add_server_row_pinned;
#endif
	auto column_widths = std::array<float, num_columns>();
	auto table_width = 0.0f;

	auto set_column_widths = [&]() {
		for (int i = 0; i < num_columns; ++i) {
			ImGui::SetColumnWidth(i, column_widths[i]);
		}
	};

	auto do_list_view = [&](bool disable_content_view) {
		const auto max_sname = 35;

		const auto avail_x = ImGui::GetContentRegionAvail().x;

		const float col_star = ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.x * 2;
		float col_1 = ImGui::CalcTextSize("9999999").x;
		float col_3 = ImGui::CalcTextSize("99h 99m 9").x;
		float col_4 = ImGui::CalcTextSize("Players  9").x;
		float col_5 = ImGui::CalcTextSize("9").x * (max_arena_name_length_v / 2);
		float col_6 = ImGui::CalcTextSize("Capture the flag 9").x;
		float col_7 = ImGui::CalcTextSize("9").x * 25;

		float total_width = col_star + col_1 + col_3 + col_4 + col_5 + col_6 + col_7;

		float col_2 = std::max(ImGui::CalcTextSize("9").x * (max_sname - 5) - 4, avail_x - total_width);
		total_width += col_2;

		/*
			Room below for the buttons view - and the Favorites tab's note there.
		*/

		const auto favorites_note_h = current_tab == browser_tab::FAVORITES ? ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y : 0.0f;
		const auto pinned_row_h = add_row_pinned_now ? ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 2 : 0.0f;
		table_width = total_width;
		const auto buttons_view_h = 2 * (ImGui::GetFrameHeightWithSpacing() + 4) + favorites_note_h + pinned_row_h;

		auto child = scoped_child("list view", ImVec2(total_width, -buttons_view_h), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (disable_content_view) {
			return;
		}

		ImGui::Columns(num_columns);

#if WEB_LOWEND
		(void)col_7;
		column_widths = { col_star, col_1, col_2, col_3, col_4, col_5, col_6 };
#else
		column_widths = { col_star, col_1, col_2, col_3, col_4, col_5, col_6, col_7 };
#endif

		set_column_widths();

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
			ImGui::NextColumn();
			do_column("Ping");
			do_column(with_label, col);
			do_column("Players");
			do_column("Event");
			do_column("Arena");
			do_column("Game mode");
#if !WEB_LOWEND
			do_column("Uptime");
#endif

			ImGui::Separator();
		};

		/*
			The Favorites tab: the favorites from each list, then the ones not on it.
		*/

		const bool favorites_tab = current_tab == browser_tab::FAVORITES;

		auto shown = [&](const std::vector<server_list_entry*>& list) {
			if (!favorites_tab) {
				return list;
			}

			auto out = std::vector<server_list_entry*>();

			for (const auto e : list) {
				if (const auto key = e->get_public_key(); key.has_value() && in.favorites.contains(*key, e->get_instance_label())) {
					out.push_back(e);
				}
			}

			return out;
		};

		const auto locals = shown(local_server_list);
		const auto tournaments = shown(tournament_server_list);
		const auto officials = shown(official_server_list);
		const auto communities = shown(community_server_list);

		const auto local_servers_label = typesafe_sprintf("Local servers (%x)", locals.size());
		const auto tournament_servers_label = typesafe_sprintf("Ongoing Tournament (%x)", tournaments.size());
		const auto official_servers_label = typesafe_sprintf("Official servers (%x)", officials.size());
		const auto community_servers_label = typesafe_sprintf("Community servers (%x)", communities.size());
#if !PLATFORM_WEB
		const auto unlisted_label = typesafe_sprintf("Unlisted (%x)", unlisted_shown.size());
#endif

		auto separate_with_label_only = [&](const auto& label, const auto& color) {
			text_disabled("\n\n");

			ImGui::Separator();
			name_column_row([&]() {
				text_color(label, color);
			});
			ImGui::Separator();
		};

		bool first_section = true;

		auto header_or_separator = [&](const auto& label, const auto& color) {
			if (first_section) {
				do_column_labels(label, color);
				first_section = false;
			}
			else {
				separate_with_label_only(label, color);
			}
		};

		auto section = [&](const auto& header, const auto& color, const std::string& kind, const auto& list, const std::string& none_text) {
			header_or_separator(header, color);

			if (list.empty() && !none_text.empty()) {
				name_column_row([&]() {
					text_disabled(none_text);
				});
			}
			else {
				show_server_list(kind, list, in);
			}
		};

		const auto t = current_tab;

		if (favorites_tab) {
			const auto any_favorite = !in.favorites.all().empty();

			if (!locals.empty()) {
				section(local_servers_label, green, "local", locals, "");
			}

			if (!tournaments.empty()) {
				section(tournament_servers_label, cyan, "tournament", tournaments, "");
			}

			if (!officials.empty()) {
				section(official_servers_label, yellow, "official", officials, "");
			}

			if (!communities.empty()) {
				section(community_servers_label, orange, "community", communities, "");
			}

#if !PLATFORM_WEB
			if (!unlisted_shown.empty()) {
				section(unlisted_label, white, "unlisted", unlisted_shown, "");
			}
#endif

			if (first_section) {
				do_column_labels(std::string("Favorites"), std::optional<rgba>());

				name_column_row([&]() {
					if (any_favorite) {
						text_disabled("No favorite servers match the applied filters.");
					}
					else {
#if PLATFORM_WEB
						text_disabled("No favorite servers yet. Click the star next to a server.");
#else
						text_disabled("No favorite servers yet. Click the star next to a server, or add an unlisted one below.");
#endif
					}
				});
			}

#if !PLATFORM_WEB
			/*
				The last row: adding a server by hand - pinned below the table instead once the table scrolls
				(decided by the last frame's height, the same with the row inside or not - no flicker).
			*/

			const auto add_row_h = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 2;
			const auto table_h_without_pin = ImGui::GetWindowHeight() + (add_server_row_pinned ? add_row_h : 0.0f);
			const bool pin_next = ImGui::GetCursorPosY() + add_row_h > table_h_without_pin;

			if (!add_server_row_pinned) {
				ImGui::Separator();
				draw_add_server_row();
			}

			add_server_row_pinned = pin_next;
#endif
			return;
		}

		if (t == browser_tab::LOCAL || (t == browser_tab::ALL && has_local_servers)) {
			section(local_servers_label, green, "local", locals, has_local_servers ? "" : "No servers found on your local network.");
		}

		if (t == browser_tab::ALL) {
			if (has_tournament_servers) {
				section(tournament_servers_label, cyan, "tournament", tournaments, "");
			}

			section(official_servers_label, yellow, "official", officials, has_official_servers ? "" : "No official servers are online at the time.");
		}

		if (t == browser_tab::ALL) {
			section(community_servers_label, orange, "community", communities, has_community_servers ? "" : "No community servers are online at the time.");
		}
	};

	do_list_view(false);

#if !PLATFORM_WEB
	if (add_row_pinned_now) {
		/*
			A child like the table's, so the columns line up with it.
		*/

		ImGui::Separator();

		auto pinned = scoped_child("pinned add row", ImVec2(table_width, ImGui::GetTextLineHeightWithSpacing()), false, ImGuiWindowFlags_NoScrollbar);

		ImGui::Columns(num_columns, "##PinnedAddServer");
		set_column_widths();
		draw_add_server_row();
		ImGui::Columns(1);
	}
#endif

	{
		auto child = scoped_child("buttons view");

		ImGui::Separator();

		if (current_tab == browser_tab::FAVORITES) {
			text_disabled(typesafe_sprintf("Favorite servers are remembered by their public key (%x), for security.", std::string(augs::key_scheme_name)));
			ImGui::Separator();
		}

		checkbox("Only responding", only_responding);
		ImGui::SameLine();

		checkbox("No password", no_password);
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
			const bool ranked_unavailable =
				selected_server.is_set()
				&& selected_server.is_official_server()
				&& selected_server.heartbeat.is_ranked_server()
				&& !ranked_unavailable_reason.empty()
			;

			auto scope = maybe_disabled_cols({}, !selected_server.is_set() || ranked_unavailable);

			if (ImGui::Button("Connect") && !ranked_unavailable) {
				auto& s = selected_server;

				LOG("Chosen server list entry: %x (%x). Connecting.", ToString(s.address), s.heartbeat.server_name);

				displayed_connecting_server_name = s.heartbeat.server_name;
				requested_connection = s.get_join_string();
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

		/*
			The list on its way - the buttons are greyed meanwhile.
		*/

		if (refresh_in_progress()) {
			ImGui::SameLine();
			spinner(ImGui::GetTextLineHeight(), white);
			ImGui::SameLine();
			text_disabled("Downloading the server list...");
		}
	}
	
	/*
		Once the new list is in (or the edited favorite) - tried once, as it is now.
	*/

	if (reselect_after_refresh.has_value() && !refresh_in_progress()) {
		auto reselect = [&](const auto& list) {
			for (const auto& s : list) {
				if (s.same_server(*reselect_after_refresh)) {
					selected_server = s;
					return true;
				}
			}

			return false;
		};

		if (!selected_server.is_set() && !reselect(displayed_list())) {
			reselect(favorite_only_list);
		}

		reselect_after_refresh.reset();
	}

	/*
		Server details only with the browser.
	*/

	if (server_details.perform(selected_server, in.faction_view, in.streamer_mode, std::addressof(in.favorites))) {
		refresh_server_list(in);
	}

#if !PLATFORM_WEB
	if (std::exchange(server_details.forget_requested, false)) {
		if (const auto key = selected_server.get_public_key(); key.has_value() && selected_server.unlisted) {
			forget_unlisted = std::make_pair(*key, selected_server.favorite_address);
			forget_unlisted_name = std::string(selected_server.heartbeat.server_name);
		}
	}

	/*
		Forgetting a favorite added by hand loses what was typed in - asked first. Its name may be its address,
		not shown to a stream.
	*/

	if (forget_unlisted.has_value()) {
		if (const auto forget = ::ask_to_forget(forget_unlisted_name, in.streamer_mode); forget.has_value()) {
			if (*forget) {
				in.favorites.remove_manual(forget_unlisted->first, forget_unlisted->second);
				selected_server = server_list_entry();
			}

			forget_unlisted.reset();
		}
	}

	if (std::exchange(server_details.edit_requested, false)) {
		if (const auto key = selected_server.get_public_key(); key.has_value() && selected_server.unlisted) {
			add_server_open = true;
			editing_unlisted = std::make_pair(*key, selected_server.favorite_address);

			const auto name = std::string(selected_server.heartbeat.server_name);

			add_server = unlisted_server_fields();
			add_server.address = selected_server.favorite_address;
			add_server.key = "0x" + augs::bytes_to_hex(*key);
			add_server.name = name == selected_server.favorite_address ? std::string() : name;
		}
	}

	{
		/*
			Add an unlisted server to the favorites by hand: where it is and its key - both needed; the name optional
			(the address if none). Never matched against the server list (see favorite_servers.h).
			The same dialog edits one (Edit in its Server details).
		*/

		if (add_server_open) {
			center_next_window(ImGuiCond_Appearing);
		}

		const auto dialog_title = editing_unlisted.has_value() ? "Edit an unlisted server###unlisted_server" : "Add an unlisted server###unlisted_server";

		if (auto popup = cond_scoped_modal_popup(
			add_server_open,
			dialog_title,
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize
		)) {
			if (ImGui::IsWindowAppearing()) {
				ImGui::SetKeyboardFocusHere();
			}

			/*
				A paste taken whole (see unlisted_server_fields.h) - the key's and the name's fields would cut it short.
			*/

			add_server.take_paste();

			{
				auto width = scoped_item_width(ImGui::CalcTextSize("9").x * 66);

				/*
					A hostname is at most 253 characters, with ":port" 259; the key 64 hex digits with "0x"; the name
					as long as a server's own. A click into a field selects all of it - to paste over at once.
				*/

				auto field = [](const std::string& label, const char* const hint, std::string& value, auto buf) {
					input_text_with_hint(buf, label, hint, value, ImGuiInputTextFlags_AutoSelectAll);
				};

				field("Address##add_server", "e.g. 154.12.226.211:8412", add_server.address, std::array<char, 260>());
				add_server.address_active = ImGui::IsItemActive();

				if (std::exchange(add_server.focus_key_next, false)) {
					ImGui::SetKeyboardFocusHere();
				}

				field("Public key##add_server", "64 hex digits - from the server's admin", add_server.key, std::array<char, 67>());
				add_server.key_active = ImGui::IsItemActive();
				field("Server name (optional)##add_server", "", add_server.name, std::array<char, max_server_name_length_v + 1>());
			}

			/*
				"<address>,0x<key>" typed into the address's field (or pasted there not with Ctrl+V) is split too.
			*/

			add_server.take(add_server.address);

			const auto key = add_server.typed_key();
			const auto address = add_server.typed_address();

			/*
				Not twice - the one being edited saved as it was is fine.
			*/

			const bool the_edited_one =
				editing_unlisted.has_value()
				&& key == editing_unlisted->first
				&& address == editing_unlisted->second
			;

			const bool already_there = key.has_value() && !the_edited_one && in.favorites.contains_manual(*key, address);
			const bool can_add = add_server.ready() && !already_there;

			const auto already_there_note = std::make_pair(
				std::string("Already in your favorites."),
				rgba(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled])
			);

			add_server.status_line(
				already_there ? std::make_optional(already_there_note) : std::nullopt,
				!editing_unlisted.has_value(),
				"\"ip,key,name\" in clipboard auto-fills these."
			);

			ImGui::Separator();

			const bool add_clicked = [&]() {
				auto scope = maybe_disabled_cols(!can_add);
				return ImGui::Button(editing_unlisted.has_value() ? "Save" : "Add server");
			}();

			if (can_add && add_clicked) {
				const auto name = connect_target::trimmed(add_server.name);

				if (editing_unlisted.has_value()) {
					in.favorites.edit_manual(editing_unlisted->first, editing_unlisted->second, *key, address, name);

					/*
						The edited row stays selected - as it is after the edit, once the favorites have it.
					*/

					auto edited = server_list_entry();
					edited.favorite_only = true;
					edited.unlisted = true;
					edited.meta.server_public_key = *key;
					edited.favorite_address = address;

					selected_server = server_list_entry();
					reselect_after_refresh = edited;
				}
				else {
					in.favorites.add_manual(*key, address, name);
				}

				add_server_open = false;
				editing_unlisted.reset();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				add_server_open = false;
				editing_unlisted.reset();
			}
		}
	}
#endif

	if (requested_connection.has_value()) {
		in.client_connect = *requested_connection;
		in.displayed_connecting_server_name = displayed_connecting_server_name;

		return true;
	}

	return false;
}

std::optional<server_list_entry> browse_servers_gui_state::find_best_server(const bool allow_ranked) const {
	auto filtered = server_list;

	erase_if(
		filtered,
		[&](auto& f) {
			if (!f.heartbeat.versions_compatible()) {
				return true;
			}

			if (const bool filter_rankeds = !allow_ranked) {
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

std::optional<server_list_entry> browse_servers_gui_state::find_quick_play_server() const {
	auto candidates = server_list;

	erase_if(candidates, [&](const auto& s) {
		const bool is_ranked = s.is_official_server() && s.heartbeat.is_ranked_server();
		return !s.heartbeat.versions_compatible() || is_ranked || s.heartbeat.require_password || s.heartbeat.is_full();
	});

	auto public_ones = candidates;

	erase_if(public_ones, [&](const auto& s) {
		return s.is_behind_nat() && !s.progress.found_on_internal_network;
	});

	const auto& chosen = public_ones.empty() ? candidates : public_ones;

	if (chosen.empty()) {
		return std::nullopt;
	}

	return minimum_of(chosen, compare_servers);
}

const server_list_entry* browse_servers_gui_state::find_entry_by_key(
	const augs::ed25519_public_key_type& key,
	const std::string& label
) const {
	/*
		That instance if the label names one, else the host's best (see compare_servers).
	*/

	const server_list_entry* best = nullptr;

	for (const auto& s : server_list) {
		if (s.get_public_key() != key) {
			continue;
		}

		if (!label.empty()) {
			if (std::string(s.heartbeat.instance_label) == label) {
				return &s;
			}

			continue;
		}

		if (best == nullptr || ::compare_servers(s, *best)) {
			best = &s;
		}
	}

	return best;
}

invite_target invite_target::of(
	const std::string& address,
	const std::optional<augs::ed25519_public_key_type>& key,
	const server_list_entry* const listed
) {
	auto out = invite_target();
	out.key = key;

	auto is_internal_address = [](const std::string& joined) {
		if (const auto ip = ::find_netcode_addr(joined)) {
			return ::is_internal(*ip);
		}

		return false;
	};

	if (listed != nullptr) {
		out.native_address = listed->get_invite_address();
		out.web_id = listed->get_web_connect_string();

		if (const auto joined = connect_target::without_default_port(address); is_internal_address(address) && joined != out.native_address) {
			out.local_address = joined;
		}

		return out;
	}

	/*
		Not on the list: where the player joined. A web client joined by a WebRTC id, which a native game joins too;
		an IP is a WebRTC id for the browser as it is.
	*/

	out.native_address = connect_target::without_default_port(address);

	auto web_id = client_connect_string(address);
	cut_preffix(web_id, "rtc:");

	/*
		A server's WebRTC id is its IP always with the port - the default one too.
	*/

	if (const auto ip = ::find_netcode_addr(web_id)) {
		web_id = ::ToString(*ip);
	}

	out.web_id = web_id;
	out.local_network_only = is_internal_address(web_id);

	return out;
}

std::string invite_target::join_string() const {
	return connect_target { native_address, key }.to_string();
}

void invite_to_join_gui_state::perform(const invite_to_join_input& in) {
	using namespace augs::imgui;

	if (!show) {
		return;
	}

	auto window = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);

	if (!window) {
		return;
	}

	text(in.server_name.empty() ? std::string("This server") : in.server_name);

	/*
		A favorite only once the server told which instance it is (its label) - see favorite_servers.h.
	*/

	if (in.server_key.has_value() && favorites_kept && !in.server_label.empty()) {
		ImGui::SameLine();

		if (in.favorites.contains(*in.server_key, in.server_label)) {
			star_line("In favorites", green);
			ImGui::SameLine();

			if (ImGui::Button("Forget")) {
				forget_asked = true;
			}

			if (forget_asked) {
				if (const auto forget = ::ask_to_forget(in.server_name, in.streamer_mode); forget.has_value()) {
					if (*forget) {
						in.favorites.remove(*in.server_key, in.server_label);
					}

					forget_asked = false;
				}
			}
		}
		else if (star_button("Add to favorites")) {
			const auto target = invite_target::of(in.address, in.server_key, in.listed);
			const auto official = in.listed != nullptr ? std::make_optional(in.listed->is_official_server()) : std::nullopt;
			in.favorites.set(*in.server_key, in.server_label, target.native_address, in.server_name, official);
		}
	}

	ImGui::Separator();

	if (in.address.empty() && in.listed == nullptr) {
		text_color(
			in.hosting
			? "The server's public address is not known yet -\nit learns it from the server list host."
			: "Not connected to a server.",
			orange
		);

		return;
	}

	const auto target = invite_target::of(in.address, in.server_key, in.listed);
	const auto rdonly_flags = ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll;

	/*
		A key shown shortened in the middle, all of it when clicked (see long_input_text_with_dots).
	*/

	auto row = [&](const std::string& label, std::string value, const bool hidden, const bool expanding = false) {
		if (hidden) {
			text_disabled("(hidden)");
		}
		else if (expanding) {
			long_input_text_with_dots<67>("##" + label, value, ImGuiInputTextFlags_ReadOnly);
		}
		else {
			input_text("##" + label, value, rdonly_flags);
		}

		ImGui::SameLine();
		copy_icon_button(label, value);
		ImGui::SameLine();
		text(label);
	};

	/*
		The streamer mode hides an address - the links carry it too.
	*/

	const bool hide = in.streamer_mode;

	row("Browser join link", ::get_browser_join_link(connect_target { target.web_id, target.key }.to_web_string()), hide);
	row("Steam join link", ::get_steam_join_link(target.join_string()), hide);
	row("Address", target.native_address, hide);

	if (!target.local_address.empty()) {
		row("Local network address", target.local_address, hide);
	}

	if (target.key.has_value()) {
		row("Server public key", "0x" + augs::bytes_to_hex(*target.key), false, true);
	}
	else {
		text_color("The server's key is not known - a join is not verified.", orange);
	}

	if (target.local_network_only) {
		ImGui::Separator();
		text_color("An address in your local network - friends elsewhere need the server's public one\n(it shows here once the server list has this server).", orange);
	}
}

const server_list_entry* browse_servers_gui_state::find_entry_by_connect_string(const client_connect_string& joined) const {
	LOG("Finding the server entry by: %x", joined);
	LOG("Number of servers in the browser: %x", server_list.size());

	/*
		A join that names the key finds the server by it - wherever it is now.
	*/

	const auto target = connect_target::parse(joined);

	/*
		With an address too: the instance there if it has the key (all of a host's instances share it) - never another
		instance of the host. The key alone: the host's best one.
	*/

	if (target.key.has_value()) {
		if (!target.address.empty()) {
			if (const auto there = find_entry_by_connect_string(client_connect_string(target.address)); there != nullptr && there->get_public_key() == target.key) {
				return there;
			}

			return nullptr;
		}

		return find_entry_by_key(*target.key, std::string());
	}

	const auto& in = target.address;

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
			if (s.get_web_connect_string() == in) {
				return &s;
			}

			const auto native = connect_target::without_default_port(s.get_native_connect_string(true));

			if (native == connect_target::without_default_port(in)) {
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
	const bool streamer_mode,
	favorite_servers* const favorites
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

	/*
		The Steam join link names the server by its IP (a domain is only a label the server declares) and its key.
	*/

	const auto steam_join = [&]() {
		if (entry.favorite_only) {
			return entry.get_join_string();
		}

		return connect_target { entry.get_invite_address(), entry.get_public_key() }.to_string();
	}();

	auto steam_link = ::get_steam_join_link(steam_join);

	auto sid = scoped_id(steam_link.c_str());

	bool refresh_requested = false;

	if (listed) {
		if (ImGui::Button("Refresh")) {
			refresh_requested = true;
		}

		ImGui::SameLine();
	}

	if (favorites != nullptr && favorites_kept) {
		if (const auto key = entry.get_public_key()) {
			const bool manual = entry.favorite_only && entry.unlisted;

			const bool is_favorite =
				manual ?
				favorites->contains_manual(*key, entry.favorite_address) :
				favorites->contains(*key, entry.get_instance_label())
			;

			if (is_favorite) {
				/*
					Only a favorite added by hand is edited - the list's ones are as the list has them.
				*/

				if (manual) {
					if (ImGui::Button("Edit")) {
						edit_requested = true;
					}

					ImGui::SameLine();
				}

				if (ImGui::Button("Remove from favorites")) {
					if (manual) {
						forget_requested = true;
					}
					else {
						favorites->remove(*key, entry.get_instance_label());
					}
				}
			}
			else if (star_button("Add to favorites")) {
				if (manual) {
					favorites->add_manual(*key, entry.favorite_address, std::string(entry.heartbeat.server_name));
				}
				else {
					favorites->set(
						*key,
						entry.get_instance_label(),
						::favorite_address_of(entry),
						::favorite_name_of(entry),
						entry.is_official_server()
					);
				}
			}
		}
	}

	if (refresh_requested) {
		return true;
	}

	ImGui::Separator();

	auto heartbeat = entry.heartbeat;
	const auto& internal_addr = heartbeat.internal_network_address;

	auto external_address = connect_target::without_default_port(::ToString(entry.address));

	auto internal_address =
		internal_addr ?
		connect_target::without_default_port(::ToString(*internal_addr)) :
		std::string("Unknown yet")
	;

	text_disabled("Join:");

	auto do_steam_join_link = [&](bool acquire) {
		auto link = steam_link;

		if (acquire) {
			acquire_keyboard_once();
		}

		input_text("##Steam join link", link, rdonly_flags);

		ImGui::SameLine();

		copy_icon_button("SteJoin", link);

		ImGui::SameLine();

		text_color("Steam join link", green);
	};

	auto do_browser_join_link = [&](bool acquire) {
		auto link = ::get_browser_join_link(connect_target { entry.get_web_connect_string(), entry.get_public_key() }.to_web_string());

		if (acquire) {
			acquire_keyboard_once();
		}

		input_text("##Browser join link", link, rdonly_flags);

		ImGui::SameLine();

		copy_icon_button("BroJoin", link);

		ImGui::SameLine();
		text_color("Browser join link", green);
	};

	/*
		A favorite the list does not have now: no browser link either - a browser reaches a server only through the
		list's signalling.
	*/

	if (listed && !entry.favorite_only) {
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
		text("IP address (hidden)");
		ImGui::SameLine();
		copy_icon_button("HiddenExtIp", external_address);

		text("IP address (internal, hidden)");
		ImGui::SameLine();
		copy_icon_button("HiddenIntIp", internal_address);
	}
	else {
		input_text("##IP address", external_address, rdonly_flags);

		ImGui::SameLine();

		copy_icon_button("ExtIp", external_address);

		ImGui::SameLine();

		text("IP address");

		if (external_address != internal_address && heartbeat.internal_network_address.has_value()) {
			input_text("##IP address (internal)", internal_address, rdonly_flags);

			ImGui::SameLine();

			copy_icon_button("IntIp", internal_address);

			ImGui::SameLine();

			text("IP address (internal)");
		}
	}

	auto domain = entry.get_domain_with_port();

	/*
		A community server's domain reveals as much as its IP in the streamer mode.
	*/

	if (!domain.empty() && (do_show_ips || entry.is_official_server())) {
		input_text("##Domain", domain, rdonly_flags);

		ImGui::SameLine();

		copy_icon_button("Domain", domain);

		ImGui::SameLine();

		text("Domain");
	}

	{
		auto webrtc_id = entry.meta.webrtc_id;

		if (!webrtc_id.empty()) {
			input_text("##WebRTC id", webrtc_id, rdonly_flags);

			ImGui::SameLine();

			copy_icon_button("WebId", webrtc_id);

			ImGui::SameLine();

			text("WebRTC id");
		}
	}

	/*
		The server's public key - what the game checks the server by; compare it with what the server's admin publishes
		(e.g. its page on hypersomnia.io).
	*/

	if (const auto key = entry.get_public_key()) {
		auto full = "0x" + augs::bytes_to_hex(*key);
		long_input_text_with_dots<67>("##Server key", full, ImGuiInputTextFlags_ReadOnly);

		ImGui::SameLine();

		copy_icon_button("ServerKey", full);

		ImGui::SameLine();

		text("Server public key");
	}

	if (!listed) {
		ImGui::Separator();
		text_disabled("Server is unlisted.\nNo more data to display.");

		return false;
	}

	auto name = heartbeat.server_name;
	auto arena = heartbeat.current_arena;
	auto version = heartbeat.server_version;

	if (streamer_mode && entry.is_community_server()) {
		name = "Community server";
		arena = "Community arena";
	}

	auto copyable_row = [&](const std::string& label, std::string value) {
		input_text("##" + label, value, rdonly_flags);
		ImGui::SameLine();
		copy_icon_button(label, value);
		ImGui::SameLine();
		text(label);
	};

	copyable_row("Server name", std::string(name));
	copyable_row("Arena", std::string(arena));

	/*
		A favorite the list does not have now: only its name, address and key are known - nothing made up for the rest.
	*/

	if (entry.favorite_only) {
		return false;
	}

	auto nat = nat_type_to_string(heartbeat.nat.type); 

	input_text("NAT type", nat, rdonly_flags);
	auto delta = std::to_string(heartbeat.nat.port_delta);

	if (heartbeat.nat.type != nat_type::PUBLIC_INTERNET) {
		input_text("Port delta", delta, rdonly_flags);
	}

	input_text("Server version", version, rdonly_flags);

	if (heartbeat.speed_hundredths > 0 && heartbeat.bullet_speed_hundredths > 0) {
		auto mult = [](const uint8_t hundredths) {
			return ::format_multiplier(hundredths / 100.f);
		};

		text("Speeds: " + mult(heartbeat.speed_hundredths));
		ImGui::SameLine();
		text_disabled("(game)");
		ImGui::SameLine(0, 0);
		text(",");
		ImGui::SameLine();
		text(mult(heartbeat.bullet_speed_hundredths));
		ImGui::SameLine();
		text_disabled("(bullets)");
	}

	perform_online_players(entry, faction_view, streamer_mode);
	return false;
}

void browse_servers_gui_state::reping_all_servers() {
	for (auto& s : displayed_list()) {
		s.progress = {};
	}

	for (auto& s : favorite_only_list) {
		s.progress = {};
	}
}

void browse_servers_gui_state::update_favorite_only_list(favorite_servers& favorites) {
	auto new_list = std::vector<server_list_entry>();

	/*
		Whether a starred one is official, as the list has it now - its section while the list does not have it.
	*/

	auto now_official = std::vector<std::pair<favorite_server, bool>>();

	for (const auto& f : favorites.all()) {
		const auto& shown_list = displayed_list();

		/*
			One starred on the list: shown here only while the list does not have it. One added by hand: always here,
			never matched against the list.
		*/

		const auto listed_entry = f.manual ? shown_list.end() : std::find_if(shown_list.begin(), shown_list.end(), [&](const auto& s) {
			return s.get_public_key() == f.key && s.get_instance_label() == f.label;
		});

		if (const bool listed = listed_entry != shown_list.end()) {
			if (listed_entry->is_official_server() != f.official) {
				now_official.emplace_back(f, listed_entry->is_official_server());
			}

			continue;
		}

		if (f.address.empty()) {
			continue;
		}

		auto entry = server_list_entry();

		for (const auto& old : favorite_only_list) {
			if (old.unlisted == f.manual && old.meta.server_public_key == f.key && old.get_instance_label() == f.label && old.favorite_address == f.address) {
				entry = old;
			}
		}

		entry.favorite_only = true;
		entry.unlisted = f.manual;
		entry.meta.is_official = f.official;
		entry.meta.server_public_key = f.key;
		entry.heartbeat.instance_label = f.label;
		entry.favorite_address = f.address;
		entry.heartbeat.server_name = f.name.empty() ? "0x" + augs::bytes_to_hex(f.key).substr(0, 8) + "..." : f.name;

#if BUILD_NATIVE_SOCKETS
		/*
			Pinged at the address it was last at - a domain is resolved first, once.
		*/

		auto host = client_connect_string(f.address);
		cut_preffix(host, "rtc:");

		if (const auto parsed = ::find_netcode_addr(host)) {
			entry.address = *parsed;
		}
		else if (const auto resolved = favorite_resolved.find(host); resolved != favorite_resolved.end()) {
			if (resolved->second.has_value()) {
				entry.address = *resolved->second;
			}
			else {
				entry.progress.state = server_entry_state::GIVEN_UP;
			}
		}
		else if (const auto resolving = favorite_resolutions.find(host); resolving != favorite_resolutions.end()) {
			if (::valid_and_is_ready(resolving->second)) {
				const auto result = resolving->second.get();

				favorite_resolved[host] =
					result.result == resolve_result_type::OK ?
					std::optional<netcode_address_t>(result.addr) :
					std::nullopt
				;

				favorite_resolutions.erase(resolving);
			}
		}
		else {
			auto in = host_with_default_port();
			in.address = host;
			favorite_resolutions.emplace(host, ::async_resolve_address(in));
		}
#else
		entry.progress.state = server_entry_state::GIVEN_UP;
#endif

		/*
			A listed server with another key at that address: the favorite is not there (moved, or got a new key).
		*/

		const bool was_elsewhere = !entry.other_server_at_address.empty();
		entry.other_server_at_address.clear();

		for (const auto& s : shown_list) {
			if (f.manual) {
				break;
			}

			const bool same_address =
				(entry.address.type != NETCODE_ADDRESS_NONE && s.address == entry.address)
				|| connect_target::without_default_port(s.get_my_connect_string()) == f.address
				|| connect_target::without_default_port(s.get_native_connect_string(true)) == f.address
			;

			if (same_address && s.get_public_key() != f.key) {
				entry.other_server_at_address = std::string(s.heartbeat.server_name);
				entry.progress.state = server_entry_state::GIVEN_UP;
				break;
			}
		}

		if (was_elsewhere && entry.other_server_at_address.empty()) {
			entry.progress = {};
		}

		new_list.emplace_back(std::move(entry));
	}

	favorite_only_list = std::move(new_list);

	for (const auto& [f, official] : now_official) {
		favorites.set(f.key, f.label, std::string(), std::string(), official);
	}
}

