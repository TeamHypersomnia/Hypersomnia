#pragma once
#include "augs/math/camera_cone.h"
#include "augs/drawing/drawing.h"
#include "view/necessary_resources.h"
#include "augs/math/vec2.h"
#include "view/gui_fonts.h"

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
	const augs::drawer_with_default& drawer;
	const augs::line_drawer_with_default& line_drawer;
	const config_lua_table& config;
	const necessary_images_in_atlas_map& necessary_images;
	const images_in_atlas_map& images_in_atlas;
	const vec2i mouse_pos;
	const vec2i screen_size;
	const all_loaded_gui_fonts& gui_fonts;
	const all_necessary_sounds& sounds;
};
