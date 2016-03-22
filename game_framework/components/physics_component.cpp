#include "physics_component.h"
#include "item_component.h"
#include "driver_component.h"

#include <Box2D\Box2D.h>

#include "graphics/renderer.h"
#include "fixtures_component.h"
#include "physics_definition_component.h"
#include "../messages/collision_message.h"

#include "math/vec2.h"
#include "entity_system/world.h"

namespace components {
	augs::entity_id physics::get_owner_friction_field(augs::entity_id id) {
		return get_owner_body_entity(id)->get<components::physics>().owner_friction_ground;
	}
	
	augs::entity_id physics::get_owner_body_entity(augs::entity_id id) {
		auto* fixtures = id->find<components::fixtures>();
		if (fixtures) return fixtures->get_body_entity();
		else if (id->find<components::physics>()) return id;
		assert(0);
		return augs::entity_id();
	}

	bool physics::is_entity_physical(augs::entity_id id) {
		return id->find<components::fixtures>() || id->find<components::physics>();
	}

	bool physics::are_connected_by_friction(augs::entity_id child, augs::entity_id parent) {
		if (components::physics::is_entity_physical(child) && components::physics::is_entity_physical(parent)) {
			bool matched_ancestor = false;

			entity_id parent_body_entity = components::physics::get_owner_body_entity(parent);
			entity_id childs_ancestor_entity = components::physics::get_owner_body_entity(child)->get<components::physics>().get_owner_friction_ground();

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

	float physics::get_angle() {
		return body->GetAngle() * RAD_TO_DEG;
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

	vec2 physics::get_aabb_size() {
		b2AABB aabb;
		aabb.lowerBound.Set(FLT_MAX, FLT_MAX);
		aabb.upperBound.Set(-FLT_MAX, -FLT_MAX);

		b2Fixture* fixture = body->GetFixtureList();
		
		while (fixture != nullptr) {
			aabb.Combine(aabb, fixture->GetAABB(0));
			fixture = fixture->GetNext();
		}

		return vec2(aabb.upperBound.x - aabb.lowerBound.x, aabb.upperBound.y - aabb.lowerBound.y);
	}

	void physics::set_active(augs::entity_id id, bool active) {
		if (id->find<components::physics>() == nullptr)
			id->get<components::physics_definition>().body.active = active;
		else	
			id->get<components::physics>().body->SetActive(active);
	}

	void physics::recreate_fixtures_and_attach_to(augs::entity_id from_fixture_entity, augs::entity_id to_body_entity, components::transform offset_created_shapes) {
		destroy_physics_of_entity(from_fixture_entity);
		auto& def = from_fixture_entity->get<components::physics_definition>();

		def.attach_fixtures_to_entity = to_body_entity;
		def.offset_created_shapes = offset_created_shapes;
		def.create_fixtures_and_body = true;

		auto& physics = from_fixture_entity->get_owner_world().get_system<physics_system>();
		physics.create_physics_for_entity(from_fixture_entity);
	}

	void physics::destroy_physics_of_entity(augs::entity_id id) {
		id->get<components::physics_definition>().create_fixtures_and_body = false;
		auto& physics = id->get_owner_world().get_system<physics_system>();
		physics.destroy_physics_of_entity(id);

		for (auto& c : physics.parent_world.get_message_queue<messages::collision_message>()) {
			if (c.subject == id || c.collider == id) {
				c.delete_this_message = true;
			}
		}

		physics.parent_world.delete_marked_messages<messages::collision_message>();
	}

	void physics::resolve_density_of_associated_fixtures(augs::entity_id id) {
		auto* maybe_physics = id->find<components::physics>();

		if (maybe_physics) {
			for (auto& f : maybe_physics->fixture_entities) {
				if(f != id)
					resolve_density_of_associated_fixtures(f);
			}
		}

		auto& fixtures = id->get<components::fixtures>();
		auto& definition = id->get<components::physics_definition>();

		float density_multiplier = 1.f;

		auto* item = id->find<components::item>();

		if (item != nullptr && item->current_slot.alive() && item->current_slot.should_item_inside_keep_physical_body())
			density_multiplier *= item->current_slot.calculate_density_multiplier_due_to_being_attached();

		auto owner_body = get_owner_body_entity(id);
		auto* driver = owner_body->find<components::driver>();

		if (driver) {
			if (driver->owned_vehicle.alive()) {
				density_multiplier *= driver->density_multiplier_while_driving;
			}
		}

		for (auto& f : fixtures.list_of_fixtures) {
			auto& fixdef = definition.fixtures[f.index_in_fixture_definitions];
			f.fixture->SetDensity(fixdef.density * density_multiplier);
		}

		fixtures.get_body()->ResetMassData();
	}

	entity_id physics::get_owner_friction_ground() {
		return owner_friction_ground;
	}

	void physics::set_transform(augs::entity_id id) {
		set_transform(id->get<components::transform>());
	}

	void physics::set_transform(components::transform transform) {
		body->SetTransform(transform.pos * PIXELS_TO_METERSf, transform.rotation * RAD_TO_DEG);
	}
}