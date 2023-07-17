#include <future>

#include "application/gui/browse_servers_gui.h"
#include "augs/network/netcode_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "3rdparty/include_httplib.h"
#include "augs/templates/thread_templates.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/setups/client/client_start_input.h"
#include "augs/log.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "augs/misc/time_utils.h"
#include "augs/network/netcode_sockets.h"
#include "application/nat/nat_detection_settings.h"
#include "application/network/resolve_address.h"
#include "application/masterserver/masterserver_requests.h"
#include "application/masterserver/gameserver_command_readwrite.h"
#include "augs/misc/httplib_utils.h"

constexpr auto ping_retry_interval = 1;
constexpr auto reping_interval = 10;
constexpr auto server_entry_timeout = 5;
constexpr auto max_packets_per_frame_v = 64;

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

bool server_list_entry::is_set() const {
	return heartbeat.server_name.size() > 0;
}

struct browse_servers_gui_internal {
	std::future<std::optional<httplib::Result>> future_response;
	netcode_socket_t socket;

	std::future<official_addrs> future_official_addresses;

	bool refresh_op_in_progress() const {
		return future_official_addresses.valid() || future_response.valid();
	}
};

browse_servers_gui_state::~browse_servers_gui_state() = default;

browse_servers_gui_state::browse_servers_gui_state(const std::string& title) 
	: base(title), data(std::make_unique<browse_servers_gui_internal>()) 
{

}

double yojimbo_time();

void browse_servers_gui_state::refresh_server_list(const browse_servers_input in) {
	using namespace httplib;

	if (data->refresh_op_in_progress()) {
		return;
	}

	official_server_addresses.clear();
	error_message.clear();
	server_list.clear();
	auto prev_addr = selected_server.address;
	selected_server = {};
	selected_server.address = prev_addr;

	data->future_official_addresses = launch_async(
		[addresses=in.official_arena_servers]() {
			official_addrs results;

			LOG("Resolving %x official arena hosts for the server list.", addresses.size());

			for (const auto& a : addresses) {
				address_and_port in;

				in.address = a;
				in.default_port = 0;

				const auto result = resolve_address(in);
				LOG(result.report());

				if (result.result == resolve_result_type::OK) {
					results.emplace_back(result);
				}
			}

			return results;
		}
	);

	data->future_response = launch_async(
		[address = in.server_list_provider]() -> std::optional<httplib::Result> {
			const auto resolved = resolve_address(address);
			LOG(resolved.report());

			if (resolved.result != resolve_result_type::OK) {
				return std::nullopt;
			}

			auto resolved_addr = resolved.addr;
			const auto intended_port = resolved_addr.port;
			resolved_addr.port = 0;

			const auto address_str = ::ToString(resolved_addr);
			const auto timeout = 5;

			LOG("Connecting to server list at: %x:%x", address_str, intended_port);

			httplib::Client cli(address_str.c_str(), intended_port);
			cli.set_write_timeout(timeout);
			cli.set_read_timeout(timeout);
			cli.set_follow_location(true);

			return cli.Get("/server_list_binary");
		}
	);

	when_last_started_refreshing_server_list = yojimbo_time();
}

