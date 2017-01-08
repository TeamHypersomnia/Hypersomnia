#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/texture_baker/texture_baker.h"
#include "augs/graphics/vertex.h"
#include "game/detail/camera_cone.h"
#include <functional>
class viewing_step;
class interpolation_system;

namespace rendering_scripts {
	void standard_rendering(viewing_step& step);
	void illuminated_rendering(viewing_step& step);

	void draw_crosshair_lines(
		std::function<void(vec2, vec2, rgba)> callback,
		std::function<void(vec2, vec2)> dashed_line_callback,
		const interpolation_system& sys,
		const const_entity_handle crosshair, 
		const const_entity_handle character
	);
}