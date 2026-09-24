#pragma once
#include <optional>
#include <vector>
#include "view/character_camera.h"
#include "view/game_drawing_settings.h"
#include "view/necessary_resources.h"
#include "view/game_gui/special_indicator.h"
#include "view/audiovisual_state/particle_triangle_buffers.h"
#include "augs/graphics/renderer_settings.h"
#include "game/stateless_systems/visibility_system.h"
#include "view/gui_fonts.h"
#include "view/damage_indication_settings.h"

namespace augs {
	class thread_pool;

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

struct minimap_world_transform;

struct illuminated_rendering_input {
	const character_camera camera;
	const float camera_requested_fow_expansion;
	const float camera_edge_zoomout_mult;
	const camera_cone queried_cone;
	const vec2 pre_step_crosshair_displacement;
	const audiovisual_state& audiovisuals;
	const game_drawing_settings drawing;
	const bool viewer_is_spectator;
	const bool see_enemies_behind_walls = false;
	const bool draw_enemy_crosshairs = false;
	const minimap_state_type minimap_state = minimap_state_type::NORMAL;
	const float minimap_area_zoom = 1.0f;
	const necessary_images_in_atlas_map& necessary_images;
	const all_loaded_gui_fonts& fonts;
	const images_in_atlas_map& game_images;
	const double interpolation_ratio = 0.0;
	augs::renderer& renderer;
	frame_profiler& frame_performance;
	augs::graphics::texture* general_atlas;
	illuminated_rendering_fbos& fbos;
	const illuminated_rendering_shaders& shaders;
	const visible_entities& all_visible;
	const performance_settings& perf_settings;
	const augs::renderer_settings& renderer_settings;
	const std::vector<additional_highlight>& additional_highlights;
	const std::vector<special_indicator>& special_indicators;
	const special_indicator_meta indicator_meta;
	const particle_triangle_buffers& drawn_particles;

	const damage_indication_settings damage_indication;
	
	const std::vector<visibility_request>& light_requests;
	const bool streamer_mode;
	minimap_world_transform* const minimap_transform;
	augs::thread_pool& pool;

	float get_environment_shadow_strength() const;

	bool environment_shadows_enabled() const {
		return get_environment_shadow_strength() > 0.0f;
	}

	bool strict_fow_mode() const {
		return camera_edge_zoomout_mult > 0.35f;
	}

	auto get_considered_fow() const {
		auto considered_fow = drawing.fog_of_war;

		considered_fow.size *= camera_requested_fow_expansion;

		if (camera_edge_zoomout_mult > 0.0f) {
			const auto max_fow_reduction = 1.0f / 12;
			const auto fow_reduction = augs::interp(1.0f, max_fow_reduction, camera_edge_zoomout_mult);

			considered_fow.angle *= fow_reduction;
		}

		return considered_fow;
	}
};

void enqueue_illuminated_rendering_jobs(
	augs::thread_pool& pool, 
	const illuminated_rendering_input& in
);

void illuminated_rendering(const illuminated_rendering_input in);
