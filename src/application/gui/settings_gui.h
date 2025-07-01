#pragma once
#include <set>
#include <unordered_set>

#include "augs/filesystem/path.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "augs/misc/imgui/simple_popup.h"
#include "augs/network/netcode_socket_raii.h"
#include "augs/misc/randomization.h"
#include "application/nat/stun_server_provider.h"
#include "application/nat/stun_session.h"
#include "application/masterserver/netcode_address_hash.h"

struct config_json_table;

namespace augs {
	class audio_context;
	class window;
	class renderer;
	struct window_settings;
};

enum class settings_pane {
	// GEN INTROSPECTOR enum class settings_pane
	GENERAL,
	CONTROLS,
	GAMEPLAY,
	GRAPHICS,
	AUDIO,
	CLIENT,
#if !WEB_LOWEND
	EDITOR,
#endif
	INTERFACE,
	ADVANCED,

	COUNT
	// END GEN INTROSPECTOR
};

struct key_hijack_request {
	bool for_secondary = false;
	std::optional<int> for_idx;
	std::optional<augs::event::keys::key> captured;
};

class stun_server_tester {
	randomization rng;

	std::unordered_set<netcode_address_t> resolved_stun_hosts;
	std::unordered_set<netcode_address_t> resolved_my_addresses;

public:
	netcode_socket_raii socket;
	netcode_packet_queue packet_queue;

	stun_server_provider provider;
	std::vector<std::unique_ptr<stun_session>> current_sessions;

	stun_server_tester(const stun_server_provider&, port_type source_port);
	std::set<std::tuple<double, std::string, std::string>> resolved_servers;

	int num_failed_servers = 0;
	int num_duplicate_servers = 0;
	int num_duplicate_resolved_addresses = 0;

	void advance();
};

struct stun_manager_window : public standard_window_mixin<stun_manager_window> {
	using base = standard_window_mixin<stun_manager_window>;
	using base::base;

	std::optional<stun_server_provider> all_candidates;
	std::optional<stun_server_tester> tester;

	void perform();
};

class settings_gui_state : public standard_window_mixin<settings_gui_state> {
	settings_pane active_pane = settings_pane::GENERAL;

#if BUILD_NETWORKING
	stun_manager_window stun_manager = std::string("STUN manager");
#endif

	std::optional<simple_popup> already_bound_popup;

	key_hijack_request reassignment_request;
	key_hijack_request hijacking;
	std::optional<bool> separate_sensitivity_axes;

public:
	using base = standard_window_mixin<settings_gui_state>;
	using base::base;

	vec2i display_size_for_clipping;

	bool should_hijack_key() const;
	void set_hijacked_key(augs::event::keys::key);

	void perform(
		augs::window& window,
		const augs::audio_context& audio,
		const augs::path_type& path_for_saving,
		const config_json_table& canon_config,
		const config_json_table& canon_config_with_confd,
		config_json_table& into,
		config_json_table& last_saved,
		vec2i screen_size
	);
};

struct all_necessary_fbos;
struct all_necessary_shaders;

struct all_necessary_sounds;
struct necessary_image_definitions_map;

struct configuration_subscribers {
	augs::window& window;
	all_necessary_fbos& fbos;
	augs::audio_context& audio_context;
	augs::renderer& renderer;

#if TODO
	all_necessary_shaders& shaders;
	const all_necessary_sounds& sounds;
	const necessary_image_definitions_map& images;
#endif

	void apply(const config_json_table&) const;

	void apply_main_thread(const augs::window_settings&) const;
	void sync_back_into(config_json_table&) const;
};