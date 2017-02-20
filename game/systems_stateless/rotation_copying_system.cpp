#include "rotation_copying_system.h"
#include <Box2D\Dynamics\b2Body.h>

#include "game/transcendental/entity_id.h"
#include "augs/graphics/renderer.h"

#include "game/components/physics_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/gun_component.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_utils.h"

#include "augs/ensure.h"

#include "game/components/rotation_copying_component.h"
#include "game/components/transform_component.h"

#include "augs/misc/timer.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"

using namespace augs;

float colinearize_AB(const vec2 O_center_of_rotation, vec2 A_barrel_center, vec2 B_muzzle, vec2 C_crosshair) {
	auto crosshair_vector = C_crosshair - O_center_of_rotation;
	auto barrel_vector = B_muzzle - O_center_of_rotation;
	
	if (crosshair_vector.is_epsilon(1.f)) {
		crosshair_vector.set(1, 0);
	}

	if (crosshair_vector.length() < barrel_vector.length() + 1.f) {
		return crosshair_vector.degrees();
	}
	
	C_crosshair = O_center_of_rotation + crosshair_vector;

	const float oc_radius = crosshair_vector.length();
	
	const auto intersection = circle_ray_intersection(B_muzzle, A_barrel_center, O_center_of_rotation, oc_radius);
	const bool has_intersection = intersection.hit;
	
	ensure(has_intersection);

	const auto G = intersection.intersection;
	const auto CG = C_crosshair - G;
	const auto AG = A_barrel_center - G;

	const auto final_angle = 2 * (CG.degrees() - AG.degrees());
	
	if (augs::renderer::get_current().debug_draw_colinearization) {
		auto& ln = augs::renderer::get_current().logic_lines;

		ln.draw_cyan(O_center_of_rotation, C_crosshair);
		ln.draw_red(O_center_of_rotation, A_barrel_center);
		ln.draw_red(O_center_of_rotation, B_muzzle);
		ln.draw_yellow(O_center_of_rotation, G);
		
		ln.draw_green(G, A_barrel_center);
		ln.draw_green(G, C_crosshair);

		A_barrel_center.rotate(final_angle, O_center_of_rotation);
		B_muzzle.rotate(final_angle, O_center_of_rotation);

		ln.draw_red(O_center_of_rotation, A_barrel_center);
		ln.draw_red(O_center_of_rotation, B_muzzle);

		ln.draw(A_barrel_center - (B_muzzle - A_barrel_center) * 100, B_muzzle + (B_muzzle - A_barrel_center)*100);
	}

	return final_angle;
}

float rotation_copying_system::resolve_rotation_copying_value(const const_entity_handle it) const {
	auto& rotation_copying = it.get<components::rotation_copying>();
	auto& cosmos = it.get_cosmos();
	const auto target = cosmos[rotation_copying.target];

	if (target.dead())
		return 0.f;

	float new_angle = 0.f;

	if (rotation_copying.look_mode == components::rotation_copying::look_type::ROTATION) {
		new_angle = target.get_logic_transform().rotation;
	}
	else if (rotation_copying.look_mode == components::rotation_copying::look_type::POSITION) {
		const auto& target_transform = target.get_logic_transform();
		
		const auto diff = target_transform.pos - position(it);

		if (diff.is_epsilon(1.f)) {
			new_angle = 0.f;
		}
		else {
			new_angle = diff.degrees();
		}

		if (rotation_copying.colinearize_item_in_hand) {
			const auto guns = it.guns_wielded();

			if (guns.size() > 0) {
				const auto subject_item = guns[0];

				const auto& gun = subject_item.get<components::gun>();

				const auto rifle_transform = subject_item.get_logic_transform();
				auto barrel_center = gun.calculate_barrel_center(rifle_transform);
				auto muzzle = gun.calculate_muzzle_position(rifle_transform);
				const auto mc = position(it);

				barrel_center.rotate(-rotation(it), mc);
				muzzle.rotate(-rotation(it), mc);

				auto crosshair_vector = target_transform.pos - mc;

				new_angle = colinearize_AB(mc, barrel_center, muzzle, target_transform.pos);
			}
		}
	}
	else {
		if (target.has<components::physics>()) {
			const auto target_physics = target.get<components::physics>();

			vec2 direction;

			if (rotation_copying.look_mode == components::rotation_copying::look_type::VELOCITY) {
				direction = vec2(target_physics.velocity());
			}

			if (direction.non_zero()) {
				new_angle = direction.degrees();
			}
		}
	}

	if (rotation_copying.easing_mode == components::rotation_copying::easing_type::EXPONENTIAL) {
		ensure(false);
		//float averaging_constant = static_cast<float>(
		//	pow(rotation_copying.smoothing_average_factor, rotation_copying.averages_per_sec * delta_seconds())
		//	);
		//
		//rotation_copying.last_rotation_interpolant = (rotation_copying.last_rotation_interpolant * averaging_constant + new_rotation * (1.0f - averaging_constant)).normalize();
		//rotation_copying.last_value = rotation_copying.last_rotation_interpolant.degrees();
	}
	else if (rotation_copying.easing_mode == components::rotation_copying::easing_type::NONE) {
	
	}

	return new_angle;
}

void rotation_copying_system::update_rotations(cosmos& cosmos) const {
	for (const auto it : cosmos.get(processing_subjects::WITH_ROTATION_COPYING)) {
		const auto& rotation_copying = it.get<components::rotation_copying>();

		if (rotation_copying.update_value) {
			if (it.has<components::physics>()) {
				const auto target_angle = resolve_rotation_copying_value(it);
				const auto phys = it.get<components::physics>();

				phys.set_transform({ phys.get_position(), target_angle });
				phys.set_angular_velocity(0);
			}
			else {
				it.get<components::transform>().rotation = resolve_rotation_copying_value(it);
			}
		}
	}
}