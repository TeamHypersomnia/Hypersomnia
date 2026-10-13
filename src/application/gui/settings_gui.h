#pragma once
#include "augs/misc/future.h"
#include <set>
#include <unordered_set>

#include "augs/filesystem/path.h"
#include "augs/image/image.h"
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
#include "application/main/loopback_link_listener.h"
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
	BACKUP,

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

	std::optional<augs::ed25519_seed_type> import_to_confirm;


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
		bool has_avatar = false;
	};

	augs::async_response_ptr link_target_request;
	std::string link_target_code;

	std::string steam_link_state;

	/*
		Native: where the browser comes back after the sign-in through Steam.
	*/

	loopback_link_listener link_listener;
	int link_listener_port = 0;
	std::optional<std::string> link_code_from_steam;

	/*
		"Link with Steam..." runs in one dimmed modal, from asking whether the key is linked already to the result.
	*/

	enum class link_flow_type {
		IDLE,
		CHECKING_BEFORE,
		NEEDS_STEAM_SIGN_IN,
		LINKING,
		CHECKING_AFTER,
		DONE
	};

	link_flow_type link_flow = link_flow_type::IDLE;
	std::string link_flow_message;
	std::string linked_name;
	bool linked_has_avatar = false;
	bool link_flow_succeeded = false;
	bool link_flow_conflict = false;
	bool link_flow_message_is_warning = false;

	/*
		The site's answer to the link did not arrive (or it failed on its side): whether it linked is asked after.
	*/

	bool link_answer_lost = false;

	/*
		What the site said when it failed on its side (a 5xx with a reason), shown if the link did not happen.
	*/

	std::string link_lost_reason;

	/*
		A key just linked (or a linked key imported) plays with the Steam name and avatar - the toggle is turned on and saved.
	*/

	bool account_profile_wanted = false;

	bool import_modal_open = false;
	bool import_profile_applied = false;

	/*
		Confirmations and results of the Backup and Link panes, each in its own modal.
	*/

	bool confirm_forget_key = false;
	bool reveal_private_key_open = false;
	bool confirm_unlink = false;
	bool unlink_has_avatar = false;

	/*
		The unlink modal stays open for the answer: "Unlinking..." in place of the question, then the result.
	*/

	bool unlinking = false;
	std::optional<std::string> unlink_result;
	bool unlink_succeeded = false;
	std::string unlinked_name;

	/*
		Decodes an account's avatar into link_avatar_to_upload - true if there is one to show.
	*/

	bool upload_link_avatar(const std::vector<std::byte>& bytes);
	std::optional<std::string> account_message;

	augs::future<std::optional<augs::path_type>> chosen_avatar;

	/*
		The private key file chosen to restore from - its text (empty if it could not be read).
	*/

	augs::future<std::optional<std::string>> chosen_key_file;

	/*
		Where to save a copy of the key file (Backup - Save a copy...).
	*/

	augs::future<std::optional<augs::path_type>> chosen_key_copy_path;
	bool no_save_dialog_note = false;

	void perform_account_link_pane(const std::string& website_url);
	void perform_account_manage_pane(augs::window& window);

	void perform_import_modal(
		config_json_table& config,
		config_json_table& last_saved_config,
		const config_json_table& canon_config_with_confd,
		const augs::path_type& config_path_for_saving
	);

	void perform_account_message_modal();

	std::optional<augs::ed25519_seed_type> current_account_seed();
	std::optional<augs::ed25519_public_key_type> current_public_key();

	/*
		Linked to Steam: a saved Steam cert, or signed in to a Steam account right now.
	*/

	bool is_key_linked() const;

	/*
		Why the key or its link may not change now, if so: a ranked match on or one that can still be rejoined
		(read from the disk only when asked, on a click), or the browser's storage not writable.
	*/

	std::optional<std::string> account_change_blocked() const;

public:
	/*
		Any modal of the Account settings - another modal waits while one is open (two would close each other every frame).
	*/

	bool account_modal_open(bool with_message = true) const;

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
		When Refresh was last clicked (unix time) - kept in the cache folder, so a restart does not let it be clicked
		again before hypersomnia.io's minute is over. Read once, on the first draw.
	*/

	std::optional<int64_t> profile_refreshed_at;

	/*
		account.key was just replaced or forgotten here - the game reads it again.
	*/

	/*
		CrazyGames' "Sign in" in Info - opened by work.cpp, like the leaderboards' one.
	*/

	bool wants_crazygames_sign_in = false;
	bool key_file_changed = false;
	bool key_file_unreadable_with_key_in_use = false;

	/*
		Set by the game: a ranked match is on - the key may not be replaced meanwhile.
	*/

	bool in_ranked_match = false;

	/*
		Set by the game: the avatar preview shows the account's avatar, not one's own - only then is it shown as the account's.
	*/

	bool account_avatar_previewed = false;

	/*
		The avatar of the Steam account a link code is for, decoded here - the game uploads it as the LINK_AVATAR texture.
	*/

	std::optional<augs::image> link_avatar_to_upload;

	/*
		For icons, e.g. the Steam logo on its tab. Set by the game.
	*/

	const necessary_images_in_atlas_map* necessary_images = nullptr;

	/*
		Web: the code Sign in with Steam came back with - taken only for the state this game opened it with.
	*/

	void on_steam_link_code(const std::string& code, const std::string& state);

	/*
		A key not seen before appeared in account.key (pasted there by hand) - shown exactly as an import.
	*/

	void on_key_replaced_outside();

	/*
		account.key does not read and there is no key to play with: Backup, with what to do.
	*/

	void on_key_unreadable();

	void cancel_link_flow_if_hidden();

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