#include "3rdparty/Box2D/Box2D.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/stateless_systems/physics_system.h"

void physics_system::post_and_clear_accumulated_collision_messages(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	auto& physics = cosmos.get_solvable_inferred({}).physics;
	
	step.post_message(physics.accumulated_messages);
	physics.accumulated_messages.clear();
}

void physics_system::step_and_set_new_transforms(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	auto& physics = cosmos.get_solvable_inferred({}).physics;

	const auto delta = step.get_delta();

	int32 velocityIterations = 8;
	int32 positionIterations = 3;

	physics.ray_casts_since_last_step = 0;

	physics.b2world->Step(static_cast<float32>(delta.in_seconds()), velocityIterations, positionIterations);

	post_and_clear_accumulated_collision_messages(step);

	for (b2Body* b = physics.b2world->GetBodyList(); b != nullptr; b = b->GetNext()) {
		if (b->GetType() == b2_staticBody) continue;
		entity_handle entity = cosmos[b->GetUserData()];

		physics.recurential_friction_handler(step, b, b->m_ownerFrictionGround);

		auto& body = entity.get<components::rigid_body>().get_raw_component();
		
		body.transform = b->m_xf;
		body.sweep = b->m_sweep;
		body.velocity = vec2(b->GetLinearVelocity());
		body.angular_velocity = b->GetAngularVelocity();
	}
}

