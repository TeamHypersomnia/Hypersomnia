#include "augs/entity_system/entity_id.h"
#include "game_framework/components/physics_component.h"

#include "game_framework/game/physics_setup_helpers.h"

#include "game_framework/globals/filters.h"

namespace ingredients {
	void crate_physics(augs::entity_id e) {
		body_definition body;
		body.fixed_rotation = false;

		fixture_definition info;
		info.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 1;

		create_physics_component(body, e);
		add_fixtures(info, e);
	}

	void static_crate_physics(augs::entity_id e) {
		body_definition body;
		body.fixed_rotation = false;
		body.body_type = b2_staticBody;

		fixture_definition info;
		info.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 1;

		create_physics_component(body, e);
		add_fixtures(info, e);
	}
	
}