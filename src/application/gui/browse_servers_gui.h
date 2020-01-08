#pragma once
#include "application/setups/server/server_start_input.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/server/server_instance_type.h"
#include "augs/misc/timing/timer.h"
#include "application/masterserver/server_heartbeat.h"
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include <chrono>

struct server_list_entry {
	int ping = -1;
	netcode_address_t address;
	double appeared_when;
	server_heartbeat data;

	bool is_set() const;
};

struct client_start_input;

struct browse_servers_input {
	const std::string& server_list_provider;
	client_start_input& client_start;
};

struct browse_servers_gui_internal;

struct server_details_gui_state : public standard_window_mixin<server_details_gui_state> {
	using base = standard_window_mixin<server_details_gui_state>;
	using base::base;

	void perform(const server_list_entry&);
};

class browse_servers_gui_state : public standard_window_mixin<browse_servers_gui_state> {
	std::unique_ptr<browse_servers_gui_internal> data;

	std::optional<netcode_address_t> show_server_list();

	net_time_t when_last_downloaded_server_list = 0;

	std::string error_message;
	std::vector<server_list_entry*> filtered_server_list;
	std::vector<server_list_entry> server_list;

	server_list_entry selected_server;
	std::vector<netcode_address_t> official_server_addresses;

	server_details_gui_state server_details = std::string("Server details");

	bool hide_unreachable = false;
	augs::maybe<int> at_least_players = augs::maybe<int>(1, false);

	void refresh_server_list(browse_servers_input);
	void refresh_server_pings();

public:

	using base = standard_window_mixin<browse_servers_gui_state>;
	browse_servers_gui_state(const std::string& title);
	~browse_servers_gui_state();

	bool perform(browse_servers_input);
};
