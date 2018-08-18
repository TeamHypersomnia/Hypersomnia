#pragma once
#include "augs/math/camera_cone.h"
#include "augs/drawing/drawing.h"
#include "view/necessary_resources.h"
#include "augs/math/vec2.h"

class visible_entities;
struct config_lua_table;

namespace augs {
	struct baked_font;
}

struct draw_setup_gui_input {
	const visible_entities& all_visible;
	const camera_cone cone;
	const augs::drawer_with_default& drawer;
	const augs::line_drawer_with_default& line_drawer;
	const config_lua_table& config;
	const necessary_images_in_atlas_map& necessary_images;
	const vec2i mouse_pos;
	const vec2i screen_size;
	const augs::baked_font& gui_font;
};
