#include "game/cosmos.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sprite_component.h"

#include "game/enums/filters.h"
#include "game/entity_handle.h"

namespace ingredients {
	void standard_dynamic_body(entity_handle e) {
		components::physics def;
		def.fixed_rotation = false;

		components::fixtures colliders;

		auto& info = colliders.new_collider();
		info.shape.from_sprite(e.get<components::sprite>(), true);

		info.filter = filters::dynamic_object();
		info.density = 1;

		e += def;
		e += colliders;
	}

	void see_through_dynamic_body(entity_handle e) {
		components::physics def;
		components::fixtures colliders;
		def.fixed_rotation = false;

		auto& info = colliders.new_collider();
		info.shape.from_renderable(e);

		info.filter = filters::see_through_dynamic_object();
		info.density = 1;

		e += def;
		e += colliders;
	}

	void shell_dynamic_body(entity_handle e) {
		components::physics def;
		components::fixtures colliders;
		def.fixed_rotation = false;

		auto& info = colliders.new_collider();
		info.shape.from_renderable(e);

		info.filter = filters::shell();
		info.density = 1.f;
		info.restitution = 1.4f;
		info.density = 0.001f;

		e += def;
		e += colliders;
	}

	void standard_static_body(entity_handle e) {
		components::physics def;
		components::fixtures colliders;

		def.fixed_rotation = false;
		def.body_type = components::physics::type::STATIC;

		auto& info = colliders.new_collider();
		info.shape.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 1;

		e += def;
		e += colliders;
	}
	
	void bullet_round_physics(entity_handle e) {
		components::physics body;
		components::fixtures colliders;

		body.bullet = true;
		body.angular_damping = 0.f,
		body.linear_damping = 0.f,
		body.gravity_scale = 0.f;
		body.fixed_rotation = false;
		body.angled_damping = false;
		
		colliders.disable_standard_collision_resolution = true;

		auto& info = colliders.new_collider();
		info.shape.from_renderable(e);

		info.filter = filters::bullet();
		info.density = 1;

		e += body;
		e += colliders;
	}
}