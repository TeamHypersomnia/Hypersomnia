#include "all.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/gun_component.h"
#include "augs/graphics/drawers.h"

#include "game/systems_temporary/physics_system.h"
#include "game/enums/filters.h"
#include "game/detail/entity_scripts.h"

namespace rendering_scripts {
	void draw_crosshair_lines(
		std::function<void(vec2, vec2, rgba)> callback,
		const interpolation_system& interp,
		const const_entity_handle crosshair, 
		const const_entity_handle character) {
		if (crosshair.alive()) {
			const auto& cosmos = crosshair.get_cosmos();
			const auto& physics = cosmos.systems_temporary.get<physics_system>();

			vec2 line_from[2];
			vec2 line_to[2];

			const auto crosshair_pos = crosshair.viewing_transform(interp).pos;

			const auto guns = character.guns_wielded();

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

			if (guns.size() >= 1) {
				const auto subject_item = guns[0];

				const auto& gun = subject_item.get<components::gun>();

				const auto rifle_transform = subject_item.viewing_transform(interp);
				const auto barrel_center = gun.calculate_barrel_center(rifle_transform);
				const auto muzzle = gun.calculate_muzzle_position(rifle_transform);

				line_from[0] = muzzle;
				
				const auto proj = crosshair_pos.get_projection_multiplier(barrel_center, muzzle);

				if (proj > 1.f) {
					line_to[0] = barrel_center + (muzzle - barrel_center) * proj;
					
					const auto raycast = physics.ray_cast_px(line_from[0], line_to[0], filters::bullet(), subject_item);

					auto col = cyan;

					if (raycast.hit) {
						line_to[0] = raycast.intersection;
						col = calculate_color(cosmos[raycast.what_entity]);
					}

					callback(line_from[0], line_to[0], col);
				}
			}

			if (guns.size() >= 2) {
				const auto subject_item = guns[1];

				const auto& gun = subject_item.get<components::gun>();

				const auto rifle_transform = subject_item.viewing_transform(interp);
				const auto barrel_center = gun.calculate_barrel_center(rifle_transform);
				const auto muzzle = gun.calculate_muzzle_position(rifle_transform);

				line_from[1] = muzzle;

				const auto proj = crosshair_pos.get_projection_multiplier(barrel_center, muzzle);

				if (proj > 1.f) {
					line_to[1] = barrel_center + (muzzle - barrel_center) * proj;
					
					const auto raycast = physics.ray_cast_px(line_from[1], line_to[1], filters::bullet(), subject_item);

					auto col = cyan;

					if (raycast.hit) {
						line_to[1] = raycast.intersection;
						col = calculate_color(cosmos[raycast.what_entity]);
					}

					callback(line_from[1], line_to[1], col);
				}
			}
		}
	}
}