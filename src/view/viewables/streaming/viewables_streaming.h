#pragma once
#include <optional>
#include <future>
#include <vector>

#define EXPERIMENTAL_USE_PBO 0

#if EXPERIMENTAL_USE_PBO
#include "augs/graphics/pbo.h"
#endif

#include "augs/image/font.h"
#include "augs/texture_atlas/atlas_profiler.h"
#include "augs/graphics/renderer.h"
#include "view/viewables/streaming/viewables_streaming_profiler.h"
#include "view/viewables/all_viewables_defs.h"
#include "view/viewables/image_cache.h"
#include "view/viewables/loaded_sounds_map.h"

#include "view/viewables/atlas_distributions.h"
#include "view/viewables/regeneration/content_regeneration_settings.h"

class sound_system;

namespace augs {
	class renderer;
}

struct viewables_load_input {
	const all_viewables_defs& new_defs;
	const necessary_image_definitions_map& necessary_image_definitions;
	const all_gui_fonts_inputs& gui_fonts;
	const content_regeneration_settings settings;
	const augs::path_type& unofficial_content_dir;

	augs::renderer& renderer;
	const unsigned max_atlas_size;
};

struct viewables_finalize_input {
	const bool measure_atlas_upload;
	augs::renderer& renderer;

	sound_system& sounds;
};

class viewables_streaming {
#if EXPERIMENTAL_USE_PBO
	augs::graphics::pbo uploading_pbo;
	bool pbo_ready_to_use = false;
#endif

	std::vector<rgba> pbo_fallback;

	all_loaded_gui_fonts loaded_gui_fonts;

	image_definitions_map future_image_definitions;
	all_gui_fonts_inputs future_gui_fonts;

	std::future<general_atlas_output> future_general_atlas;

	all_viewables_defs now_loaded_viewables_defs;
	all_gui_fonts_inputs now_loaded_gui_font_defs;

	sound_definitions_map future_sound_definitions;
	std::vector<std::pair<assets::sound_id, augs::sound_buffer_loading_input>> sound_requests;
	std::future<std::vector<std::optional<augs::sound_buffer>>> future_loaded_buffers;

public:
	std::optional<augs::graphics::texture> general_atlas;
	viewables_streaming_profiler performance;

	loaded_sounds_map loaded_sounds;
	images_in_atlas_map images_in_atlas;
	necessary_images_in_atlas_map necessary_images_in_atlas;

	atlas_profiler general_atlas_performance;
	atlas_profiler neon_map_atlas_performance;

	viewables_streaming(augs::renderer& renderer);

	void load_all(viewables_load_input);
	void finalize_load(viewables_finalize_input);

	auto& get_loaded_gui_fonts() {
		return loaded_gui_fonts;
	}

	void finalize_pending_tasks();
};
