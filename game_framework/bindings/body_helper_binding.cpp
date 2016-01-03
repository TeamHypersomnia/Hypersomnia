#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../shared/drawing_state.h"
#include "../game/body_helper.h"

#include "entity_system/entity.h"

namespace bindings {
	luabind::scope _body_helper() {
		return (
			luabind::class_ <physics_info>("physics_info")
			.def(luabind::constructor<>())
			.def("from_renderable", &physics_info::from_renderable)
			.def_readwrite("rect_size", &physics_info::rect_size)
			.def_readwrite("shape_type", &physics_info::type)
			.def_readwrite("filter", &physics_info::filter)
			.def_readwrite("density", &physics_info::density)
			.def_readwrite("radius", &physics_info::radius)
			.def_readwrite("friction", &physics_info::friction)
			.def_readwrite("restitution", &physics_info::restitution)
			.def_readwrite("angular_damping", &physics_info::angular_damping)
			.def_readwrite("gravity_scale", &physics_info::gravity_scale)
			.def_readwrite("linear_damping", &physics_info::linear_damping)
			.def_readwrite("fixed_rotation", &physics_info::fixed_rotation)
			.def_readwrite("angled_damping", &physics_info::angled_damping)
			.def_readwrite("sensor", &physics_info::sensor)
			.def_readwrite("bullet", &physics_info::bullet)
			//.def("add_vertex", &physics_info::add_vertex)
			.enum_("constants")[
				luabind::value("POLYGON", physics_info::POLYGON),
				luabind::value("CIRCLE", physics_info::CIRCLE),
				luabind::value("RECT", physics_info::RECT)
			],

			luabind::class_ <b2JointDef>("b2JointDef")
			.def(luabind::constructor<>())
			.def_readwrite("bodyA", &b2JointDef::bodyA)
			.def_readwrite("bodyB", &b2JointDef::bodyB)
			.def_readwrite("collideConnected", &b2JointDef::collideConnected)
			,

			luabind::class_ <b2RevoluteJoint>("b2RevoluteJoint")
			.def("GetJointAngle", &b2RevoluteJoint::GetJointAngle)
			,

			luabind::class_ <b2RevoluteJointDef, b2JointDef>("b2RevoluteJointDef")
			.def(luabind::constructor<>())
			.def_readwrite("localAnchorA", &b2RevoluteJointDef::localAnchorA)
			.def_readwrite("localAnchorB", &b2RevoluteJointDef::localAnchorB)
			.def_readwrite("referenceAngle", &b2RevoluteJointDef::referenceAngle)
			.def_readwrite("enableLimit", &b2RevoluteJointDef::enableLimit)
			.def_readwrite("lowerAngle", &b2RevoluteJointDef::lowerAngle)
			.def_readwrite("upperAngle", &b2RevoluteJointDef::upperAngle)
			,


			luabind::class_ <b2WeldJoint>("b2WeldJoint")
			//.def("GetJointAngle", &b2RevoluteJoint::GetJointAngle)
			//.def(luabind::constructor<>())
			,

			luabind::class_ <b2WeldJointDef, b2JointDef>("b2WeldJointDef")
			.def(luabind::constructor<>())
			.def("Initialize", &b2WeldJointDef::Initialize)
			.def_readwrite("localAnchorA", &b2WeldJointDef::localAnchorA)
			.def_readwrite("localAnchorB", &b2WeldJointDef::localAnchorB)
			.def_readwrite("referenceAngle", &b2WeldJointDef::referenceAngle)
			.def_readwrite("frequencyHz", &b2WeldJointDef::frequencyHz)
			.def_readwrite("dampingRatio", &b2WeldJointDef::dampingRatio)
			,
			
			//luabind::def("create_joint", create_joint<b2RevoluteJoint, b2RevoluteJointDef>),
			//luabind::def("create_joint", create_joint<b2WeldJoint, b2WeldJointDef>),
			luabind::def("create_physics_component", create_physics_component),
			luabind::def("body_to_entity", body_to_entity)
		);
	}
}