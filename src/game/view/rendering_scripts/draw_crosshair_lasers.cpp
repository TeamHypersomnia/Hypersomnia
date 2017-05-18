#include "all.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/explosive_component.h"
#include "augs/graphics/drawers.h"

#include "game/systems_inferred/physics_system.h"
#include "game/enums/filters.h"
#include "game/detail/entity_scripts.h"

namespace rendering_scripts {
	void draw_crosshair_lasers(
		std::function<void(vec2, vec2, rgba)> callback,
		std::function<void(vec2, vec2)> dashed_line_callback,
		const interpolation_system& interp,
		const const_entity_handle crosshair, 
		const const_entity_handle character) {
		if (crosshair.alive()) {
			const auto& cosmos = crosshair.get_cosmos();
			const auto& physics = cosmos.systems_inferred.get<physics_system>();

			const auto crosshair_pos = crosshair.get_viewing_transform(interp).pos;

			auto calculate_color = [&](const const_entity_handle target) {
				const auto att = calculate_attitude(character, target);

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
					dashed_line_callback(raycast.intersection, line_to);

					callback(
						line_from, 
						raycast.intersection, 
						calculate_color(cosmos[raycast.what_entity])
					);
				}
				else {
					callback(
						line_from,
						line_to,
						cyan
					);
				}
			};

			for (const auto subject_item_id : character.get_wielded_items()) {
				const auto subject_item = cosmos[subject_item_id];

				if (subject_item.has<components::gun>()) {
					const auto& gun = subject_item.get<components::gun>();

					const auto rifle_transform = subject_item.get_viewing_transform(interp);
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
					const auto explosive_transform = subject_item.get_viewing_transform(interp);
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
}