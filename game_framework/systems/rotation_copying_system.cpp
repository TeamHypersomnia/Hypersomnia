#include "rotation_copying_system.h"
#include "entity_system/entity.h"

#include "../components/physics_component.h"
#include <Box2D\Dynamics\b2Body.h>

void rotation_copying_system::resolve_rotation_copying_value(augs::entity_id it) {
	auto& rotation_copying = it->get<components::rotation_copying>();

	if (rotation_copying.target.dead())
		return;

	auto& transform = it->get<components::transform>();
	vec2 new_rotation;

	if (rotation_copying.look_mode == components::rotation_copying::look_type::POSITION) {
		auto target_transform = rotation_copying.target->find<components::transform>();
		if (target_transform != nullptr)
			new_rotation = (vec2(vec2i(target_transform->pos) - vec2i(transform.pos))).normalize();
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
				new_rotation = direction.normalize();
		}
	}

	if (rotation_copying.easing_mode == rotation_copying.EXPONENTIAL) {
		float averaging_constant = static_cast<float>(
			pow(rotation_copying.smoothing_average_factor, rotation_copying.averages_per_sec * delta_seconds())
			);

		rotation_copying.last_rotation_interpolant = (rotation_copying.last_rotation_interpolant * averaging_constant + new_rotation * (1.0f - averaging_constant)).normalize();
		rotation_copying.last_value = rotation_copying.last_rotation_interpolant.degrees();
	}
	else if (rotation_copying.easing_mode == rotation_copying.NONE) {
		rotation_copying.last_value = new_rotation.degrees();
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
