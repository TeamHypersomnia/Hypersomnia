#include "game/transcendental/cosmos.h"
#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/sprite_component.h"

#include "game/enums/filters.h"
#include "game/transcendental/entity_handle.h"

namespace ingredients {
	void add_standard_dynamic_body(const logic_step step, const entity_handle e, const bool destructible) {
		components::rigid_body def;
		const auto si = e.get_cosmos().get_si();

		if (e.has<components::transform>()) {
			def.set_transform(si, e.get<components::transform>());
			e.remove<components::transform>();
		}

		e.add_shape_component_from_renderable(
			step
		);

		components::fixtures group;

		group.destructible = destructible;
		group.filter = filters::dynamic_object();
		group.density = 1;

		e += group;
		e += def;
		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_see_through_dynamic_body(const logic_step step, entity_handle e) {
		components::rigid_body def;
		const auto si = e.get_cosmos().get_si();

		if (e.has<components::transform>()) {
			def.set_transform(si, e.get<components::transform>());
			e.remove<components::transform>();
		}

		e.add_shape_component_from_renderable(
			step
		);

		components::fixtures group;

		group.filter = filters::see_through_dynamic_object();
		group.density = 1;
		group.restitution = 0.5f;

		e += group;
		e += def;
		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_shell_dynamic_body(const logic_step step, entity_handle e) {
		components::rigid_body def;
		const auto si = e.get_cosmos().get_si();

		if (e.has<components::transform>()) {
			def.set_transform(si, e.get<components::transform>());
			e.remove<components::transform>();
		}

		e.add_shape_component_from_renderable(
			step
		);

		components::fixtures group;

		group.filter = filters::shell();
		group.density = 1;
		group.restitution = 0.5f;
		group.restitution = 1.4f;
		group.density = 0.001f;
		group.collision_sound_gain_mult = 100.f;

		e += group;
		e += def;

		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_standard_static_body(const logic_step step, entity_handle e) {
		components::rigid_body def;
		def.body_type = rigid_body_type::STATIC;

		const auto si = e.get_cosmos().get_si();

		if (e.has<components::transform>()) {
			def.set_transform(si, e.get<components::transform>());
			e.remove<components::transform>();
		}

		e.add_shape_component_from_renderable(
			step
		);

		components::fixtures group;

		group.filter = filters::dynamic_object();
		group.density = 1;

		e += group;
		e += def;
		e.get<components::fixtures>().set_owner_body(e);
	}
	
	void add_bullet_round_physics(const logic_step step, entity_handle e) {
		components::rigid_body body;
		const auto si = e.get_cosmos().get_si();

		if (e.has<components::transform>()) {
			body.set_transform(si, e.get<components::transform>());
			e.remove<components::transform>();
		}

		body.bullet = true;
		body.angular_damping = 0.f,
		body.linear_damping = 0.f,
		body.gravity_scale = 0.f;
		body.fixed_rotation = false;
		body.angled_damping = false;
		
		e.add_shape_component_from_renderable(
			step
		);

		components::fixtures group;

		group.filter = filters::bullet();
		group.density = 1;
		group.disable_standard_collision_resolution = true;

		e += group;
		e += body;
		e.get<components::fixtures>().set_owner_body(e);
	}
}