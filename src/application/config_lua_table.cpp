#include "generated/introspectors.h"

#include "augs/log.h"
#include "augs/misc/lua_readwrite.h"
#include "augs/templates/introspect.h"
#include "augs/templates/corresponding_field.h"

#include "application/game_window.h"
#include "application/config_lua_table.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_session.h"
#include "game/detail/gui/character_gui.h"

#include "augs/misc/script_utils.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/platform_utils.h"

config_lua_table::config_lua_table(const std::string& config_lua_path) {
	augs::load_from_lua_table(*this, config_lua_path);
}

void config_lua_table::save(const std::string& target_path) const {
	augs::save_as_lua_table(*this, target_path);
}

launch_type config_lua_table::get_launch_mode() const {
	return launch_mode;
}

input_recording_type config_lua_table::get_input_recording_mode() const {
	return input_recording_mode;
}

vec2i config_lua_table::get_screen_size() const {
	return fullscreen ? augs::window::get_display().get_size() : windowed_size;
}

void apply_changes(
	const config_lua_table& config,
	const config_lua_table& origin,
	viewing_session& session,
	const bool force
) {
	auto changed = [&](auto& field) {
		return !(field == augs::get_corresponding_field(field, config, origin));
	};

	if (force 
		|| config.get_screen_size() != origin.get_screen_size()
		|| changed(config.window_border)
	) {
		session.set_screen_size(config.get_screen_size());
	}
}

void apply_changes(
	const config_lua_table& config,
	const config_lua_table& origin,
	augs::window::glwindow& window,
	 bool force
) {
	auto changed = [&](auto& field) {
		return !(field == augs::get_corresponding_field(field, config, origin));
	};

	if (force || changed(config.window_name)) {
		window.set_window_name(config.window_name);
	}

	if (force || changed(config.window_position)) {
		auto r = window.get_window_rect();
		r.set_position(config.window_position);
		window.set_window_rect(r);
	}

	if (force || changed(config.enable_cursor_clipping)) {
		augs::window::set_cursor_visible(!config.fullscreen && !config.enable_cursor_clipping);
	}

	if (force || changed(config.window_border)) {
		window.set_window_border_enabled(!config.fullscreen && config.window_border);
	}

	if (force || changed(config.fullscreen)) {
		if (config.fullscreen) {
			window.set_window_border_enabled(false);
			augs::window::set_cursor_visible(false);

			augs::window::set_display(config.get_screen_size(), config.bpp);
		}
		else {
			window.set_window_border_enabled(config.window_border);
			augs::window::set_cursor_visible(!config.enable_cursor_clipping);
		}
	}

	if (force
		|| config.get_screen_size() != origin.get_screen_size()
		|| changed(config.window_border)
	) {
		xywhi screen_rect;

		if (!config.fullscreen) {
			screen_rect.set_position(config.window_position);
		}

		screen_rect.set_size(config.get_screen_size());
		window.set_window_rect(screen_rect);
	}
}

void apply_changes(
	const config_lua_table& config,
	const config_lua_table& origin,
	augs::renderer& renderer,
	const bool force
) {
	if (force || config.get_screen_size() != origin.get_screen_size()) {
		renderer.resize_fbos(config.get_screen_size());
	}
}

void sync_config_back(config_lua_table& config, const augs::window::glwindow& window) {
	if (!config.fullscreen) {
		config.windowed_size = window.get_window_rect().get_size();
		config.window_position = window.get_window_rect().get_position();
	}
}