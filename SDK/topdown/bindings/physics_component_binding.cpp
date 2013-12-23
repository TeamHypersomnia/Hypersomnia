#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/physics_component.h"
#include "../systems/physics_system.h"

#include "entity_system/entity.h"
#include <luabind/iterator_policy.hpp>

void set_density(b2Body* body, float density) {
	for (b2Fixture* it = body->GetFixtureList(); it; it = it->GetNext()) {
		it->SetDensity(density);
	}

	body->ResetMassData();
}

struct dummy_Box2D {

};

namespace bindings {
	luabind::scope _physics_component() {
		return
			(
			luabind::def("SetDensity", set_density),
			luabind::class_<dummy_Box2D>("Box2D")
			.enum_("constants")	[
				luabind::value("b2_dynamicBody", b2_dynamicBody),
				luabind::value("b2_staticBody", b2_staticBody),
				luabind::value("b2_kinematicBody", b2_kinematicBody)
			],

			luabind::class_<physics_system::raycast_output>("raycast_output")
			.def(luabind::constructor<>())
			.def_readwrite("intersection", &physics_system::raycast_output::intersection)
			.def_readwrite("hit", &physics_system::raycast_output::hit)
			.def_readwrite("normal", &physics_system::raycast_output::normal)
			.def_readwrite("what_entity", &physics_system::raycast_output::what_entity)
			,

			luabind::class_<physics_system::query_output>("query_output")
			.def(luabind::constructor<>())
			.def_readwrite("bodies", &physics_system::query_output::bodies, luabind::return_stl_iterator)
			,

			luabind::class_<physics_system>("_physics_system")
			.def_readwrite("timestep_multiplier", &physics_system::timestep_multiplier)
			.def_readwrite("enable_interpolation", &physics_system::enable_interpolation)
			.def("ray_cast", &physics_system::ray_cast_px)
			.def("query_aabb", &physics_system::query_aabb_px)
			.def("query_body", &physics_system::query_body)
			.def("push_away_from_walls", &physics_system::push_away_from_walls)
			, 

			luabind::class_<physics>("physics_component")
			.def(luabind::constructor<>())
			.def_readwrite("body", &physics::body),
			
			luabind::class_<b2Vec2>("b2Vec2")
			.def(luabind::constructor<float, float>())
			.def(luabind::constructor<>())
			.def_readwrite("x", &b2Vec2::x)
			.def_readwrite("y", &b2Vec2::y),

			luabind::class_<b2Body>("b2Body")
			.def("ApplyForce", &b2Body::ApplyForce)
			.def("ApplyLinearImpulse", &b2Body::ApplyLinearImpulse)
			.def("GetWorldCenter", &b2Body::GetWorldCenter)
			.def("SetFixedRotation", &b2Body::SetFixedRotation)
			.def("SetLinearDamping", &b2Body::SetLinearDamping)
			.def("SetLinearVelocity", &b2Body::SetLinearVelocity)
			.def("SetAngularDamping", &b2Body::GetAngularDamping)
			.def("ApplyAngularImpulse", &b2Body::ApplyAngularImpulse)
			.def("ApplyTorque", &b2Body::ApplyTorque)
			.def("GetLinearVelocity", &b2Body::GetLinearVelocity)
			.def("GetAngularVelocity", &b2Body::GetAngularVelocity)
			);


	}
}