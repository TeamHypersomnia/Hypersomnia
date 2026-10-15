#pragma once
#include <cstddef>
#include <cstdint>
#include "augs/math/vec2.h"
#include "augs/filesystem/path.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "augs/misc/timing/timer.h"
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "augs/network/netcode_sockets.h"
#include "augs/network/netcode_socket_raii.h"
#include "application/setups/client/client_connect_string.h"
#include "view/faction_view_settings.h"
#include "application/gui/server_list_entry.h"
#include "application/gui/browser_tab.h"
#include "augs/misc/ed25519.h"

#include <chrono>
#include <future>
#include <unordered_map>
#include "application/network/resolve_address_result.h"

struct nat_detection_settings;
struct browse_servers_gui_internal;
struct netcode_socket_t;
class favorite_servers;

struct browse_servers_input {
	const std::string& server_list_provider;
	client_connect_string& client_connect;
	std::string& displayed_connecting_server_name;

	const faction_view_settings& faction_view;
	const bool streamer_mode;

	favorite_servers& favorites;

	/*
		The tab remembered in gui_state.json - read on the first look, kept up to date.
	*/

	browser_tab& remembered_tab;

	/*
		The last server list downloaded - shown at once on the next start, while a fresh one downloads.
	*/

	const augs::path_type& list_cache_path;
};

struct server_details_gui_state : public standard_window_mixin<server_details_gui_state> {
	using base = standard_window_mixin<server_details_gui_state>;
	using base::base;

	bool show_ips = false;

	/*
		Edit pressed for a favorite added by hand - the server browser opens its dialog.
	*/

	bool edit_requested = false;

	void open() {
		show_ips = false;
		base::open();
	}

	bool perform(const server_list_entry&, const faction_view_settings&, const bool streamer_mode, favorite_servers* favorites = nullptr);
	void perform_online_players(const server_list_entry&, const faction_view_settings&, const bool streamer_mode);

private:
	using base::open;
};

/*
	Invite to join: the server played on, from what the game knows of it - no waiting for the server list.
	The list (if it has this key) only adds the public address (joined over the local network) and an official alias.
*/

struct invite_to_join_input {
	std::string server_name;

	/*
		Where the player joined (or, hosting, where the server is) and its key - the address empty if not known yet.
	*/

	std::string address;
	std::optional<augs::ed25519_public_key_type> server_key;
	std::string server_label;

	const server_list_entry* listed;
	favorite_servers& favorites;
	const bool streamer_mode;
	const bool hosting;
};

/*
	Where a friend joins: from the list entry of this key if there is one, else the address played at.
*/

struct invite_target {
	std::string native_address;
	std::string web_id;
	std::optional<augs::ed25519_public_key_type> key;
	bool local_network_only = false;

	/*
		Joined over the local network while the list knows the public address - for others in the same network.
	*/

	std::string local_address;

	static invite_target of(const std::string& address, const std::optional<augs::ed25519_public_key_type>& key, const server_list_entry* listed);

	std::string join_string() const;
};

struct invite_to_join_gui_state : public standard_window_mixin<invite_to_join_gui_state> {
	using base = standard_window_mixin<invite_to_join_gui_state>;
	using base::base;

	void perform(const invite_to_join_input&);
};

class browse_servers_gui_state : public standard_window_mixin<browse_servers_gui_state> {
#if BUILD_NATIVE_SOCKETS
	netcode_socket_raii server_browser_socket;
#endif

	std::unique_ptr<browse_servers_gui_internal> data;

	void show_server_list(
		const std::string& label,
		const std::vector<server_list_entry*>&,
		const browse_servers_input& in
	);

	/*
		The favorites not on the server list - pinged at the address each was last at.
	*/

	std::vector<server_list_entry> favorite_only_list;
	std::vector<server_list_entry*> favorite_only_shown;

#if BUILD_NATIVE_SOCKETS
	std::unordered_map<std::string, std::future<resolve_address_result>> favorite_resolutions;
	std::unordered_map<std::string, std::optional<netcode_address_t>> favorite_resolved;
#endif

	void update_favorite_only_list(const favorite_servers&);

	/*
		Remembered in gui_state.json. Local only while there are local servers (seen in the last frame).
	*/

	browser_tab current_tab = browser_tab::FAVORITES;
	bool had_local_servers = false;
	bool tab_loaded = false;
	bool shown_once = false;
	double first_shown_at = 0.0;
	bool local_tab_precedence_pending = true;

