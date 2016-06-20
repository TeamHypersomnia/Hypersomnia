#include "physics_system.h"

#include "game/entity_id.h"
#include "game/cosmos.h"

#include "game/components/item_component.h"
#include "game/components/driver_component.h"
#include "game/components/fixtures_component.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/new_entity_message.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/cosmos.h"
#include "game/step_state.h"

double METERS_TO_PIXELS = 100.0;
double PIXELS_TO_METERS = 1.0 / METERS_TO_PIXELS;
float METERS_TO_PIXELSf = 100.f;
float PIXELS_TO_METERSf = 1.0f / METERS_TO_PIXELSf;

physics_system::physics_system(cosmos& parent_cosmos) : parent_cosmos(parent_cosmos),
b2world(b2Vec2(0.f, 0.f)), ray_casts_since_last_step(0) {
	b2world.SetAllowSleeping(false);
	b2world.SetAutoClearForces(false);
	enable_listener(true);
}

void physics_system::enable_listener(bool flag) {
	b2world.SetContactListener(flag ? &listener : nullptr);
}

void physics_system::step_and_set_new_transforms(step_state& step) {
	listener.cosmos_ptr = &parent_cosmos;
	listener.step_ptr = &step;

	int32 velocityIterations = 8;
	int32 positionIterations = 3;

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;

		auto& physics = static_cast<entity_id>(b->GetUserData()).get<components::physics>();
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
			physics.black_detail.body->ApplyTorque((angular_resistance * angular_speed * angular_speed )* -sgn(angular_speed) * b->GetInertia(), true);
		}

		if (physics.enable_angle_motor) {
			float next_angle = b->GetAngle() + b->GetAngularVelocity() / static_cast<float>(parent_cosmos.delta.get_steps_per_second());
			
			auto target_orientation = vec2().set_from_degrees(physics.target_angle);
			auto next_orientation = vec2().set_from_radians(next_angle);

			float total_rotation = target_orientation.radians_between(next_orientation);

			if (target_orientation.cross(next_orientation) > 0)
				total_rotation *= -1;

			float desired_angular_velocity = total_rotation / static_cast<float>(parent_cosmos.delta.in_seconds());
			float impulse = b->GetInertia() * desired_angular_velocity;// disregard time factor
			b->ApplyAngularImpulse(impulse * physics.angle_motor_force_multiplier, true);
		}
	}

	listener.after_step_callbacks.clear();

	ray_casts_since_last_step = 0;
	b2world.Step(static_cast<float32>(parent_cosmos.delta.in_seconds()), velocityIterations, positionIterations);
	b2world.ClearForces();
	
	for (auto& c : listener.after_step_callbacks)
		c();

	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto entity = b->GetUserData();
		auto& physics = entity.get<components::physics>();
		
		recurential_friction_handler(b->GetUserData(), physics.get_owner_friction_ground());
	}

	set_transforms_from_body_transforms();
}

void physics_system::set_transforms_from_body_transforms() {
	for (b2Body* b = b2world.GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		auto& transform = static_cast<entity_id>(b->GetUserData()).get<components::transform>();
		auto& physics = static_cast<entity_id>(b->GetUserData()).get<components::physics>();

		auto body_pos = METERS_TO_PIXELSf * b->GetPosition();
		auto body_angle = b->GetAngle() * RAD_TO_DEG;

		for (auto& fe : physics.black_detail.fixture_entities) {
			auto& fixtures = fe.get<components::fixtures>();
			auto total_offset = fixtures.get_total_offset();

			auto& transform = fe.get<components::transform>();
			transform.pos = body_pos;

			if (!b->IsFixedRotation())
				transform.rotation = body_angle;

			transform.pos += total_offset.pos;
			transform.rotation += total_offset.rotation;

			transform.pos.rotate(body_angle, body_pos);
		}

		transform.pos = body_pos;

		if (!b->IsFixedRotation())
			transform.rotation = body_angle;
	}
}

