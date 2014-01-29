#include "stdafx.h"
#include "lookat_system.h"
#include "entity_system/entity.h"

#include "../components/physics_component.h"

void lookat_system::process_entities(world& owner) {
	double delta = smooth_timer.extract<std::chrono::seconds>();

	for (auto it : targets) {
		auto& lookat = it->get<components::lookat>();
	
		if (lookat.target == nullptr)
			continue;

		auto& transform = it->get<components::transform>().current;
		vec2<> new_rotation;

		if (lookat.look_mode == components::lookat::look_type::POSITION) {
			auto target_transform = lookat.target->find<components::transform>();
			if (target_transform != nullptr)
				new_rotation = (target_transform->current.pos - transform.pos).normalize();
		}
		else {
			auto target_physics = lookat.target->find<components::physics>();
			
			if (target_physics != nullptr) {
				vec2<> direction;

				if (lookat.look_mode == components::lookat::look_type::VELOCITY)
					direction = vec2<>(target_physics->body->GetLinearVelocity());

				if (lookat.look_mode == components::lookat::look_type::ACCELEARATION)
					direction = vec2<>(target_physics->body->m_last_force);

				if (direction.non_zero())
					new_rotation = direction.normalize();
			}
		}

		if (lookat.easing_mode == lookat.EXPONENTIAL) {
			float averaging_constant = static_cast<float>(
				pow(lookat.smoothing_average_factor, lookat.averages_per_sec * delta)
				);

			lookat.last_rotation_interpolant = (lookat.last_rotation_interpolant * averaging_constant + new_rotation * (1.0f - averaging_constant)).normalize();
			transform.rotation = lookat.last_rotation_interpolant.get_degrees();
		}
		else if (lookat.easing_mode == lookat.NONE) {
			transform.rotation = new_rotation.get_degrees();
		}
	}
}
