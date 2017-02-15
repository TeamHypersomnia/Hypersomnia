#include "game/transcendental/cosmos.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sprite_component.h"

#include "game/enums/filters.h"
#include "game/transcendental/entity_handle.h"

namespace ingredients {
	void add_standard_dynamic_body(const entity_handle e, const bool destructible) {
		components::physics def;

		if (e.has<components::transform>()) {
			def.set_transform(e.get<components::transform>());
			e.remove<components::transform>();
		}

		def.fixed_rotation = false;

		components::fixtures colliders;

		auto& info = colliders.new_collider();
		info.shape.from_sprite(e.get<components::sprite>(), true);
		info.destructible = destructible;

		info.filter = filters::dynamic_object();
		info.density = 1;

		e += def;
		e += colliders;
		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_see_through_dynamic_body(entity_handle e) {
		components::physics def;

		if (e.has<components::transform>()) {
			def.set_transform(e.get<components::transform>());
			e.remove<components::transform>();
		}

		components::fixtures colliders;
		def.fixed_rotation = false;

		auto& info = colliders.new_collider();
		info.shape.from_renderable(e);

		info.filter = filters::see_through_dynamic_object();
		info.density = 1;

		e += def;
		e += colliders;
		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_shell_dynamic_body(entity_handle e) {
		components::physics def;

		if (e.has<components::transform>()) {
			def.set_transform(e.get<components::transform>());
			e.remove<components::transform>();
		}

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
		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_standard_static_body(entity_handle e) {
		components::physics def;

		if (e.has<components::transform>()) {
			def.set_transform(e.get<components::transform>());
			e.remove<components::transform>();
		}

		components::fixtures colliders;

		def.fixed_rotation = false;
		def.body_type = components::physics::type::STATIC;

		auto& info = colliders.new_collider();
		info.shape.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 1;

		e += def;
		e += colliders;
		e.get<components::fixtures>().set_owner_body(e);
	}
	
	void add_bullet_round_physics(entity_handle e) {
		components::physics body;

		if (e.has<components::transform>()) {
			body.set_transform(e.get<components::transform>());
			e.remove<components::transform>();
		}

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
		e.get<components::fixtures>().set_owner_body(e);
	}
}