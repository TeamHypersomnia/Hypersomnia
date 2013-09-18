#include "lookat_system.h"
#include "entity_system/entity.h"

#include "../messages/moved_message.h"
#include "../components/physics_component.h"

void lookat_system::process_entities(world& owner) {
	for (auto it : targets) {
		auto& lookat = it->get<components::lookat>();
	
		if (lookat.target == nullptr)
			continue;

		auto& transform = it->get<components::transform>();
		if (lookat.look_mode == components::lookat::chase_type::POSITION) {
			auto target_transform = lookat.target->find<components::transform>();
			if (target_transform != nullptr)
				transform.current.rotation = (target_transform->current.pos - transform.current.pos).get_radians();
		}
		else if (lookat.look_mode == components::lookat::chase_type::VELOCITY) {
			auto target_physics = lookat.target->find<components::physics>();
			
			if (target_physics != nullptr)
				transform.current.rotation = vec2<>(target_physics->body->GetLinearVelocity()).get_radians();
		}
	}
}
