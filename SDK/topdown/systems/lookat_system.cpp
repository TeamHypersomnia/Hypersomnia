#include "lookat_system.h"
#include "../messages/moved_message.h"
#include "../components/physics_component.h"

void lookat_system::process_entities(world& owner) {
	for (auto it : targets) {
		auto& lookat = it->get<components::lookat>();
		auto& transform = it->get<components::transform>();

		if (lookat.type == components::lookat::chase_type::POSITION) {
			transform.current.rotation = (lookat.target->get<components::transform>().current.pos - transform.current.pos).get_radians();
		}
		else if (lookat.type == components::lookat::chase_type::VELOCITY) {
			transform.current.rotation = vec2<>(lookat.target->get<components::physics>().body->GetLinearVelocity()).get_radians();
		}
	}
}