void physics_system::react_to_destroyed_entities(step_state& step) {
	auto& events = step.messages.get_queue<messages::will_soon_be_deleted>();

	for (auto& it : events) {
		auto e = it.subject;

		auto* maybe_physics = e.find<components::physics>();
		auto* maybe_fixtures = e.find<components::fixtures>();

		if (maybe_physics)
			maybe_physics->destroy_body();

		if (maybe_fixtures)
			maybe_fixtures->destroy_fixtures();
	}
}

void physics_system::react_to_new_entities(step_state& step) {
	auto& events = step.messages.get_queue<messages::new_entity_message>();

	for (auto& it : events) {
		auto e = it.subject;

		auto* maybe_physics = e.find<components::physics>();
		auto* maybe_fixtures = e.find<components::fixtures>();

		if (maybe_physics) {
			maybe_physics->black_detail.body_owner = e;
			maybe_physics->black_detail.parent_system = this;

			if (maybe_physics->should_body_exist_now()) {
				maybe_physics->build_body();
			}
		}

		if (maybe_fixtures) {
			if (maybe_fixtures->get_owner_body().dead()) {
				maybe_fixtures->set_owner_body(e);
			}

			maybe_fixtures->black_detail.all_fixtures_owner = e;
			maybe_fixtures->black_detail.parent_system = this;

			if (maybe_fixtures->should_fixtures_exist_now()) {
				maybe_fixtures->build_fixtures();
			}
		}
	}
}

entity_id physics_system::get_owner_friction_field(entity_id id) {
	return get_owner_body_entity(id).get<components::physics>().owner_friction_ground;
}

entity_id physics_system::get_owner_body_entity(entity_id id) {
	auto* fixtures = id.find<components::fixtures>();
	if (fixtures) return fixtures->get_body_entity();
	else if (id.find<components::physics>()) return id;
	return entity_id();
}

bool physics_system::is_entity_physical(entity_id id) {
	return id.find<components::fixtures>() || id.find<components::physics>();
}

void physics_system::resolve_density_of_associated_fixtures(entity_id id) {
	auto* maybe_physics = id.find<components::physics>();

	if (maybe_physics) {
		const auto& entities = maybe_physics->get_fixture_entities();

		for (auto& f : entities) {
			if (f != id)
				resolve_density_of_associated_fixtures(f);
		}
	}

	auto& fixtures = id.get<components::fixtures>();

	float density_multiplier = 1.f;

	auto* item = id.find<components::item>();

	if (item != nullptr && item->current_slot.alive() && item->current_slot.should_item_inside_keep_physical_body())
		density_multiplier *= item->current_slot.calculate_density_multiplier_due_to_being_attached();

	auto owner_body = get_owner_body_entity(id);
	auto* driver = owner_body.find<components::driver>();

	if (driver) {
		if (driver->owned_vehicle.alive()) {
			density_multiplier *= driver->density_multiplier_while_driving;
		}
	}

	for(size_t i = 0; i < fixtures.get_num_colliders(); ++i)
		fixtures.set_density_multiplier(density_multiplier, i);
}

std::vector<b2Vec2> physics_system::get_world_vertices(entity_id subject, bool meters, int fixture_num) {
	std::vector<b2Vec2> output;

	auto& b = subject.get<components::physics>();

	auto& verts = subject.get<components::fixtures>().get_definition().colliders[0].shape.convex_polys[fixture_num];

	/* for every vertex in given fixture's shape */
	for (auto& v : verts) {
		auto position = b.get_position();
		/* transform angle to degrees */
		auto rotation = b.get_angle();

		/* transform vertex to current entity's position and rotation */
		vec2 out_vert = (vec2(v).rotate(rotation, b2Vec2(0, 0)) + position);

		if (meters) out_vert *= PIXELS_TO_METERSf;

		output.push_back(out_vert);
	}

	return output;
}
