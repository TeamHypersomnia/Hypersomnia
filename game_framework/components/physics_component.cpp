#include "physics_component.h"
#include <Box2D\Box2D.h>

#include "graphics/renderer.h"
#include "fixtures_component.h"

#include "math/vec2.h"

namespace components {
	physics& physics::get_owner_body(augs::entity_id id) {
		auto* physics = id->find<components::physics>();
		if (physics) return *physics;

		return id->get<components::fixtures>().get_body_entity()->get<components::physics>();
	}
	
	augs::entity_id physics::get_owner_body_entity(augs::entity_id id) {
		auto* physics = id->find<components::physics>();
		if (physics) return id;

		return id->get<components::fixtures>().get_body_entity();
	}

	bool physics::is_physical(augs::entity_id id) {
		return id->find<components::fixtures>() || id->find<components::physics>();
	}

	bool physics::are_connected_by_friction(augs::entity_id child, augs::entity_id parent) {
		if (components::physics::is_physical(child) && components::physics::is_physical(parent)) {
			bool matched_ancestor = false;

			entity_id parent_body_entity = components::physics::get_owner_body_entity(parent);
			entity_id childs_ancestor_entity = components::physics::get_owner_body(child).get_owner_friction_ground();

			while (childs_ancestor_entity.alive()) {
				if (childs_ancestor_entity == parent_body_entity) {
					matched_ancestor = true;
					break;
				}

				childs_ancestor_entity = childs_ancestor_entity->get<components::physics>().get_owner_friction_ground();
			}

			if (matched_ancestor)
				return true;
		}

		return false;
	}

	void physics::set_velocity(vec2 pixels) {
		body->SetLinearVelocity(pixels * PIXELS_TO_METERSf);
	}	
	
	void physics::set_linear_damping(float damping) {
		body->SetLinearDamping(damping);
	}

	void physics::set_density(float density) {
		b2Fixture* f = body->GetFixtureList();

		while (f) {
			f->SetDensity(density);
			f = f->GetNext();
		}

		body->ResetMassData();
	}
	
	void physics::set_linear_damping_vec(vec2 pixels) {
		body->SetLinearDampingVec(pixels);
	}

	void physics::apply_force(vec2 pixels) {
		apply_force(pixels, vec2(0,0), true);
	}
	
	void physics::apply_force(vec2 pixels, vec2 center_offset, bool wake) {
		vec2 force = pixels * PIXELS_TO_METERSf;
		vec2 location = body->GetWorldCenter() + (center_offset * PIXELS_TO_METERSf);

		body->ApplyForce(force, location, wake);

		if (renderer::get_current().debug_drawing && force.non_zero()) {
			auto& lines = renderer::get_current().logic_lines;
			lines.draw_green(location * METERS_TO_PIXELSf + force * METERS_TO_PIXELSf, location * METERS_TO_PIXELSf);
		}
	}

	void physics::apply_impulse(vec2 pixels) {
		apply_impulse(pixels, vec2(0, 0), true);
	}

	void physics::apply_impulse(vec2 pixels, vec2 center_offset, bool wake) {
		vec2 force = pixels * PIXELS_TO_METERSf;
		vec2 location = body->GetWorldCenter() + (center_offset * PIXELS_TO_METERSf);

		body->ApplyLinearImpulse(force, location, true);

		if (renderer::get_current().debug_drawing && force.non_zero()) {
			auto& lines = renderer::get_current().logic_lines;
			lines.draw_green(location * METERS_TO_PIXELSf + force * METERS_TO_PIXELSf, location * METERS_TO_PIXELSf);
		}
	}

	float physics::get_mass() {
		return body->GetMass();
	}

	vec2 physics::get_position() {
		return METERS_TO_PIXELSf * body->GetPosition();
	}

	vec2 physics::velocity() {
		return vec2(body->GetLinearVelocity()) * METERS_TO_PIXELSf;
	}

	vec2 physics::get_world_center() {
		return METERS_TO_PIXELSf * body->GetWorldCenter();
	}

	entity_id physics::get_owner_friction_ground() {
		return owner_friction_ground;

		//if (owner_friction_grounds.empty()) return entity_id();
		//
		//return owner_friction_grounds[0];
	}

	void physics::set_transform(augs::entity_id id) {
		set_transform(id->get<components::transform>());
	}

	void physics::set_transform(components::transform transform) {
		body->SetTransform(transform.pos * PIXELS_TO_METERSf, transform.rotation * RAD_TO_DEG);
	}
}