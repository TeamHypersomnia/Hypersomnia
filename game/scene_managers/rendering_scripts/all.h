#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/texture_baker/texture_baker.h"
#include "augs/graphics/vertex.h"
#include "game/detail/camera_cone.h"
#include "game/components/sentience_component.h"

#include "game/assets/texture_id.h"

namespace components {
	struct sentience;
}

#include <functional>
class viewing_step;
class interpolation_system;

namespace resources {
	class manager;
}

namespace rendering_scripts {
	void standard_rendering(const viewing_step step);
	void illuminated_rendering(const viewing_step step);

	void draw_crosshair_lines(
		std::function<void(vec2, vec2, rgba)> callback,
		std::function<void(vec2, vec2)> dashed_line_callback,
		const interpolation_system& sys,
		const const_entity_handle crosshair, 
		const const_entity_handle character
	);

	void draw_sentience_meters(
		augs::vertex_triangle_buffer&,
		const components::sentience&,
		const vec2i left_top_position,
		const unsigned total_width,
		const unsigned vertical_bar_padding,
		const assets::texture_id health_icon,
		const assets::texture_id personal_electricity_icon,
		const assets::texture_id consciousness_icon
	);
}