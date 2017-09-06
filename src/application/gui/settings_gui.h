#pragma once
#include "augs/filesystem/path.h"

struct config_lua_table;

class settings_gui_state {
	int active_pane = 0;

public:
	bool show = false;
	
	void perform(
		const augs::path_type& path_for_saving,
		config_lua_table& into,
		config_lua_table& last_saved
	);
};

namespace augs {
	class window;
	class renderer;
	class audio_context;
}

struct necessary_fbos;
struct necessary_shaders;

struct necessary_sound_buffers;
struct necessary_image_definitions;

struct configuration_subscribers {
	augs::window& window;
	necessary_fbos& fbos;
	augs::audio_context& audio_context;

#if TODO
	augs::renderer& renderer;
	necessary_shaders& shaders;
	const necessary_sound_buffers& sounds;
	const necessary_image_definitions& images;
#endif

	void apply(const config_lua_table&) const;
	void sync_back_into(config_lua_table&) const;
};