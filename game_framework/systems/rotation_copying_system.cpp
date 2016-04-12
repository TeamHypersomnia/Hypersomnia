#include "rotation_copying_system.h"
#include "entity_system/entity.h"

#include "../components/physics_component.h"
#include "../components/gun_component.h"
#include "../detail/inventory_slot_id.h"
#include "../detail/inventory_slot.h"
#include "../detail/inventory_utils.h"
#include "ensure.h"
#include <Box2D\Dynamics\b2Body.h>

float colinearize_AB(vec2 center_of_rotation, vec2 A_inside, vec2 B_circumferential, vec2 crosshair_outside) {
	vec2 radius = (B_circumferential - center_of_rotation).length();

	return 90.f;
}

void rotation_copying_system::resolve_rotation_copying_value(augs::entity_id it) {
	auto& rotation_copying = it->get<components::rotation_copying>();

	if (rotation_copying.target.dead())
		return;

	auto& transform = it->get<components::transform>();
	float new_angle = 0.f;

	if (rotation_copying.look_mode == components::rotation_copying::look_type::POSITION) {
		auto target_transform = rotation_copying.target->find<components::transform>();
		
		if (target_transform != nullptr) {
			new_angle = (vec2(vec2i(target_transform->pos) - vec2i(transform.pos))).degrees();

			if (rotation_copying.colinearize_item_in_hand) {
				auto hand = map_primary_action_to_secondary_hand_if_primary_empty(it, 0);

				if (hand.has_items()) {
					auto subject_item = hand->items_inside[0];

					auto* maybe_gun = subject_item->find<components::gun>();

					if (maybe_gun) {
						auto gun_transform = subject_item->get<components::transform>();
						gun_transform.rotation = 0;

						new_angle = colinearize_AB(transform.pos, gun_transform.pos, maybe_gun->calculate_barrel_transform(gun_transform).pos, target_transform->pos);
					}
				}
			}
		}
	}
	else {
		auto target_physics = rotation_copying.target->find<components::physics>();

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
		auto& rotation_copying = it->get<components::rotation_copying>();

		if (rotation_copying.update_value) {
			if (rotation_copying.use_physical_motor) {
				resolve_rotation_copying_value(it);

				auto& physics = it->get<components::physics>();
				physics.enable_angle_motor = true;
				physics.target_angle = rotation_copying.last_value;
			}
		}
	}
}

void rotation_copying_system::update_rotations() {
	for (auto it : targets) {
		auto& rotation_copying = it->get<components::rotation_copying>();

		if (rotation_copying.update_value) {
			if (!rotation_copying.use_physical_motor) {
				resolve_rotation_copying_value(it);
				it->get<components::transform>().rotation = rotation_copying.last_value;
			}
		}
	}
}
