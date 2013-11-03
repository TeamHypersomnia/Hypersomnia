#include "stdafx.h"
#include "lookat_system.h"
#include "entity_system/entity.h"

#include "../components/physics_component.h"

void lookat_system::process_entities(world& owner) {
	for (auto it : targets) {
		auto& lookat = it->get<components::lookat>();
	
		if (lookat.target == nullptr)
			continue;

		auto& transform = it->get<components::transform>().current;
		if (lookat.look_mode == components::lookat::look_type::POSITION) {
			auto target_transform = lookat.target->find<components::transform>();
			if (target_transform != nullptr)
				transform.rotation = (target_transform->current.pos - transform.pos).get_degrees();
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
					transform.rotation = direction.get_degrees();
			}
		}
	}
}