	/*
		Add a server (the Favorites tab).
	*/

	bool add_server_open = false;
	bool add_server_row_pinned = false;

	bool cache_loaded = false;
	void load_cached_list_once(const browse_servers_input&);
	std::string add_server_name;
	std::string add_server_address;
	std::string add_server_key;

	/*
		The same dialog editing a favorite added by hand: its key and address before the edit.
	*/

	std::optional<std::pair<augs::ed25519_public_key_type, std::string>> editing_unlisted;

	std::optional<std::string> requested_connection;
	std::string displayed_connecting_server_name;

	std::optional<server_list_entry> pending_password_entry;
	std::string password_prompt_input;

	net_time_t when_last_started_refreshing_server_list = 0;
	net_time_t when_last_updated_time_to_events = 0;

	std::string error_message;

	std::vector<server_list_entry*> local_server_list;
	std::vector<server_list_entry*> tournament_server_list;
	std::vector<server_list_entry*> official_server_list;
	std::vector<server_list_entry*> community_server_list;

	std::vector<server_list_entry> server_list;

	/*
		The last list downloaded, from the disk - only shown in the browser until a fresh one comes in this run.
		Joins, quick play, invites and keys are found only in the fresh one (server_list).
	*/

	std::vector<server_list_entry> cached_list;
	bool fresh_list_arrived = false;

	std::vector<server_list_entry>& displayed_list() {
		return fresh_list_arrived ? server_list : cached_list;
	}

	bool scroll_once_to_selected = false;
	server_list_entry selected_server;

	server_details_gui_state server_details = std::string("Server details");

	bool only_responding = false;
	bool no_password = false;
#if IS_PRODUCTION_BUILD
	bool show_incompatible = false;
#else
	bool show_incompatible = true;
#endif
	augs::maybe<int> at_least_players = augs::maybe<int>(1, false);
	augs::maybe<uint32_t> at_most_ping = augs::maybe<uint32_t>(100, false);

	int sort_by_column = 2;
	bool ascending = false;
	std::string loading_dots;
	uint64_t ping_sequence_counter = 0;

	void refresh_server_pings();

	server_list_entry* find_entry(const netcode_address_t&, uint64_t ping_sequence);
	server_list_entry* find_entry_by_internal_address(const netcode_address_t&, uint64_t ping_sequence);

	bool handle_gameserver_response(const netcode_address_t& from, uint8_t* packet_buffer, std::size_t packet_bytes);

	void animate_dot_column();
#if BUILD_NATIVE_SOCKETS
	void handle_incoming_udp_packets(netcode_socket_t&);
	void send_pings_and_punch_requests(netcode_socket_t&);
#endif
public:
	/*
		Why this game can not play ranked (itch.io, a CrazyGames guest) - ranked servers are not joined then, and say why.
	*/

	std::string ranked_unavailable_reason;



	using base = standard_window_mixin<browse_servers_gui_state>;
	browse_servers_gui_state(const std::string& title);
	~browse_servers_gui_state();

	bool perform(browse_servers_input);

	/*
		Every frame, menu or not: the cached list at the start, the downloads - so an invite or a join by key
		has the list in the game too.
	*/

	void advance_list(const browse_servers_input&);

	/*
		The list entry of this key, if the list (or the cached one) has it.
	*/

	const server_list_entry* find_entry_by_key(const augs::ed25519_public_key_type&, const std::string& label = "") const;

#if BUILD_NATIVE_SOCKETS
	void advance_ping_logic();
#endif

	void reping_all_servers();

	const server_list_entry* find_entry_by_connect_string(const client_connect_string& in) const;
	std::optional<server_list_entry> find_best_server(bool allow_ranked) const;

	/*
		Quick play: the best casual server, skipping the ones behind NAT unless there is nothing else.
	*/

	std::optional<server_list_entry> find_quick_play_server() const;

	bool server_list_failed() const {
		return !error_message.empty();
	}
	void refresh_server_list(browse_servers_input);

	bool refreshed_at_least_once() const;

	void select_server(const server_list_entry&);

	bool refresh_in_progress() const;

	/*
		A join to a server with a password and none saved asks for it right before connecting.
		The entered password, with the server it was entered for.
	*/

	void ask_password(const server_list_entry&);
	std::optional<std::pair<std::string, server_list_entry>> perform_password_prompt();
};
