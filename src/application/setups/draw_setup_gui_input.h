#pragma once
#include "augs/math/camera_cone.h"
#include "augs/drawing/drawing.h"
#include "view/necessary_resources.h"
#include "augs/math/vec2.h"
#include "view/gui_fonts.h"
#include "view/viewables/avatars_in_atlas_map.h"
#include "augs/graphics/renderer.h"
#include "view/mode_gui/arena/arena_player_meta.h"

class images_in_atlas_map;
class visible_entities;
struct config_lua_table;

namespace augs {
	struct baked_font;
	struct all_necessary_sounds;
}

struct draw_setup_gui_input {
	const visible_entities& all_visible;
	const camera_cone cone;
	const augs::atlas_entry blank_texture;
	const config_lua_table& config;
	const necessary_images_in_atlas_map& necessary_images;
	const augs::graphics::texture* general_atlas;
	const augs::graphics::texture* avatar_atlas;
	const images_in_atlas_map& images_in_atlas;
	const avatars_in_atlas_map& avatars_in_atlas;
	augs::renderer& renderer;
	const vec2i mouse_pos;
	const vec2i screen_size;
	const all_loaded_gui_fonts& gui_fonts;
	const all_necessary_sounds& sounds;
	const arena_player_metas* const player_metas;

	auto get_drawer() const {
		return augs::drawer_with_default { renderer.get_triangle_buffer(), blank_texture };
	}

	auto get_line_drawer() const {
		return augs::line_drawer_with_default { renderer.get_line_buffer(), blank_texture };
	}
};
