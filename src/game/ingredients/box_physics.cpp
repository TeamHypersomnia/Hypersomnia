#include "game/transcendental/cosmos.h"
#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/sprite_component.h"

#include "game/enums/filters.h"
#include "game/transcendental/entity_handle.h"

namespace ingredients {
	void add_standard_dynamic_body(const logic_step step, const entity_handle e, const components::transform target_transform, const bool destructible) {
		components::rigid_body body;
		const auto si = e.get_cosmos().get_si();

		body.set_transform(si, target_transform);

		e.add_shape_component_from_renderable(
			step
		);

		components::fixtures group;

		group.destructible = destructible;
		group.filter = filters::dynamic_object();
		group.density = 1;
		group.material = assets::physical_material_id::METAL;

		e += group;
		e += body;
		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_see_through_dynamic_body(const logic_step step, entity_handle e, const components::transform target_transform) {
		components::rigid_body body;
		const auto si = e.get_cosmos().get_si();

		body.set_transform(si, target_transform);

		e.add_shape_component_from_renderable(
			step
		);

		components::fixtures group;

		group.filter = filters::see_through_dynamic_object();
		group.density = 0.2;
		group.restitution = 0.5f;
		group.material = assets::physical_material_id::METAL;

		e += group;
		e += body;
		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_shell_dynamic_body(const logic_step step, entity_handle e, const components::transform target_transform) {
		components::rigid_body body;
		
		const auto si = e.get_cosmos().get_si();
		
		body.set_transform(si, target_transform);

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
		group.material = assets::physical_material_id::METAL;

		e += group;
		e += body;

		e.get<components::fixtures>().set_owner_body(e);
	}

	void add_standard_static_body(const logic_step step, entity_handle e, const components::transform target_transform) {
		components::rigid_body body;
		body.body_type = rigid_body_type::STATIC;

		const auto si = e.get_cosmos().get_si();

		body.set_transform(si, target_transform);

		e.add_shape_component_from_renderable(
			step
		);

		components::fixtures group;

		group.filter = filters::dynamic_object();
		group.density = 1;
		group.material = assets::physical_material_id::METAL;

		e += group;
		e += body;
		e.get<components::fixtures>().set_owner_body(e);
	}
	
	void add_bullet_round_physics(const logic_step step, entity_handle e, const components::transform target_transform) {
		components::rigid_body body;
		const auto si = e.get_cosmos().get_si();

		body.set_transform(si, target_transform);

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
		group.material = assets::physical_material_id::METAL;

		e += group;
		e += body;
		e.get<components::fixtures>().set_owner_body(e);
	}
}