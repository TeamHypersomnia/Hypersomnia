#pragma once
#include "application/setups/server/server_start_input.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/server/server_instance_type.h"
#include "augs/misc/timing/timer.h"
#include "application/masterserver/server_heartbeat.h"
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include <chrono>

struct netcode_socket_t;
struct address_and_port;

enum class server_entry_state {
	GIVEN_UP,

	AWAITING_RESPONSE,

	PUNCHED,
	PING_MEASURED
};

struct nat_progress {
	int ping = -1;
	uint64_t ping_sequence = -1;

	net_time_t when_sent_first_ping = 0;
	net_time_t when_sent_last_ping = 0;

	net_time_t when_last_nat_request = 0;
	net_time_t when_first_nat_request = 0;

	server_entry_state state = server_entry_state::AWAITING_RESPONSE;

	bool found_on_internal_network = false;
};

struct server_list_entry {
	netcode_address_t address;
	net_time_t appeared_when;
	server_heartbeat data;

	nat_progress progress;

	bool is_set() const;
	bool is_behind_nat() const;
};

struct client_start_input;

struct browse_servers_input {
	const address_and_port& server_list_provider;
	const address_and_port& nat_punch_provider;
	client_start_input& client_start;
	const netcode_socket_t* nat_puncher_socket;
};

struct browse_servers_gui_internal;

struct server_details_gui_state : public standard_window_mixin<server_details_gui_state> {
	using base = standard_window_mixin<server_details_gui_state>;
	using base::base;

	void perform(const server_list_entry&);
};

class browse_servers_gui_state : public standard_window_mixin<browse_servers_gui_state> {
	std::unique_ptr<browse_servers_gui_internal> data;

	void show_server_list(const std::vector<server_list_entry*>&);
	std::optional<netcode_address_t> requested_connection;

	net_time_t when_last_downloaded_server_list = 0;

	std::string error_message;

	std::vector<server_list_entry*> local_server_list;
	std::vector<server_list_entry*> official_server_list;
	std::vector<server_list_entry*> community_server_list;

	std::vector<server_list_entry> server_list;

	server_list_entry selected_server;
	std::vector<netcode_address_t> official_server_addresses;

	server_details_gui_state server_details = std::string("Server details");

	bool only_responding = false;
	augs::maybe<int> at_least_players = augs::maybe<int>(1, false);

	int sort_by_column = 0;
	bool ascending = true;
	std::string loading_dots;
	uint64_t ping_sequence_counter = 0;

	void refresh_server_list(browse_servers_input);
	void refresh_server_pings();

	server_list_entry* find(const netcode_address_t&);
	server_list_entry* find_by_internal_network_address(const netcode_address_t&, uint64_t ping_sequence);

public:

	using base = standard_window_mixin<browse_servers_gui_state>;
	browse_servers_gui_state(const std::string& title);
	~browse_servers_gui_state();

	bool perform(browse_servers_input);
	void advance_ping_and_nat_logic(browse_servers_input);

	void request_nat_reopen();
};
