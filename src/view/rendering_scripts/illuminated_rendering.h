#pragma once
#include <optional>
#include <vector>
#include "view/character_camera.h"
#include "view/game_drawing_settings.h"
#include "view/necessary_resources.h"
#include "view/game_gui/special_indicator.h"

namespace augs {
	namespace graphics { 
		class texture;
	}

	class renderer;
	struct baked_font;
}

struct frame_profiler;
struct audiovisual_state;
struct all_necessary_shaders;
struct all_necessary_fbos;
struct performance_settings;

class images_in_atlas_map;
class visible_entities;

/* Require all */

using illuminated_rendering_fbos = all_necessary_fbos;
using illuminated_rendering_shaders = all_necessary_shaders;

struct additional_highlight {
	entity_id id;
	rgba col;
};

struct illuminated_rendering_input {
	const character_camera camera;
	const vec2 pre_step_crosshair_displacement;
	const float camera_query_mult;
	const audiovisual_state& audiovisuals;
	const game_drawing_settings drawing;
	const necessary_images_in_atlas_map& necessary_images;
	const augs::baked_font& gui_font;
	const images_in_atlas_map& game_images;
	const double interpolation_ratio = 0.0;
	augs::renderer& renderer;
	frame_profiler& frame_performance;
	std::optional<augs::graphics::texture>& general_atlas;
	illuminated_rendering_fbos& fbos;
	const illuminated_rendering_shaders& shaders;
	const visible_entities& all_visible;
	const performance_settings& perf_settings;
	const std::vector<additional_highlight>& additional_highlights;
	const std::vector<special_indicator>& special_indicators;
	const special_indicator_meta& indicator_meta;
};

void illuminated_rendering(const illuminated_rendering_input in);
