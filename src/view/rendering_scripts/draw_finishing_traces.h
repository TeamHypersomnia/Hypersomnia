#pragma once
#include "augs/drawing/sprite.hpp"

#include "game/cosmos/cosmos.h"

#include "view/rendering_scripts/draw_entity_input.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/finishing_trace_system.hpp"

/*
	Drawn exactly as the finishing traces were back when they were entities of the MISSILES layer,
	after all the rounds, since their entity type came later.
*/

inline void draw_finishing_traces(
	const finishing_trace_system& finishing_traces,
	const cosmos& cosm,
	const draw_renderable_input& in,
	const bool neons
) {
	const auto visible_area = in.cone.get_visible_world_rect_aabb();

	finishing_traces.for_each_drawn(
		cosm,
		in.interp.get_everything_else_alpha(),
		[&](const invariants::sprite& sprite, const transformr where, const vec2 size_mult) {
			const auto drawn_size = vec2(sprite.size) * size_mult;

			if (!visible_area.hover(augs::calc_vertices_aabb(augs::make_sprite_points(where.pos, drawn_size, where.rotation)))) {
				return;
			}

			auto input = in.make_input_for<invariants::sprite>();
			input.renderable_transform = where;
			input.size_mult = size_mult;
			input.global_time_seconds = in.global_time_seconds;
			input.use_neon_map = neons;

			augs::draw(sprite, in.manager, input);
		}
	);
}
