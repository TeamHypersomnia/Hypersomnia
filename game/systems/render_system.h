#pragma once
#include "game/enums/render_layer.h"

#include <vector>
#include <array>

#include "augs/graphics/vertex.h"
#include "game/entity_handle_declaration.h"

using namespace augs;

struct state_for_drawing_camera;

class render_system {
public:
	void draw_entities(augs::vertex_triangle_buffer& output, std::vector<const_entity_handle>, state_for_drawing_camera in, bool only_border_highlights = false) const;
	std::array<std::vector<const_entity_handle>, render_layer::LAYER_COUNT> get_visible_per_layer(std::vector<const_entity_handle>) const;
};