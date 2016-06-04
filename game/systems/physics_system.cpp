#include "physics_system.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/collision_message.h"
#include "../messages/destroy_message.h"
#include "../messages/new_entity_message.h"
#include "../messages/rebuild_physics_message.h"
#include "../messages/physics_operation.h"

double METERS_TO_PIXELS = 100.0;
double PIXELS_TO_METERS = 1.0 / METERS_TO_PIXELS;
float METERS_TO_PIXELSf = 100.f;
float PIXELS_TO_METERSf = 1.0f / METERS_TO_PIXELSf;

physics_system::physics_system(world& parent_world) : event_only_system(parent_world),
b2world(b2Vec2(0.f, 0.f)), ray_casts_since_last_step(0) {
	b2world.SetAllowSleeping(false);
	b2world.SetAutoClearForces(false);
	enable_listener(true);
}

void physics_system::enable_listener(bool flag) {
	b2world.SetContactListener(flag ? &listener : nullptr);
}

void physics_system::step_and_set_new_transforms() {
	listener.world_ptr = &parent_world;

	int32 velocityIterations = 8;
	int32 positionIterations = 3;

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;

		auto& physics = static_cast<entity_id>(b->GetUserData())->get<components::physics>();
		physics.measured_carried_mass = 0.f;

		b2Vec2 vel(b->GetLinearVelocity());
		float32 speed = vel.Normalize();
		float32 angular_speed = b->GetAngularVelocity();

		//if (physics.air_resistance > 0.f) {
		//	auto force_dir = physics.get_mass() * -vel;
		//	auto force_components = (physics.air_resistance  * speed * speed);
		//
		//	//if (speed > 1.0)
		//	//	force_components += (0.5f * sqrt(std::abs(speed)));
		//	
		//	physics.body->ApplyForce(force_components * force_dir, physics.body->GetWorldCenter(), true);
		//}

		auto angular_resistance = physics.angular_air_resistance;
		if (angular_resistance < 0.f) angular_resistance = physics.air_resistance;
		
		if (angular_resistance > 0.f) {
			//physics.body->ApplyTorque((angular_resistance * sqrt(sqrt(angular_speed * angular_speed)) + 0.2 * angular_speed * angular_speed)* -sgn(angular_speed) * b->GetInertia(), true);
			physics.body->ApplyTorque((angular_resistance * angular_speed * angular_speed )* -sgn(angular_speed) * b->GetInertia(), true);
		}

		if (physics.enable_angle_motor) {
			float next_angle = b->GetAngle() + b->GetAngularVelocity() / static_cast<float>(parent_overworld.delta_timer.get_steps_per_second());
			
			auto target_orientation = vec2().set_from_degrees(physics.target_angle);
			auto next_orientation = vec2().set_from_radians(next_angle);

			float total_rotation = target_orientation.radians_between(next_orientation);

			if (target_orientation.cross(next_orientation) > 0)
				total_rotation *= -1;

			float desired_angular_velocity = total_rotation / static_cast<float>(parent_overworld.delta_timer.delta_seconds());
			float impulse = b->GetInertia() * desired_angular_velocity;// disregard time factor
			b->ApplyAngularImpulse(impulse * physics.angle_motor_force_multiplier, true);
		}
	}

	listener.after_step_callbacks.clear();

	ray_casts_since_last_step = 0;
	b2world.Step(static_cast<float32>(delta_seconds()), velocityIterations, positionIterations);
	b2world.ClearForces();
	
	for (auto& c : listener.after_step_callbacks)
		c();

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto entity = b->GetUserData();
		auto& physics = entity->get<components::physics>();
		
		recurential_friction_handler(b->GetUserData(), physics.get_owner_friction_ground());
	}

	reset_states();
}

void physics_system::destroy_whole_world() {
	std::vector<b2Body*> to_destroy;

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext())
		to_destroy.push_back(b);

	for (auto& just_die : to_destroy)
		b2world.DestroyBody(just_die);
}

