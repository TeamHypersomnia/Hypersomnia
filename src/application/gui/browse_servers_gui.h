#pragma once
#include "application/setups/server/server_start_input.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/server/server_instance_type.h"
#include "augs/misc/timing/timer.h"
#include "application/masterserver/server_heartbeat.h"
#include "3rdparty/yojimbo/netcode.io/netcode.h"

struct server_list_entry {
	int ping = -1;
	netcode_address_t address;
	server_heartbeat data;
};

struct browse_servers_input {
	const std::string& server_list_provider;
};

struct browse_servers_gui_internal;

class browse_servers_gui_state : public standard_window_mixin<browse_servers_gui_state> {
	std::unique_ptr<browse_servers_gui_internal> data;

	void show_server_list();

	net_time_t when_last_downloaded_server_list = 0;
	bool show_only_responding_servers = false;

	std::string error_message;
	std::vector<server_list_entry*> filtered_server_list;
	std::vector<server_list_entry> server_list;

public:

	using base = standard_window_mixin<browse_servers_gui_state>;
	browse_servers_gui_state(const std::string& title);
	~browse_servers_gui_state();

	void refresh_server_list(browse_servers_input);
	void perform(browse_servers_input);
};
