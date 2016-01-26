#include "utilities/entity_system/entity_id.h"
#include "game_framework/components/physics_component.h"

#include "game_framework/game/body_helper.h"

#include "game_framework/globals/filters.h"

namespace archetypes {
	void crate_physics(augs::entity_id e) {
		helpers::body_definition body;
		body.fixed_rotation = false;

		helpers::fixture_definition info;
		info.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 1;

		helpers::create_physics_component(body, e);
		helpers::add_fixtures(info, e);
	}

	void static_crate_physics(augs::entity_id e) {
		helpers::body_definition body;
		body.fixed_rotation = false;
		body.body_type = b2_staticBody;

		helpers::fixture_definition info;
		info.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 1;

		helpers::create_physics_component(body, e);
		helpers::add_fixtures(info, e);
	}
	
}