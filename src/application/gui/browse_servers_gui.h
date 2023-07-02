#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/server/server_instance_type.h"
#include "augs/misc/timing/timer.h"
#include "application/masterserver/server_heartbeat.h"
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "augs/network/netcode_sockets.h"
#include "augs/network/netcode_socket_raii.h"
#include "view/faction_view_settings.h"

#include <chrono>

struct client_start_input;
struct nat_detection_settings;
struct browse_servers_gui_internal;
struct netcode_socket_t;
struct address_and_port;
struct resolve_address_result;

using official_addrs = std::vector<resolve_address_result>;

enum class server_entry_state {
	GIVEN_UP,
	AWAITING_RESPONSE,
	PING_MEASURED
};

struct ping_progress {
	int ping = -1;
	uint64_t ping_sequence = -1;

	net_time_t when_sent_first_ping = -1;
	net_time_t when_sent_last_ping = -1;

	server_entry_state state = server_entry_state::AWAITING_RESPONSE;

	bool found_on_internal_network = false;

	void set_ping_from(const net_time_t current_time) {
		ping = static_cast<int>((current_time - when_sent_last_ping) * 1000);
	}
};

struct server_list_entry {
	netcode_address_t address;
	double time_hosted;
	server_heartbeat heartbeat;

	ping_progress progress;

	bool is_set() const;
	bool is_behind_nat() const;
};

struct browse_servers_input {
	const address_and_port& server_list_provider;
	client_start_input& client_start;
	const std::vector<std::string>& official_arena_servers;
	const faction_view_settings& faction_view;
};

struct server_details_gui_state : public standard_window_mixin<server_details_gui_state> {
	using base = standard_window_mixin<server_details_gui_state>;
	using base::base;

	bool perform(const server_list_entry&, const faction_view_settings&);
	void perform_online_players(const server_list_entry&, const faction_view_settings&);
};

class browse_servers_gui_state : public standard_window_mixin<browse_servers_gui_state> {
	netcode_socket_raii server_browser_socket;

	std::unique_ptr<browse_servers_gui_internal> data;

	void show_server_list(const std::string& label, const std::vector<server_list_entry*>&, const faction_view_settings&);
	std::optional<netcode_address_t> requested_connection;
	std::string displayed_connecting_server_name;

	net_time_t when_last_started_refreshing_server_list = 0;

	std::string error_message;

	std::vector<server_list_entry*> local_server_list;
	std::vector<server_list_entry*> official_server_list;
	std::vector<server_list_entry*> community_server_list;

	std::vector<server_list_entry> server_list;

	server_list_entry selected_server;
	official_addrs official_server_addresses;

	server_details_gui_state server_details = std::string("Server details");

	bool only_responding = false;
	augs::maybe<int> at_least_players = augs::maybe<int>(1, false);
	augs::maybe<int> at_most_ping = augs::maybe<int>(100, false);

	int sort_by_column = 0;
	bool ascending = true;
	std::string loading_dots;
	uint64_t ping_sequence_counter = 0;

	void refresh_server_list(browse_servers_input);
	void refresh_server_pings();

	server_list_entry* find_entry(const netcode_address_t&);
	server_list_entry* find_entry_by_internal_address(const netcode_address_t&, uint64_t ping_sequence);

	const resolve_address_result* find_resolved_official(const netcode_address_t&);

	bool handle_gameserver_response(const netcode_address_t& from, uint8_t* packet_buffer, std::size_t packet_bytes);

	void animate_dot_column();
	void handle_incoming_udp_packets(netcode_socket_t&);
	void send_pings_and_punch_requests(netcode_socket_t&);

public:

	using base = standard_window_mixin<browse_servers_gui_state>;
	browse_servers_gui_state(const std::string& title);
	~browse_servers_gui_state();

	bool perform(browse_servers_input);
	void advance_ping_logic();

	void reping_all_servers();

	const server_list_entry* find_entry(const client_start_input& in) const;
};
