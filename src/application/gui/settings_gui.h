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
	GUI_STYLES,
	PERFORMANCE,
	ADVANCED,

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

class settings_gui_state : public standard_window_mixin<settings_gui_state> {
	settings_pane active_pane = settings_pane::GENERAL;


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