void physics_system::reset_states() {
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto& transform = static_cast<entity_id>(b->GetUserData())->get<components::transform>();
		auto& physics = static_cast<entity_id>(b->GetUserData())->get<components::physics>();

		auto body_pos = METERS_TO_PIXELSf * b->GetPosition();
		auto body_angle = b->GetAngle() * RAD_TO_DEG;

		for (auto& fe : physics.fixture_entities) {
			auto& fixtures = fe->get<components::fixtures>();

			auto& transform = fe->get<components::transform>();
			transform.pos = body_pos;

			if (!b->IsFixedRotation())
				transform.rotation = body_angle;

			transform.pos += fixtures.shape_offset.pos;
			transform.rotation += fixtures.shape_offset.rotation;

			transform.pos.rotate(body_angle, body_pos);
		}

		transform.pos = body_pos;

		if (!b->IsFixedRotation())
			transform.rotation = body_angle;
	}
}

void physics_system::create_physics_for_entity(augs::entity_id e) {
	ensure(e->find<components::physics>() == nullptr);
	ensure(e->find<components::fixtures>() == nullptr);

	auto* physics_definition = e->find<components::physics_definition>();

	if (physics_definition) {
		if (physics_definition->create_fixtures_and_body) {
			auto other_body_entity = physics_definition->attach_fixtures_to_entity;

			if (other_body_entity.dead() || other_body_entity == e)
				create_physics_component(physics_definition->body, e);

			for (auto fixture : physics_definition->fixtures) {
				for(auto& offset : physics_definition->offsets_for_created_shapes)
					fixture.transform_vertices += offset;

				if (other_body_entity.alive())
					add_fixtures_to_other_body(fixture, e, physics_definition->attach_fixtures_to_entity);
				else
					add_fixtures(fixture, e);
			}

			components::physics::resolve_density_of_associated_fixtures(e);
		}
	}
}

bool physics_system::has_entity_any_physics(augs::entity_id subject) {
	return subject->find<components::physics>() || subject->find<components::fixtures>();
}

void physics_system::clear_collision_messages() {
	auto& collisions = parent_world.get_message_queue<messages::collision_message>();
	collisions.clear();
}

void physics_system::destroy_fixtures_of_entity(augs::entity_id subject) {
	auto* maybe_fixtures = subject->find<components::fixtures>();

	if (maybe_fixtures) {
		auto& fixture_list = maybe_fixtures->get_body_entity()->get<components::physics>().fixture_entities;
		fixture_list.erase(std::remove(fixture_list.begin(), fixture_list.end(), subject), fixture_list.end());

		for (auto f : maybe_fixtures->list_of_fixtures)
			f.fixture->GetBody()->DestroyFixture(f.fixture);

		subject->remove<components::fixtures>();
	}
}

void physics_system::destroy_physics_of_entity(augs::entity_id subject) {
	destroy_fixtures_of_entity(subject);

	auto* maybe_physics = subject->find<components::physics>();

	if (maybe_physics) {
		auto all_fixture_entities = maybe_physics->fixture_entities;
		
		for (auto fe : all_fixture_entities)
			destroy_fixtures_of_entity(fe);

		b2world.DestroyBody(maybe_physics->body);

		subject->remove<components::physics>();
	}
}

void physics_system::consume_rebuild_physics_messages_and_save_new_definitions() {
	auto& events = parent_world.get_message_queue<messages::rebuild_physics_message>();

	for (auto& it : events) {
		if (it.subject->find<components::physics_definition>() == nullptr)
			it.subject->add(it.new_definition);
		else
			it.subject->get<components::physics_definition>() = it.new_definition;

		destroy_physics_of_entity(it.subject);
		create_physics_for_entity(it.subject);
	}

	events.clear();
}

void physics_system::execute_delayed_physics_ops() {
	auto& events = parent_world.get_message_queue<messages::physics_operation>();

	for (auto& e : events) {
		auto& physics = e.subject->get<components::physics>();

		if (e.apply_force.non_zero())
			physics.apply_force(e.apply_force, e.force_offset);

		if (e.reset_drop_timeout) {
			physics.since_dropped.set(e.timeout_ms);
			reset(physics.since_dropped);
		}

		if (e.set_velocity)
			physics.set_velocity(e.velocity);
	}

	events.clear();
}

void physics_system::create_bodies_and_fixtures_from_physics_definitions() {
	auto& events = parent_world.get_message_queue<messages::new_entity_message>();

	for (auto& it : events) {
		if(!has_entity_any_physics(it.subject))
			create_physics_for_entity(it.subject);
	}
}

void physics_system::destroy_fixtures_and_bodies() {
	auto& to_destroy = parent_world.get_message_queue<messages::destroy_message>();

	for (auto& m : to_destroy)
		destroy_physics_of_entity(m.subject);
}
