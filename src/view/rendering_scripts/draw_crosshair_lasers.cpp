#include "rendering_scripts.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/components/gun_component.h"
#include "game/components/explosive_component.h"
#include "augs/drawing/drawing.h"

#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"
#include "game/detail/entity_scripts.h"
#include "game/detail/gun/gun_math.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void line_output_wrapper::operator()(const vec2 from, const vec2 to, const rgba col) const {
	output.line(line_tex, from, to, col);
}

void dashed_line_output_wrapper::operator()(const vec2 from, const vec2 to, const rgba col) const {
	output.dashed_line(line_tex, from, to, col, len, vel, global_time_seconds);
}

void draw_crosshair_lasers(const draw_crosshair_lasers_input in) {
	if (in.character.alive()) {
		const auto subject_with_crosshair = in.character; 

		const auto& cosmos = in.character.get_cosmos();
		const auto& physics = cosmos.get_solvable_inferred().physics;

		const vec2 crosshair_pos = subject_with_crosshair.get_world_crosshair_transform(in.interpolation).pos;

		auto calc_color = [&](const const_entity_handle target) {
			const auto att = calc_attitude(in.character, target);

			if (att == attitude_type::WANTS_TO_KILL || att == attitude_type::WANTS_TO_KNOCK_UNCONSCIOUS) {
				return red;
			}
			else if (att == attitude_type::WANTS_TO_HEAL) {
				return green;
			}
			else {
				return cyan;
			}
		};
		
		const auto make_laser_from_to = [&](
			const const_entity_handle subject,
			const vec2 line_from,
			const vec2 line_to
		) {
			const auto raycast = physics.ray_cast_px(
				cosmos.get_si(),
				line_from,
				line_to,
				filters::flying_item(),
				subject
			);

			if (raycast.hit) {
				in.dashed_line_callback(raycast.intersection, line_to, white);

				in.callback(
					line_from, 
					raycast.intersection, 
					calc_color(cosmos[raycast.what_entity])
				);
			}
			else {
				in.callback(
					line_from,
					line_to,
					cyan
				);
			}
		};

		for (const auto subject_item_id : in.character.get_wielded_items()) {
			const auto subject_item = cosmos[subject_item_id];

			if (subject_item.has<components::gun>()) {
				const auto rifle_transform = subject_item.get_viewing_transform(in.interpolation);
				const auto barrel_center = calc_barrel_center(subject_item, rifle_transform);
				const auto muzzle = calc_muzzle_transform(subject_item, rifle_transform).pos;

				const auto proj = crosshair_pos.get_projection_multiplier(barrel_center, muzzle);

				if (proj > 1.f) {
					const auto line_from = muzzle;
					const auto line_to = barrel_center + (muzzle - barrel_center) * proj;

					make_laser_from_to(
						subject_item,
						line_from,
						line_to
					);
				}
			}
			else if (subject_item.find<invariants::explosive>()) {
				const auto explosive_transform = subject_item.get_viewing_transform(in.interpolation);
				const auto explosive_target_vector = explosive_transform.pos + vec2::from_degrees(explosive_transform.rotation);

				const auto proj = crosshair_pos.get_projection_multiplier(
					explosive_transform.pos,
					explosive_target_vector
				);

				if (proj > 1.f) {
					const auto line_from = explosive_transform.pos;
					const auto line_to = explosive_transform.pos + (explosive_target_vector - explosive_transform.pos) * proj;

					make_laser_from_to(
						subject_item,
						line_from,
						line_to
					);
				}
			}
		}
	}
}
