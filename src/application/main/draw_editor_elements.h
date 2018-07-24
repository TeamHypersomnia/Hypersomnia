#pragma once
#include "view/necessary_resources.h"

namespace augs {
	struct drawer_with_default;
	struct line_drawer_with_default;
}

class editor_setup;
class visible_entities;
struct camera_cone;
struct editor_settings;

void draw_editor_elements(
	const editor_setup&,
	const visible_entities&,
	camera_cone cone,
	const augs::drawer_with_default&,
	const augs::line_drawer_with_default&,
	const editor_settings&,
	const necessary_images_in_atlas_map&,
	const vec2i mouse_pos,
	const vec2i screen_size
);
