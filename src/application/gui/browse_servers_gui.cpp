#include <future>

#include "application/gui/browse_servers_gui.h"
#include "augs/network/netcode_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "3rdparty/cpp-httplib/httplib.h"
#include "augs/templates/thread_templates.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/setups/client/client_start_input.h"
#include "augs/log.h"
#include "application/setups/editor/detail/maybe_different_colors.h"
#include "augs/misc/time_utils.h"
#include "augs/network/netcode_sockets.h"
#include "application/masterserver/nat_puncher_client.h"
#include "application/masterserver/nat_punch_provider_settings.h"

constexpr auto nat_request_interval = 0.5;
constexpr auto address_resolution_interval = 0.5;
constexpr auto ping_retry_interval = 1;
constexpr auto ping_nat_keepalive_interval = 10;
constexpr auto reopen_nat_after_seconds = 15;
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
	return data.server_name.size() > 0;
}

browse_servers_gui_state::~browse_servers_gui_state() = default;

struct browse_servers_gui_internal {
	std::optional<httplib::Client> http;
	std::future<std::shared_ptr<httplib::Response>> future_response;
	netcode_socket_t socket;

	std::future<official_addrs> future_official_addresses;

	nat_puncher_client nat;

	bool refresh_op_in_progress() const {
		return future_official_addresses.valid() || future_response.valid() || nat.is_resolving_host();
	}
};


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

	my_network_details.reset();

	official_server_addresses.clear();
	error_message.clear();
	server_list.clear();
	selected_server = {};

	auto& http_opt = data->http;

	data->nat.resolve_relay_host(in.nat_punch_provider.address);

	data->future_official_addresses = launch_async(
		[addresses=in.official_arena_servers]() {
			official_addrs results;

			LOG("Resolving %x official arena hosts for the server list.", addresses.size());

			for (const auto& a : addresses) {
				address_and_port in;

				in.address = a;
				in.default_port = 0;

				const auto result = resolve_address(in);
				result.report();

				if (result.result == resolve_result_type::OK) {
					results.emplace_back(result);
				}
			}

			return results;
		}
	);

	data->future_response = launch_async(
		[&http_opt, address = in.server_list_provider]() -> std::shared_ptr<Response> {
			const auto resolved = resolve_address(address);
			resolved.report();

			if (resolved.result != resolve_result_type::OK) {
				return nullptr;
			}

			auto resolved_addr = resolved.addr;
			const auto intended_port = resolved_addr.port;
			resolved_addr.port = 0;

			const auto address_str = ::ToString(resolved_addr);
			const auto timeout = 5;

			LOG("Connecting to server list at ip (no port): %x", address_str);

			http_opt.emplace(address_str.c_str(), intended_port, timeout);

			auto progress = [](uint64_t len, uint64_t total) {
				(void)len;
				(void)total;

				return true;
			};

			auto& http = *http_opt;
			http.follow_location(true);

			return http.Get("/server_list_binary", progress);
		}
	);

	when_last_downloaded_server_list = yojimbo_time();
}

void browse_servers_gui_state::refresh_server_pings() {
	for (auto& s : server_list) {
		s.progress = {};
	}
}

bool server_list_entry::is_behind_nat() const {
	return address != data.internal_network_address;
}

server_list_entry* browse_servers_gui_state::find(const netcode_address_t& address) {
	for (auto& s : server_list) {
		if (s.address == address) {
			return &s;
		}
	}

	return nullptr;
}

server_list_entry* browse_servers_gui_state::find_by_internal_network_address(const netcode_address_t& address, const uint64_t ping_sequence) {
	for (auto& s : server_list) {
		if (s.progress.ping_sequence == ping_sequence) {
			if (s.data.internal_network_address == address) {
				return &s;
			}
		}
	}

	return nullptr;
}

