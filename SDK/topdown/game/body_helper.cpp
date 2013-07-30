#include "body_helper.h"
#include "../components/physics_component.h"
#include "../components/render_component.h"
#include "../renderable.h"

namespace topdown {
	void create_physics_component(augmentations::entity_system::entity& subject, b2World& b2world, b2BodyType type) {
		subject.add(components::physics(subject.get<components::render>().instance->create_body(subject, b2world, type)));
	}
}