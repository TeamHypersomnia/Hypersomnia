#pragma once
#include "augs/misc/future.h"
#include <set>
#include <unordered_set>

#include "augs/filesystem/path.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "augs/misc/imgui/simple_popup.h"
#include "augs/network/netcode_socket_raii.h"
#include "augs/misc/randomization.h"
#include "augs/misc/ed25519.h"
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
	INTERFACE,
	GRAPHICS,
	AUDIO,
	CLIENT,
#if !WEB_LOWEND
	SERVER,
	EDITOR,
#endif
	ADVANCED,

	COUNT
	// END GEN INTROSPECTOR
};

enum class audio_settings_pane {
	// GEN INTROSPECTOR enum class audio_settings_pane
	VOLUME,
	SPATIALIZATION,
	HRTF,
	QUALITY,

	COUNT
	// END GEN INTROSPECTOR
};

enum class graphics_settings_pane {
	// GEN INTROSPECTOR enum class graphics_settings_pane
	RENDERING,
	LIGHTING,
	EFFECTS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class interface_settings_pane {
	// GEN INTROSPECTOR enum class interface_settings_pane
	LAYOUT,
	HUD,
	LASER,
	CROSSHAIR,
	MINIMAP,
	FOG_OF_WAR,
	CHAT,
	HUD_SKIN,
	STYLE_EDITOR,

	COUNT
	// END GEN INTROSPECTOR
};

enum class controls_settings_pane {
	// GEN INTROSPECTOR enum class controls_settings_pane
	MOUSE,
	KEYBINDINGS,
	CAMERA,
	INVENTORY,

	COUNT
	// END GEN INTROSPECTOR
};

enum class keybindings_settings_pane {
	// GEN INTROSPECTOR enum class keybindings_settings_pane
	COMBAT,
	GUI,
	INVENTORY,
	APPLICATION,

	COUNT
	// END GEN INTROSPECTOR
};

enum class client_settings_pane {
	// GEN INTROSPECTOR enum class client_settings_pane
	PROFILE,
#if !PLATFORM_WEB
	ACCOUNT,
#endif
	LAG_COMPENSATION,

	COUNT
	// END GEN INTROSPECTOR
};

enum class editor_settings_pane {
	// GEN INTROSPECTOR enum class editor_settings_pane
	AUTOSAVE,
	NODES,
	GRID,
	CAMERA,
	NOTIFICATIONS,
	UPLOADING,

	COUNT
	// END GEN INTROSPECTOR
};

enum class server_settings_pane {
	// GEN INTROSPECTOR enum class server_settings_pane
	GAME,
	MAP_CYCLE,
	SERVER_LIST,
	NETWORK,
	TIME_LIMITS,
	RANKED,
	PASSWORDS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class advanced_settings_pane {
	// GEN INTROSPECTOR enum class advanced_settings_pane
	SYSTEM,
	NETWORK,
	RENDERING,
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
	audio_settings_pane active_audio_pane = audio_settings_pane::VOLUME;
	graphics_settings_pane active_graphics_pane = graphics_settings_pane::RENDERING;
	interface_settings_pane active_interface_pane = interface_settings_pane::LAYOUT;
	controls_settings_pane active_controls_pane = controls_settings_pane::MOUSE;
	keybindings_settings_pane active_keybindings_pane = keybindings_settings_pane::COMBAT;
	client_settings_pane active_client_pane = client_settings_pane::PROFILE;
	editor_settings_pane active_editor_pane = editor_settings_pane::AUTOSAVE;
	server_settings_pane active_server_pane = server_settings_pane::GAME;
	advanced_settings_pane active_advanced_pane = advanced_settings_pane::SYSTEM;

#if BUILD_NETWORKING
	stun_manager_window stun_manager = std::string("STUN manager");
#endif

	std::optional<simple_popup> already_bound_popup;

	key_hijack_request reassignment_request;
	key_hijack_request hijacking;
	std::optional<bool> separate_sensitivity_axes;

	std::vector<std::string> official_hrtf_presets;
	std::vector<std::string> user_hrtf_presets;

	std::string import_key_text;
	std::string link_code_text;
	std::string account_status;
	bool confirm_forget_key = false;
	std::optional<augs::ed25519_seed_type> import_to_confirm;

	std::optional<augs::ed25519_seed_type> account_seed;
	double when_account_seed_loaded = -1.0;
	bool account_seed_reload_needed = true;
	augs::future<std::string> link_request;

	struct link_target {
		std::string code;
		std::string steam_account;
		std::string name;
		std::string error;
	};

	augs::future<link_target> link_target_request;
	std::optional<link_target> link_target_to_confirm;

	void perform_account_pane(augs::window& window, const std::string& website_url);

public:
	bool is_steam_client = false;

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