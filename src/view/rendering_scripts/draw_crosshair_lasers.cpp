#include "rendering_scripts.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/explosive_component.h"
#include "augs/drawing/drawing.h"

#include "game/inferential_systems/physics_system.h"
#include "game/enums/filters.h"
#include "game/detail/entity_scripts.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void draw_crosshair_lasers(const draw_crosshair_lasers_input in) {
	if (in.crosshair.alive()) {
		const auto& cosmos = in.crosshair.get_cosmos();
		const auto& physics = cosmos.inferential_systems.get<physics_system>();

		const auto crosshair_pos = in.crosshair.get_viewing_transform(in.interpolation).pos;

		auto calculate_color = [&](const const_entity_handle target) {
			const auto att = calculate_attitude(in.character, target);

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
				filters::bullet(),
				subject
			);

			if (raycast.hit) {
				in.dashed_line_callback(raycast.intersection, line_to);

				in.callback(
					line_from, 
					raycast.intersection, 
					calculate_color(cosmos[raycast.what_entity])
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
				const auto& gun = subject_item.get<components::gun>();

				const auto rifle_transform = subject_item.get_viewing_transform(in.interpolation);
				const auto barrel_center = gun.calculate_barrel_center(rifle_transform);
				const auto muzzle = gun.calculate_muzzle_position(rifle_transform);

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
			else if (subject_item.has<components::explosive>()) {
				const auto explosive_transform = subject_item.get_viewing_transform(in.interpolation);
				const auto explosive_target_vector = explosive_transform.pos + vec2().set_from_degrees(explosive_transform.rotation);

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
