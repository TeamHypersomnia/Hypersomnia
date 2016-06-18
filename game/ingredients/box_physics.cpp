#include "game/cosmos.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"

#include "game/globals/filters.h"

#include "game/definition_interface.h"

namespace ingredients {
	void standard_dynamic_body(definition_interface e) {
		auto& physics = e += components::physics();

		rigid_body_definition def;
		def.fixed_rotation = false;

		colliders_definition colliders;

		auto& info = colliders.new_collider();
		info.shape.from_sprite(e.get<components::sprite>(), true);

		info.filter = filters::dynamic_object();
		info.density = 1;

		e += components::fixtures(colliders);
	}

	components::physics_definition& see_through_dynamic_body(entity_id e) {
		auto& physics_definition = *e += components::physics_definition();

		physics_definition.body.fixed_rotation = false;

		auto& info = physics_definition.new_fixture();
		info.from_renderable(e);

		info.filter = filters::see_through_dynamic_object();
		info.density = 1;

		return physics_definition;
	}

	components::physics_definition& standard_static_body(entity_id e) {
		auto& physics_definition = *e += components::physics_definition();

		physics_definition.body.fixed_rotation = false;
		physics_definition.body.body_type = b2_staticBody;

		auto& info = physics_definition.new_fixture();
		info.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 1;

		return physics_definition;
	}
	
	components::physics_definition& bullet_round_physics(entity_id e) {
		auto& physics_definition = *e += components::physics_definition();

		auto& body = physics_definition.body;
		body.bullet = true;
		body.angular_damping = 0.f,
		body.linear_damping = 0.f,
		body.gravity_scale = 0.f;
		body.angular_air_resistance = 0.f;
		body.fixed_rotation = false;
		body.angled_damping = false;
		
		auto& info = physics_definition.new_fixture();
		info.from_renderable(e);

		info.filter = filters::bullet();
		info.density = 1;
		info.disable_standard_collision_resolution = true;

		return physics_definition;
	}
}