static bool is_internal(const netcode_address_t& address) {
	if (address.type == NETCODE_ADDRESS_IPV4) {
		auto ip = address.data.ipv4;

		if (ip[0] == 127) {
			return true;
		}

		if (ip[0] == 10) {
			return true;
		}

		if (ip[0] == 172) {
			if (ip[1] >= 16) {
				if (ip[2] <= 31) {
					return true;
				}
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

void my_network_details_gui_state::handle_response(const netcode_address_t& resolution_host, const netcode_address_t& resolved_addr) {
	auto check_resolved = [&](auto& into) {
		if (into.destination == resolution_host)
		{
			into.resolved = resolved_addr;
		}
	};

	check_resolved(info.first);
	check_resolved(info.second);
}

void browse_servers_gui_state::advance_ping_and_nat_logic(const browse_servers_input in) {
	if (in.nat_puncher_socket == nullptr) {
		return;
	}

	auto udp_socket = *in.nat_puncher_socket;
	auto& nat = data->nat;

	nat.advance_relay_host_resolution();

	const auto current_time = yojimbo_time();

	if (nat.relay_host_resolved())
	{
		my_network_details.handle_requesting_resolution(udp_socket, *nat.relay_host_addr, in.nat_punch_provider.extra_address_resolution_port);
	}

	{

		const auto num_dots = uint64_t(current_time * 3) % 3 + 1;
		loading_dots = std::string(num_dots, '.');
	}

	{
		struct netcode_address_t from;
		uint8_t packet_buffer[NETCODE_MAX_PACKET_BYTES];

		while (true) {
			const auto packet_bytes = netcode_socket_receive_packet(&udp_socket, &from, packet_buffer, NETCODE_MAX_PACKET_BYTES);

			if (packet_bytes < 1) {
				break;
			}

			BRW_LOG("Packet arrived!");

			const auto command = packet_buffer[0];

			if (command == uint8_t(masterserver_udp_command::TELL_ME_MY_ADDRESS)) {
				if (packet_bytes != 1 + sizeof(netcode_address_t)) {
					BRW_LOG("Wrong num of bytes for TELL_ME_MY_ADDRESS: %x!", packet_bytes);
					continue;
				}

				netcode_address_t resolved_address;
				std::memcpy(&resolved_address, packet_buffer + 1, sizeof(netcode_address_t));

				BRW_LOG("TELL_ME_MY_ADDRESS response!");
				my_network_details.handle_response(from, resolved_address);
			}
			else if (command == NETCODE_PING_RESPONSE_PACKET) {
				if (packet_bytes != 1 + sizeof(uint64_t)) {
					BRW_LOG("Wrong num of bytes for NETCODE_PING_RESPONSE_PACKET: %x!", packet_bytes);
					continue;
				}

				BRW_LOG("Ping response!");

				uint64_t sequence;
				std::memcpy(&sequence, packet_buffer + 1, sizeof(uint64_t));

				if (const auto entry = find(from)) {
					auto& progress = entry->progress;

					BRW_LOG("Has entry! Sequence: %x; entry sequence: %x", sequence, progress.ping_sequence);

					if (sequence == uint64_t(-1)) {
						BRW_LOG("Punched server: %x", ::ToString(from));
						progress.state = server_entry_state::PUNCHED;
					}
					else if (sequence == progress.ping_sequence) {
						BRW_LOG("Sequence matches!");
						progress.set_ping_from(current_time);
						progress.state = server_entry_state::PING_MEASURED;
					}
				}

				if (const auto entry = find_by_internal_network_address(from, sequence)) {
					auto& progress = entry->progress;

					BRW_LOG("Found a server on the internal network! Sequence: %", sequence);
					BRW_LOG("Sequence matches!");

					progress.set_ping_from(current_time);
					progress.state = server_entry_state::PING_MEASURED;
					progress.found_on_internal_network = true;
				}
			}
		}
	}

	int packets_left = max_packets_per_frame_v;

	if (server_list.empty()) {
		return;
	}

	for (auto& s : server_list) {
		if (packets_left <= 0) {
			break;
		}

		const bool behind_nat = s.is_behind_nat();

		auto& p = s.progress;

		using S = server_entry_state;

		auto request_ping = [&]() {
			p.when_sent_last_ping = current_time;
			p.ping_sequence = ping_sequence_counter++;

			send_ping_request(udp_socket, s.address, p.ping_sequence);
			--packets_left;

			const auto& internal_address = s.data.internal_network_address;
			const bool maybe_reachable_internally = 
				behind_nat
				&& internal_address != std::nullopt
				&& is_internal(*internal_address)
			;

			if (maybe_reachable_internally) {
				send_ping_request(udp_socket, *internal_address, p.ping_sequence);
				--packets_left;
			}
		};

		if (p.state == S::AWAITING_RESPONSE) {
			if (behind_nat) {
				if (nat.relay_host_resolved()) {
					auto& when_first = p.when_first_nat_request;
					auto& when_last = p.when_last_nat_request;

					if (when_last == 0 || current_time - when_last >= nat_request_interval) {
						when_last = current_time;

						if (when_first == 0) {
							when_first = current_time;
						}

						nat.punched_server_addr = s.address;
						nat.request_punch(udp_socket);

						--packets_left;
					}

					if (when_first != 0 && current_time - when_first >= server_entry_timeout) {
						p.state = server_entry_state::GIVEN_UP;
						continue;
					}
				}
			}
		}

		if (p.state == S::AWAITING_RESPONSE || p.state == S::PUNCHED) {
			auto& when_first = p.when_sent_first_ping;
			auto& when_last = p.when_sent_last_ping;

			if (when_last == 0 || current_time - when_last >= ping_retry_interval) {
				if (when_first == 0) {
					when_first = current_time;
				}

				request_ping();
			}

			if (when_first != 0 && current_time - when_first >= server_entry_timeout) {
				p.state = server_entry_state::GIVEN_UP;
				continue;
			}
		}

		if (p.state == S::PING_MEASURED) {
			auto& when_last = p.when_sent_last_ping;

			auto reping_if_its_time = [&]() {
				if (!show) {
					/* Never reping if the window is hidden. */
					return;
				}

				if (current_time - when_last >= ping_nat_keepalive_interval) {
					request_ping();
				}
			};

			if (behind_nat) {
				if (current_time - when_last >= reopen_nat_after_seconds * 2) {
					/* 
						If it weren't pinged for such a long time for any reason,
						consider the NAT holes dead. Refresh the server.
					*/

					p = {};
				}
				else {
					reping_if_its_time();
				}
			}
			else {
				reping_if_its_time();
			}
		}
	}
}

constexpr auto num_columns = 7;

void browse_servers_gui_state::show_server_list(const std::string& label, const std::vector<server_list_entry*>& server_list) {
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
		const auto& d = s.data;

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

		if (ImGui::IsItemClicked()) {
			if (ImGui::IsMouseDoubleClicked(0)) {
				LOG("Double-clicked server list entry: %x (%x). Connecting.", ToString(s.address), d.server_name);

				if (s.progress.found_on_internal_network) {
					requested_connection = s.data.internal_network_address;
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

		if (d.game_mode.is_set()) {
			const auto game_mode_name = d.game_mode.dispatch([](auto d) {
				using D = decltype(d);
				return format_field_name(get_type_name<D>());
			});

			do_text(game_mode_name);
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

		const auto secs_ago = augs::date_time::secs_since_epoch() - s.appeared_when;
		text_disabled(augs::date_time::format_how_long_ago(true, secs_ago));

		ImGui::NextColumn();
	}
}

bool successful(const int http_status_code);

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
	if (!show) {
		return false;
	}

	using namespace augs::imgui;

	centered_size_mult = vec2(0.75f, 0.6f);

	auto imgui_window = make_scoped_window();

	if (!imgui_window) {
		return false;
	}

	const auto couldnt_download = std::string("Couldn't download the server list.\n");

	auto handle_response = [&](auto response) {
		if (response == nullptr) {
			error_message = "Couldn't connect to the server list host.";
			return;
		}

		const auto status = response->status;

		LOG("Server list response status: %x", status);

		if (!successful(status)) {
			error_message = couldnt_download + "HTTP response: " + std::to_string(status);
			return;
		}

		const auto& bytes = response->body;

		LOG("Server list response bytes: %x", bytes.size());

		const auto buffer_ptr = augs::cpointer_to_buffer{ reinterpret_cast<const std::byte*>(bytes.data()), bytes.size() };

		auto ss = augs::cptr_memory_stream{ buffer_ptr };

		try {
			while (ss.get_unread_bytes() > 0) {
				server_list_entry entry;

				augs::read_bytes(ss, entry.address);
				augs::read_bytes(ss, entry.appeared_when);
				augs::read_bytes(ss, entry.data);

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
		const auto& name = s.data.server_name;
		const auto& arena = s.data.current_arena;
		const auto effective_name = std::string(name) + " " + std::string(arena);

		auto push_if_passes = [&](auto& target_list) {
			if (only_responding) {
				if (s.progress.ping == -1) {
					return;
				}
			}

			if (at_least_players.is_enabled) {
				if (s.data.num_fighting < at_least_players.value) {
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
					s.data.server_name = resolved->host;
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
			return a->data.server_name < b->data.server_name;
		};

		auto by_mode = [](const T& a, const T& b) {
			return a->data.game_mode < b->data.game_mode;
		};

		auto by_players = [](const T& a, const T& b) {
			return a->data.num_fighting < b->data.num_fighting;
		};

		auto by_spectators = [](const T& a, const T& b) {
			return a->data.get_num_spectators() < b->data.get_num_spectators();
		};

		auto by_arena = [](const T& a, const T& b) {
			return a->data.current_arena < b->data.current_arena;
		};

		auto by_appeared = [](const T& a, const T& b) {
			return a->appeared_when > b->appeared_when;
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

	if (when_last_downloaded_server_list == 0) {
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
		ImGui::SetColumnWidth(1, ImGui::CalcTextSize("9").x * 80);
		ImGui::SetColumnWidth(2, ImGui::CalcTextSize("9").x * 30);
		ImGui::SetColumnWidth(3, ImGui::CalcTextSize("Players  9").x);
		ImGui::SetColumnWidth(4, ImGui::CalcTextSize("Spectators  9").x);
		ImGui::SetColumnWidth(5, ImGui::CalcTextSize("9").x * (max_arena_name_length_v + 1));
		ImGui::SetColumnWidth(6, ImGui::CalcTextSize("9").x * 25);

		int current_col = 0;

		auto do_column = [&](std::string label, std::optional<rgba> col = std::nullopt) {
			const bool is_current = current_col == sort_by_column;

			if (is_current) {
				label += ascending ? "▲" : "▼";
			}

			const auto& style = ImGui::GetStyle();

			rgba final_color = is_current ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled];

			if (col != std::nullopt) {
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
			show_server_list("local", local_server_list);
		}

		if (!has_local_servers) {
			do_column_labels(official_servers_label, yellow);
		}
		else {
			separate_with_label_only(official_servers_label, yellow);
		}

		if (has_official_servers) {
			show_server_list("official", official_server_list);
		}
		else {
			ImGui::NextColumn();
			text_disabled("No official servers are online at the time.");
			next_columns(num_columns - 1);
		}

		separate_with_label_only(community_servers_label, orange);

		if (has_community_servers) {
			show_server_list("community", community_server_list);
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
				requested_connection = selected_server.address;
			}

			ImGui::SameLine();

			if (ImGui::Button("Server details")) {
				server_details.open();
			}

			ImGui::SameLine();
		}

		if (ImGui::Button("My network details")) {
			my_network_details.open();
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
	
	server_details.perform(selected_server);
	my_network_details.perform();

	if (requested_connection != std::nullopt) {
		in.client_start.set_custom(::ToString(*requested_connection));

		return true;
	}

	return false;
}

void my_network_details_gui_state::reset() {
	info = {};
}

void my_network_details_gui_state::handle_requesting_resolution(netcode_socket_t& udp_socket, const netcode_address_t& resolution_host, const port_type extra_port) {
	auto handle_addr_resolution = [&](auto& info, const std::optional<port_type> alternate_port = std::nullopt) {
		auto& when_last = info.when_last;

		if (info.resolved != std::nullopt)
		{
			return;
		}

		const auto current_time = yojimbo_time();

		if (when_last == 0 || current_time - when_last >= address_resolution_interval)
		{
			auto serv_addr = resolution_host;

			if (alternate_port != std::nullopt)
			{
				serv_addr.port = *alternate_port;
			}

			info.destination = serv_addr;

			::tell_me_my_address(udp_socket, serv_addr);

			when_last = current_time;
		}
	};

	handle_addr_resolution(info.first);
	handle_addr_resolution(info.second, extra_port);
}

void my_network_details_gui_state::perform() {
	using namespace augs::imgui;

	auto window = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!window) {
		return;
	}

	text_disabled("Tuples: Destination address:NAT mapped address");

	auto show_line = [&](const auto& info) {
		if (info.when_last == 0) {
			return;
		}

		auto dest_address = ::ToString(info.destination);
		input_text("###Destination", dest_address, ImGuiInputTextFlags_ReadOnly);

		ImGui::SameLine();

		auto nat_mapped_address = info.resolved ? ::ToString(*info.resolved) : std::string("Resolving...");
		input_text("###NatMapped", nat_mapped_address, ImGuiInputTextFlags_ReadOnly);
	};

	show_line(info.first);
	auto id = scoped_id(1);
	show_line(info.second);
}

void server_details_gui_state::perform(const server_list_entry& entry) {
	using namespace augs::imgui;

	auto window = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!window) {
		return;
	}

	if (!entry.is_set()) {
		text_disabled("No server selected.");
		return;
	}

	const auto& internal_addr = entry.data.internal_network_address;

	auto data = entry.data;
	auto external_address = ::ToString(entry.address);
	auto internal_address = internal_addr ? ::ToString(*internal_addr) : std::string("Unknown yet");

	acquire_keyboard_once();
	input_text("External IP address", external_address, ImGuiInputTextFlags_ReadOnly);
	input_text("Internal IP address", internal_address, ImGuiInputTextFlags_ReadOnly);

	input_text("Server name", data.server_name, ImGuiInputTextFlags_ReadOnly);

	input_text("Arena", data.current_arena, ImGuiInputTextFlags_ReadOnly);
}

void browse_servers_gui_state::request_nat_reopen() {
	for (auto& s : server_list) {
		if (s.is_behind_nat()) {
			s.progress = {};
		}
	}
}
