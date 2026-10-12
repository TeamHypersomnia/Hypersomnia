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
#include "augs/misc/async_response.h"
#include "application/nat/stun_server_provider.h"
#include "application/nat/stun_session.h"
#include "application/masterserver/netcode_address_hash.h"
#include "application/main/identity_session.h"
#include "view/necessary_resources.h"

struct config_json_table;

namespace augs {
	class audio_context;
	class window;
	class renderer;
	struct window_settings;
};

enum class settings_pane {
	// GEN INTROSPECTOR enum class settings_pane
	ACCOUNT,
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
	GENERAL,
	LAG_COMPENSATION,

	COUNT
	// END GEN INTROSPECTOR
};

enum class account_settings_pane {
	// GEN INTROSPECTOR enum class account_settings_pane
	INFO,
	LINK_WITH_STEAM,
	MANAGE,

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
	settings_pane active_pane = settings_pane::ACCOUNT;
	audio_settings_pane active_audio_pane = audio_settings_pane::VOLUME;
	graphics_settings_pane active_graphics_pane = graphics_settings_pane::RENDERING;
	interface_settings_pane active_interface_pane = interface_settings_pane::LAYOUT;
	controls_settings_pane active_controls_pane = controls_settings_pane::MOUSE;
	keybindings_settings_pane active_keybindings_pane = keybindings_settings_pane::COMBAT;
	client_settings_pane active_client_pane = client_settings_pane::GENERAL;
	account_settings_pane active_account_pane = account_settings_pane::INFO;
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
	std::optional<augs::ed25519_seed_type> import_to_confirm;

	std::optional<augs::ed25519_seed_type> account_seed;
	double when_account_seed_loaded = -1.0;
	bool account_seed_reload_needed = true;

	/*
		A key file that does not read is told only once it stayed so for a moment -
		right after Forget, the game makes the new key while this pane may already look.
	*/

	double key_unreadable_since = -1.0;

	augs::async_response_ptr link_request;
	bool link_request_is_unlink = false;

	struct link_target {
		std::string code;
		std::string steam_account;
		std::string name;
	};

	augs::async_response_ptr link_target_request;
	std::string link_target_code;
	std::optional<link_target> link_target_to_confirm;

	std::string steam_link_state;
	std::optional<std::string> link_code_from_steam;

	/*
		"Link to your Steam account" runs in one dimmed modal, from asking whether the key is linked already to the result.
	*/

	enum class link_flow_type {
		IDLE,
		CHECKING_BEFORE,
		NEEDS_STEAM_SIGN_IN,
		CONFIRMING,
		LINKING,
		CHECKING_AFTER,
		DONE
	};

	link_flow_type link_flow = link_flow_type::IDLE;
	std::string link_flow_message;
	std::string linked_name;

	/*
		The code already sent as typed - a failed one is not sent again until it changes.
	*/

	std::string auto_requested_code;
	bool link_flow_succeeded = false;

	bool import_modal_open = false;
	bool import_profile_applied = false;

	/*
		Confirmations and results of the Manage and Link panes, each in its own modal.
	*/

	bool confirm_forget_key = false;
	bool confirm_unlink = false;
	std::optional<std::string> account_message;

	augs::future<std::optional<augs::path_type>> chosen_avatar;

	void perform_account_link_pane(const std::string& website_url);
	void perform_account_manage_pane(augs::window& window);

	void perform_import_modal(
		config_json_table& config,
		config_json_table& last_saved_config,
		const config_json_table& canon_config_with_confd,
		const augs::path_type& config_path_for_saving
	);

	void perform_account_message_modal();

public:
	bool is_steam_client = false;

	/*
		Set by the game every frame: the account the player is known as, and why the identity broker last refused or failed.
	*/

	std::string current_account_id;
	std::string identity_error;

	/*
		The identity session and what the game feeds it, for linking, importing and showing the account.
	*/

	identity_session* identity = nullptr;
	identity_session::input identity_input;

	/*
		[Retry] pressed here - the game retries, and tells a failure again.
	*/

	bool sign_in_retry_wanted = false;

	/*
		One's own avatar was chosen here - the game shows it again even if its path is the same.
	*/

	bool avatar_changed = false;

	/*
		For icons, e.g. the Steam logo on its tab. Set by the game.
	*/

	const necessary_images_in_atlas_map* necessary_images = nullptr;

	/*
		Web: the code Sign in with Steam came back with - taken only for the state this game opened it with.
	*/

	void on_steam_link_code(const std::string& code, const std::string& state);

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