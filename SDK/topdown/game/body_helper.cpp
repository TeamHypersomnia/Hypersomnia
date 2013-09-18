#include "body_helper.h"
#include "../components/physics_component.h"
#include "../components/render_component.h"
#include "../renderable.h"

#include "entity_system/entity_system.h"

namespace topdown {
	b2World* current_b2world = nullptr;
	
	void create_physics_component_str(augmentations::entity_system::entity& subject, b2Filter filter, const std::string& body_type) {
		b2BodyType result = b2_dynamicBody;
		if (body_type == "dynamic") result = b2_dynamicBody;
		if (body_type == "kinematic") result = b2_kinematicBody;
		if (body_type == "static") result = b2_staticBody;

		create_physics_component(subject, filter, result);
	}
	void create_physics_component(augmentations::entity_system::entity& subject, b2Filter filter, b2BodyType type) {
		auto physics = components::physics(subject.get<components::render>().instance->create_body(subject, *current_b2world, type));

		physics.body->GetFixtureList()->SetFilterData(filter);
		subject.add(physics);
	}
}