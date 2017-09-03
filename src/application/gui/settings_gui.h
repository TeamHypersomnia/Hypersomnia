#pragma once
#include "augs/filesystem/path.h"

struct config_lua_table;

class settings_gui_state {
	bool show = false;
	int active_pane = 0;

public:
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

struct fbo_collection;
struct shader_collection;

struct sound_buffer_collection;
struct requisite_image_collection;

struct configuration_subscribers {
	augs::window& window;
	fbo_collection& fbos;
	augs::audio_context& audio_context;

#if TODO
	augs::renderer& renderer;
	shader_collection& shaders;
	const sound_buffer_collection& sounds;
	const requisite_image_collection& images;
#endif

	void apply(const config_lua_table&) const;
	void sync_back_into(config_lua_table&) const;
};