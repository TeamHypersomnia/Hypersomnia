#include "rotation_copying_system.h"
#include <Box2D\Dynamics\b2Body.h>

#include "game/entity_id.h"
#include "graphics/renderer.h"

#include "game/components/physics_component.h"
#include "game/components/gun_component.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_utils.h"

#include "ensure.h"

float colinearize_AB(vec2 O_center_of_rotation, vec2 A_rifle_center, vec2 B_barrel, vec2 C_crosshair) {
	auto crosshair_vector = C_crosshair - O_center_of_rotation;
	auto barrel_vector = B_barrel - O_center_of_rotation;
	
	if (crosshair_vector.is_epsilon(1.f))
		crosshair_vector.set(1, 0);

	if (crosshair_vector.length() < barrel_vector.length() + 1.f)
		crosshair_vector.set_length(barrel_vector.length() + 1.f);
	
	C_crosshair = O_center_of_rotation + crosshair_vector;

	float oc_radius = crosshair_vector.length();
	
	auto intersection = circle_ray_intersection(B_barrel, A_rifle_center, O_center_of_rotation, oc_radius);
	bool has_intersection = intersection.first;
	
	ensure(has_intersection);

	auto G = intersection.second;
	auto CG = C_crosshair - G;
	auto AG = A_rifle_center - G;

	auto final_angle = 2 * (CG.degrees() - AG.degrees());
	
	if (augs::renderer::get_current().debug_draw_colinearization) {
		auto& ln = augs::renderer::get_current().logic_lines;

		ln.draw_cyan(O_center_of_rotation, C_crosshair);
		ln.draw_red(O_center_of_rotation, A_rifle_center);
		ln.draw_red(O_center_of_rotation, B_barrel);
		ln.draw_yellow(O_center_of_rotation, G);
		
		ln.draw_green(G, A_rifle_center);
		ln.draw_green(G, C_crosshair);

		A_rifle_center.rotate(final_angle, O_center_of_rotation);
		B_barrel.rotate(final_angle, O_center_of_rotation);

		ln.draw_red(O_center_of_rotation, A_rifle_center);
		ln.draw_red(O_center_of_rotation, B_barrel);

		ln.draw(A_rifle_center - (B_barrel - A_rifle_center) * 100, B_barrel + (B_barrel - A_rifle_center)*100);
	}

	return final_angle;
}

void rotation_copying_system::resolve_rotation_copying_value(entity_id it) {
	auto& rotation_copying = it.get<components::rotation_copying>();

	if (rotation_copying.target.dead())
		return;

	auto& transform = it.get<components::transform>();
	float new_angle = 0.f;

	if (rotation_copying.look_mode == components::rotation_copying::look_type::POSITION) {
		auto target_transform = rotation_copying.target.find<components::transform>();
		
		if (target_transform != nullptr) {
			auto diff = target_transform->pos - transform.pos;
			
			if (diff.is_epsilon(1.f))
				new_angle = 0.f;
			else
				new_angle = diff.degrees();

			if (rotation_copying.colinearize_item_in_hand) {
				auto hand = map_primary_action_to_secondary_hand_if_primary_empty(it, 0);

				if (hand.has_items()) {
					auto subject_item = hand->items_inside[0];

					auto* maybe_gun = subject_item.find<components::gun>();

					if (maybe_gun) {
						auto gun_transform = subject_item.get<components::transform>();
						auto mc = it.get<components::physics>().get_mass_position();

						new_angle =
							transform.rotation +
							colinearize_AB(mc, gun_transform.pos, maybe_gun->calculate_barrel_transform(gun_transform).pos, target_transform->pos);
					}
				}
			}
		}
	}
	else {
		auto target_physics = rotation_copying.target.find<components::physics>();

		if (target_physics != nullptr) {
			vec2 direction;

			if (rotation_copying.look_mode == components::rotation_copying::look_type::VELOCITY)
				direction = vec2(target_physics->body->GetLinearVelocity());

			if (rotation_copying.look_mode == components::rotation_copying::look_type::ACCELEARATION)
				direction = vec2(target_physics->body->m_last_force);

			if (direction.non_zero())
				new_angle = direction.degrees();
		}
	}

	if (rotation_copying.easing_mode == rotation_copying.EXPONENTIAL) {
		ensure(0);
		//float averaging_constant = static_cast<float>(
		//	pow(rotation_copying.smoothing_average_factor, rotation_copying.averages_per_sec * delta_seconds())
		//	);
		//
		//rotation_copying.last_rotation_interpolant = (rotation_copying.last_rotation_interpolant * averaging_constant + new_rotation * (1.0f - averaging_constant)).normalize();
		//rotation_copying.last_value = rotation_copying.last_rotation_interpolant.degrees();
	}
	else if (rotation_copying.easing_mode == rotation_copying.NONE) {
		rotation_copying.last_value = new_angle;
	}
}

void rotation_copying_system::update_physical_motors() {
	for (auto it : targets) {
		auto& rotation_copying = it.get<components::rotation_copying>();

		if (rotation_copying.update_value) {
			if (rotation_copying.use_physical_motor && it.find<components::physics>() != nullptr) {
				resolve_rotation_copying_value(it);

				auto& physics = it.get<components::physics>();
				physics.enable_angle_motor = true;
				physics.target_angle = rotation_copying.last_value;
			}
		}
	}
}

void rotation_copying_system::update_rotations() {
	for (auto it : targets) {
		auto& rotation_copying = it.get<components::rotation_copying>();

		if (rotation_copying.update_value) {
			if (!rotation_copying.use_physical_motor) {
				resolve_rotation_copying_value(it);
				it.get<components::transform>().rotation = rotation_copying.last_value;
			}
		}
	}
}
