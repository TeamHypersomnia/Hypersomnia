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

	void draw_cast_spells_highlights(
		augs::vertex_triangle_buffer&,
		const interpolation_system& sys,
		const camera_cone cam,
		const cosmos& cosm,
		const float global_time_seconds
	);

	augs::vertex_triangle_buffer draw_circular_bars_and_get_textual_info(const viewing_step);
}