void browse_servers_gui_state::refresh_server_pings() {
	for (auto& s : server_list) {
		s.progress = {};
	}
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

bool is_internal(const netcode_address_t& address) {
	if (address.type == NETCODE_ADDRESS_IPV4) {
		auto ip = address.data.ipv4;

		if (ip[0] == 127) {
			return true;
		}

		if (ip[0] == 10) {
			return true;
		}

		if (ip[0] == 172) {
			if (ip[1] >= 16 && ip[1] <= 31) {
				return true;
			}
		}

		if (ip[0] == 192) {
			if (ip[1] == 168) {
				return true;
			}
		}
	}

	// TODO
	return false;
}

bool browse_servers_gui_state::handle_gameserver_response(const netcode_address_t& from, uint8_t* packet_buffer, std::size_t packet_bytes) {
	const auto current_time = yojimbo_time();

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

void browse_servers_gui_state::handle_incoming_udp_packets(netcode_socket_t& socket) {
	auto packet_handler = [&](const auto& from, uint8_t* buffer, const int bytes_received) {
		if (handle_gameserver_response(from, buffer, bytes_received)) {
			return;
		}
	};

	::receive_netcode_packets(socket, packet_handler);
}

void browse_servers_gui_state::animate_dot_column() {
	const auto current_time = yojimbo_time();

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
	const auto current_time = yojimbo_time();

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

constexpr auto num_columns = 7;

void browse_servers_gui_state::show_server_list(const std::string& label, const std::vector<server_list_entry*>& server_list, const faction_view_settings& faction_view) {
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
		const auto color = rgba(given_up ? style.Colors[ImGuiCol_TextDisabled] : style.Colors[ImGuiCol_Text]);

		auto do_text = [&](const auto& t) {
			text_color(t, color);
		};

		const bool is_selected = selected_server.address == s.address;

		{
			const auto ping = progress.ping;

			if (ping != -1) {
				if (ping > 999) {
					do_text("999>");
				}
				else {
					do_text(typesafe_sprintf("%x", ping));
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

		{
			auto col_scope = scoped_style_color(ImGuiCol_Text, color);

			if (ImGui::Selectable(d.server_name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
				selected_server = s;
			}
		}

		if (ImGui::IsItemHovered()) {
			auto tool = scoped_tooltip();

			if (s.heartbeat.num_online == 0) {
				text_disabled("No players online.");
			}
			else {
				text_color(typesafe_sprintf("%x players online:", s.heartbeat.num_online), green);

				auto do_faction_summary = [&](auto faction, auto& entries) {
					if (entries.empty()) {
						return;
					}

					ImGui::Separator();

					const auto labcol = faction_view.colors[faction].current_player_text;

					for (auto& e : entries) {
						text_color(e.nickname, labcol);
					}
				};

				do_faction_summary(faction_type::RESISTANCE, s.heartbeat.players_resistance);
				do_faction_summary(faction_type::METROPOLIS, s.heartbeat.players_metropolis);
				do_faction_summary(faction_type::SPECTATOR, s.heartbeat.players_spectating);
			}
		}
		
		if (ImGui::IsItemClicked()) {
			if (ImGui::IsMouseDoubleClicked(0)) {
				LOG("Double-clicked server list entry: %x (%x). Connecting.", ToString(s.address), d.server_name);

				displayed_connecting_server_name = s.heartbeat.server_name;

				if (s.progress.found_on_internal_network) {
					requested_connection = s.heartbeat.internal_network_address;
				}
				else {
					requested_connection = s.address;
				}
			}
		}

		if (ImGui::IsItemClicked(1)) {
			selected_server = s;
			server_details.open();
		}

		ImGui::NextColumn();

		if (d.game_mode.size() > 0) {
			do_text(std::string(d.game_mode));
		}
		else {
			do_text("<unknown>");
		}

		ImGui::NextColumn();

		const auto max_players = std::min(d.max_fighting, d.max_online);
		do_text(typesafe_sprintf("%x/%x", d.num_fighting, max_players));

		ImGui::NextColumn();

		do_text(typesafe_sprintf("%x/%x", d.get_num_spectators(), d.get_max_spectators()));

		ImGui::NextColumn();
		do_text(d.current_arena);

		ImGui::NextColumn();

		const auto secs_ago = augs::date_time::secs_since_epoch() - s.time_hosted;
		text_disabled(augs::date_time::format_how_long_ago(true, secs_ago));

		ImGui::NextColumn();
	}
}

const resolve_address_result* browse_servers_gui_state::find_resolved_official(const netcode_address_t& n) {
	for (const auto& o : official_server_addresses) {
		auto compared_for_officiality = o.addr;
		compared_for_officiality.port = 0;

		auto candidate = n;
		candidate.port = 0;

		if (candidate == compared_for_officiality) {
			return &o;
		}
	}

	return nullptr;
}

bool browse_servers_gui_state::perform(const browse_servers_input in) {
	using namespace httplib_utils;

	if (!show) {
		return false;
	}

	using namespace augs::imgui;

	centered_size_mult = vec2(0.8f, 0.7f);

	auto imgui_window = make_scoped_window(ImGuiWindowFlags_NoSavedSettings);

	if (!imgui_window) {
		return false;
	}

	const auto couldnt_download = std::string("Couldn't download the server list.\n");

	auto handle_response = [&](const auto& result) {
		if (result == std::nullopt || result.value() == nullptr) {
			error_message = "Couldn't connect to the server list host.";
			return;
		}

		const auto& response = result.value();
		const auto status = response->status;

		LOG("Server list response status: %x", status);

		if (!successful(status)) {
			error_message = couldnt_download + "HTTP response: " + std::to_string(status);
			return;
		}

		const auto& bytes = response->body;

		LOG("Server list response bytes: %x", bytes.size());

		auto stream = augs::make_ptr_read_stream(bytes.data(), bytes.size());

		try {
			while (stream.has_unread_bytes()) {
				server_list_entry entry;

				augs::read_bytes(stream, entry.address);
				augs::read_bytes(stream, entry.time_hosted);
				augs::read_bytes(stream, entry.heartbeat);

				server_list.emplace_back(std::move(entry));
			}
		}
		catch (const augs::stream_read_error& err) {
			error_message = "There was a problem deserializing the server list:\n" + std::string(err.what()) + "\n\nTry restarting the game and updating your client!";
			server_list.clear();
		}
	};

	if (valid_and_is_ready(data->future_response)) {
		handle_response(data->future_response.get());
	}

	if (valid_and_is_ready(data->future_official_addresses)) {
		official_server_addresses = data->future_official_addresses.get();
	}

	thread_local ImGuiTextFilter filter;

	filter.Draw();
	local_server_list.clear();
	official_server_list.clear();
	community_server_list.clear();

	bool has_local_servers = false;
	bool has_official_servers = false;
	bool has_community_servers = false;

	for (auto& s : server_list) {
		const auto& name = s.heartbeat.server_name;
		const auto& arena = s.heartbeat.current_arena;
		const auto effective_name = std::string(name) + " " + std::string(arena);

		auto push_if_passes = [&](auto& target_list) {
			if (only_responding) {
				if (s.progress.ping == -1) {
					return;
				}
			}

			if (at_least_players.is_enabled) {
				if (s.heartbeat.num_fighting < at_least_players.value) {
					return;
				}
			}

			if (at_most_ping.is_enabled) {
				if (s.progress.ping > at_most_ping.value) {
					return;
				}
			}

			if (!filter.PassFilter(effective_name.c_str())) {
				return;
			}

			target_list.push_back(&s);
		};

		const bool is_internal = s.progress.found_on_internal_network;

		if (is_internal) {
			has_local_servers = true;
			push_if_passes(local_server_list);
		}
		else {
			if (const auto resolved = find_resolved_official(s.address)) {
				has_official_servers = true;
				push_if_passes(official_server_list);

				if (std::string(name) == "Player's server") {
					s.heartbeat.server_name = resolved->host;
				}
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

		auto by_players = [](const T& a, const T& b) {
			return a->heartbeat.num_fighting < b->heartbeat.num_fighting;
		};

		auto by_spectators = [](const T& a, const T& b) {
			return a->heartbeat.get_num_spectators() < b->heartbeat.get_num_spectators();
		};

		auto by_arena = [](const T& a, const T& b) {
			return a->heartbeat.current_arena < b->heartbeat.current_arena;
		};

		auto by_appeared = [](const T& a, const T& b) {
			return a->time_hosted > b->time_hosted;
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
					return sort_by(by_mode, by_ping, by_appeared);

				case 3: 
					return sort_by(by_players, by_ping, by_appeared);

				case 4: 
					return sort_by(by_spectators, by_ping, by_appeared);

				case 5: 
					return sort_by(by_arena, by_ping, by_appeared);

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

	requested_connection = std::nullopt;

	auto do_list_view = [&](bool disable_content_view) {
		auto child = scoped_child("list view", ImVec2(0, 2 * -(ImGui::GetFrameHeightWithSpacing() + 4)));

		if (disable_content_view) {
			return;
		}

		ImGui::Columns(num_columns);
		ImGui::SetColumnWidth(0, ImGui::CalcTextSize("9999999").x);
		ImGui::SetColumnWidth(1, ImGui::CalcTextSize("9").x * max_server_name_length_v);
		ImGui::SetColumnWidth(2, ImGui::CalcTextSize("Capture the flag").x);
		ImGui::SetColumnWidth(3, ImGui::CalcTextSize("Players  9").x);
		ImGui::SetColumnWidth(4, ImGui::CalcTextSize("Spectators  9").x);
		ImGui::SetColumnWidth(5, ImGui::CalcTextSize("9").x * (max_arena_name_length_v / 2));
		ImGui::SetColumnWidth(6, ImGui::CalcTextSize("9").x * 25);

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
			do_column("Game mode");
			do_column("Players");
			do_column("Spectators");
			do_column("Arena");
			do_column("First appeared");

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
			show_server_list("local", local_server_list, in.faction_view);
		}

		if (!has_local_servers) {
			do_column_labels(official_servers_label, yellow);
		}
		else {
			separate_with_label_only(official_servers_label, yellow);
		}

		if (has_official_servers) {
			show_server_list("official", official_server_list, in.faction_view);
		}
		else {
			ImGui::NextColumn();
			text_disabled("No official servers are online at the time.");
			next_columns(num_columns - 1);
		}

		separate_with_label_only(community_servers_label, orange);

		if (has_community_servers) {
			show_server_list("community", community_server_list, in.faction_view);
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

		if (data->future_response.valid()) {
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

		checkbox("At least", at_least_players.is_enabled);

		{
			auto& val = at_least_players.value;
			auto scoped = maybe_disabled_cols({}, !at_least_players.is_enabled);
			auto width = scoped_item_width(ImGui::CalcTextSize("9").x * 4);

			ImGui::SameLine();
			auto input = std::to_string(val);
			input_text(val == 1 ? "player###numbox" : "players###numbox", input);
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
			input_text(val == 1 ? "ping###numbox" : "ping###numbox", input);
			val = std::clamp(std::atoi(input.c_str()), 0, int(999));
		}

		{
			auto scope = maybe_disabled_cols({}, !selected_server.is_set());

			if (ImGui::Button("Connect")) {
				auto& s = selected_server;

				LOG("Double-clicked server list entry: %x (%x). Connecting.", ToString(s.address), s.heartbeat.server_name);

				displayed_connecting_server_name = s.heartbeat.server_name;

				if (s.progress.found_on_internal_network) {
					requested_connection = s.heartbeat.internal_network_address;
				}
				else {
					requested_connection = s.address;
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Server details")) {
				server_details.open();
			}

			ImGui::SameLine();
		}

		ImGui::SameLine();

		{
			auto scope = maybe_disabled_cols({}, data->refresh_op_in_progress());

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

	if (server_details.perform(selected_server, in.faction_view)) {
		refresh_server_list(in);
	}

	if (requested_connection.has_value()) {
		in.client_start.set_custom(::ToString(*requested_connection));
		in.client_start.displayed_connecting_server_name = displayed_connecting_server_name;

		return true;
	}

	return false;
}

const server_list_entry* browse_servers_gui_state::find_entry(const client_start_input& in) const {
	if (const auto connected_address = to_netcode_addr(in.get_address_and_port())) {
		LOG("Finding the server entry by: %x", ::ToString(*connected_address));
		LOG("Number of servers in the browser: %x", server_list.size());

		for (auto& s : server_list) {
			if (s.address == *connected_address) {
				return &s;
			}
		}
	}

	return nullptr;
}

void server_details_gui_state::perform_online_players(const server_list_entry& entry, const faction_view_settings& faction_view) {
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

	for (auto& e : heartbeat.players_resistance) { 
		longest_nname = std::max(longest_nname, e.nickname.length());
	}
	for (auto& e : heartbeat.players_metropolis) { 
		longest_nname = std::max(longest_nname, e.nickname.length());
	}
	for (auto& e : heartbeat.players_spectating) { 
		longest_nname = std::max(longest_nname, e.nickname.length());
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
			text_color(e.nickname, col);
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

bool server_details_gui_state::perform(const server_list_entry& entry, const faction_view_settings& faction_view) {
	using namespace augs::imgui;
	//ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiCond_FirstUseEver);
	auto window = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!window) {
		return false;
	}

	if (!entry.is_set()) {
		text_disabled("No server selected.");
		return false;
	}

	if (ImGui::Button("Refresh")) {
		return true;
	}

	ImGui::Separator();

	auto heartbeat = entry.heartbeat;
	const auto& internal_addr = heartbeat.internal_network_address;

	auto external_address = ::ToString(entry.address);
	auto internal_address = internal_addr ? ::ToString(*internal_addr) : std::string("Unknown yet");

	acquire_keyboard_once();

	const bool censor_ips = true;

	if (censor_ips) {
		if (ImGui::Button("Copy External IP address to clipboard")) {
			ImGui::SetClipboardText(external_address.c_str());
		}

		if (ImGui::Button("Copy Internal IP address to clipboard")) {
			ImGui::SetClipboardText(internal_address.c_str());
		}
	}
	else {
		input_text("External IP address", external_address, ImGuiInputTextFlags_ReadOnly);
		input_text("Internal IP address", internal_address, ImGuiInputTextFlags_ReadOnly);
	}

	input_text("Server name", heartbeat.server_name, ImGuiInputTextFlags_ReadOnly);

	input_text("Arena", heartbeat.current_arena, ImGuiInputTextFlags_ReadOnly);

	auto nat = nat_type_to_string(heartbeat.nat.type); 

	input_text("NAT type", nat, ImGuiInputTextFlags_ReadOnly);
	auto delta = std::to_string(heartbeat.nat.port_delta);

	if (heartbeat.nat.type != nat_type::PUBLIC_INTERNET) {
		input_text("Port delta", delta, ImGuiInputTextFlags_ReadOnly);
	}

	perform_online_players(entry, faction_view);
	return false;
}

void browse_servers_gui_state::reping_all_servers() {
	for (auto& s : server_list) {
		s.progress = {};
	}
}
