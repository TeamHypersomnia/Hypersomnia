#pragma once
#include "augs/filesystem/path.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/editor/editor_popup.h"

struct config_lua_table;

enum class settings_pane {
	// GEN INTROSPECTOR enum class settings_pane
	GENERAL,
	RENDERING,
	AUDIO,
	CONTROLS,
	GAMEPLAY,
	CLIENT,
	SERVER,
	EDITOR,
	GUI,
	PERFORMANCE,
	DEBUG,

	COUNT
	// END GEN INTROSPECTOR
};

struct key_hijack_request {
	bool for_secondary = false;
	std::optional<int> for_idx;
	std::optional<augs::event::keys::key> captured;
};

namespace augs {
	class audio_context;
};

#include "augs/misc/randomization.h"
#include "application/nat/stun_server_provider.h"
#include "application/nat/stun_session.h"
#include <set>

class stun_server_tester {
	netcode_socket_raii socket;
	randomization rng;

public:
	stun_server_provider provider;
	std::vector<std::unique_ptr<stun_session>> current_sessions;

	stun_server_tester(const stun_server_provider&);
	std::set<std::pair<double, std::string>> resolved_servers;

	int num_failed_servers = 0;

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

	stun_manager_window stun_manager = std::string("STUN manager");

	std::optional<editor_popup> already_bound_popup;

	key_hijack_request reassignment_request;
	key_hijack_request hijacking;

public:
	using base = standard_window_mixin<settings_gui_state>;
	using base::base;

	bool should_hijack_key() const;
	void set_hijacked_key(augs::event::keys::key);

	void perform(
		sol::state& lua,
		const augs::audio_context& audio,
		const augs::path_type& path_for_saving,
		const config_lua_table& canon_config,
		config_lua_table& into,
		config_lua_table& last_saved,
		vec2i screen_size
	);
};

namespace augs {
	class window;
	class audio_context;
	class renderer;
	struct window_settings;
}

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

	void apply(const config_lua_table&) const;

	void apply_main_thread(const augs::window_settings&) const;
	void sync_back_into(config_lua_table&) const;
};