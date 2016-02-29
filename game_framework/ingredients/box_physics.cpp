#include "augs/entity_system/world.h"
#include "game_framework/components/physics_component.h"

#include "game_framework/globals/filters.h"

#include "../components/physics_definition_component.h"

namespace ingredients {
	void crate_physics(augs::entity_id e) {
		auto& physics_definition = *e += components::physics_definition();

		physics_definition.body.fixed_rotation = false;
		physics_definition.preserve_definition_for_cloning = true;
		physics_definition.dont_create_fixtures_and_body = false;

		auto& info = physics_definition.new_fixture();
		info.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 1;
	}

	void static_crate_physics(augs::entity_id e) {
		auto& physics_definition = *e += components::physics_definition();

		physics_definition.body.fixed_rotation = false;
		physics_definition.body.body_type = b2_staticBody;

		auto& info = physics_definition.new_fixture();
		info.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 1;
	}
	
}