#pragma once
#include "math/vec2.h"
#include <Box2D/Box2D.h>

#include "body_helper.h"
#include "../components/physics_component.h"
#include "../components/fixtures_component.h"
#include "../components/polygon_component.h"
#include "../components/render_component.h"
#include "../components/sprite_component.h"
#include "../shared/drawing_state.h"

#include "../systems/physics_system.h"

#include "entity_system/entity_system.h"

#include "3rdparty/polypartition/polypartition.h"

namespace helpers {
	b2World* current_b2world = nullptr;

	void physics_info::add_convex(const std::vector <vec2>& verts) {
		convex_polys.push_back(verts);
	}

	void physics_info::offset_vertices() {
		for (auto& c : convex_polys) {
			for (auto& v : c) {
				v.rotate(transform_vertices.rotation, vec2(0, 0));
				v += transform_vertices.pos;
			}
		}
	}

	void physics_info::from_renderable(augs::entity_id e) {
		auto* sprite = e->find<components::sprite>();
		auto* polygon = e->find<components::polygon>();

		if (sprite) {
			type = RECT;
			rect_size = sprite->size;

			b2PolygonShape shape;
			shape.SetAsBox(static_cast<float>(rect_size.x) / 2.f * PIXELS_TO_METERSf, static_cast<float>(rect_size.y) / 2.f * PIXELS_TO_METERSf);

			convex_polys.resize(1);

			for (int i = 0; i < shape.GetVertexCount(); ++i)
				convex_polys[0].push_back(vec2(shape.GetVertex(i))*METERS_TO_PIXELSf);
		}
		else if (polygon) {
			type = POLYGON;
			auto& p = *polygon;
			
			list<TPPLPoly> inpolys, outpolys;
			TPPLPoly subject_poly;
			subject_poly.Init(p.original_model.size());
			subject_poly.SetHole(false);

			for (size_t i = 0; i < p.original_model.size(); ++i) {
				vec2 p(p.original_model[i]);
				subject_poly[i].x = p.x;
				subject_poly[i].y = -p.y;
			}

			inpolys.push_back(subject_poly);

			TPPLPartition partition;
			partition.ConvexPartition_HM(&inpolys, &outpolys);

			for (auto& out : outpolys) {
				std::vector <vec2> new_convex;

				for (long j = 0; j < out.GetNumPoints(); ++j) {
					new_convex.push_back(vec2(static_cast<float>(out[j].x), static_cast<float>(-out[j].y)));
				}

				std::reverse(new_convex.begin(), new_convex.end());

				convex_polys.push_back(new_convex);
			}
		}

		// TODO: remove the visual components server-side
	}

	void physics_info::add_concave(const std::vector <vec2> &verts) {
		//original_model.insert(original_model.end(), verts.begin(), verts.end());
		//
		//b2Separator separator;
		//std::vector<std::vector<b2Vec2>> output;
		//auto& input = reinterpret_cast<const std::vector<b2Vec2>&>(verts);
		//int res = separator.Validate(input);
		//
		//if (res != 0) {
		//	auto reversed_input = input;
		//	std::reverse(reversed_input.begin(), reversed_input.end());
		//	separator.calcShapes(reversed_input, output);
		//}
		//else separator.calcShapes(input, output);
		//
		//for (auto& convex : output)
		//	convex_polys.push_back(std::vector <vec2> (convex.begin(), convex.end()));
	}

	void create_weld_joint(augs::entity_id a, augs::entity_id b, vec2 orbit_offset, joint_name name) {
		physics_system& physics = a->owner_world.get_system<physics_system>();

		b2WeldJointDef def;
		def.bodyA = a->get<components::physics>().body;
		def.bodyB = b->get<components::physics>().body;
		def.collideConnected = false;
		def.localAnchorB = orbit_offset*PIXELS_TO_METERSf;
		def.localAnchorB.y *= -1;
		def.userData = name;

		physics.b2world.CreateJoint(&def);
	}

	void create_friction_joint(augs::entity_id inside_object, augs::entity_id friction_field, joint_name name) {
		physics_system& physics = inside_object->owner_world.get_system<physics_system>();

		b2FrictionJointDef def;
		def.bodyA = inside_object->get<components::physics>().body;
		def.bodyB = friction_field->get<components::physics>().body;
		def.collideConnected = false;
		def.userData = name;
		def.maxForce = 500000.5;
		def.maxTorque = 10000000;

		//def.localAnchorB = orbit_offset*PIXELS_TO_METERSf;
		//def.localAnchorB.y *= -1;

		physics.b2world.CreateJoint(&def);
	}

	void remove_joints(augs::entity_id e, joint_name name) {
		physics_system& physics = e->owner_world.get_system<physics_system>();

		auto* body = e->get<components::physics>().body;
		auto* je = e->get<components::physics>().body->GetJointList();

		std::vector<b2Joint*> to_remove;

		while (je) {
			if (je->joint->GetUserData() == name) {
				to_remove.push_back(je->joint);
			}
			je = je->next;
		}

		for (auto j : to_remove)
			physics.b2world.DestroyJoint(j);
	}

	bool joint_exists(augs::entity_id e, joint_name name) {
		assert(0);
		return false;
	}

	components::fixtures& add_fixtures(physics_info fixture_data, augs::entity_id subject) {
		return add_fixtures_to_other_body(fixture_data, subject, subject);
	}

	components::fixtures& add_fixtures_to_other_body(physics_info fixture_data, augs::entity_id subject, augs::entity_id existing_body) {
		physics_system& physics = subject->owner_world.get_system<physics_system>();

		b2PolygonShape shape;
		b2CircleShape circle_shape;

		b2FixtureDef fixdef;
		fixdef.density = fixture_data.density;
		fixdef.friction = fixture_data.friction;
		fixdef.isSensor = fixture_data.sensor;
		fixdef.filter = fixture_data.filter;
		fixdef.restitution = fixture_data.restitution;
		fixdef.shape = &shape;
		fixdef.userData = subject;
		
		auto& physics_component = existing_body->get<components::physics>();
		auto body = physics_component.body;
		physics_component.fixture_entities.push_back(subject);

		fixture_data.offset_vertices();
		auto& fixtures = subject->add<components::fixtures>();
		fixtures.convex_polys = fixture_data.convex_polys;
		fixtures.shape_offset = fixture_data.transform_vertices;

		for (auto convex : fixture_data.convex_polys) {
			std::vector<b2Vec2> b2verts(convex.begin(), convex.end());

			for (auto& v : b2verts)
				v *= PIXELS_TO_METERSf;

			shape.Set(b2verts.data(), b2verts.size());
			fixtures.list_of_fixtures.push_back({ body->CreateFixture(&fixdef) });
		}

		return fixtures;
	}

	components::physics& create_physics_component(body_info body_data, augs::entity_id subject) {
		physics_system& physics = subject->owner_world.get_system<physics_system>();

		auto& transform = subject->get<components::transform>();
		if(subject->find<components::render>())
			subject->get<components::render>().previous_transform = subject->get<components::transform>();

		b2BodyDef def;
		def.type = b2BodyType(body_data.body_type);
		def.angle = 0;
		def.userData = subject;
		def.bullet = body_data.bullet;
		def.position = transform.pos*PIXELS_TO_METERSf;
		def.angle = transform.rotation*0.01745329251994329576923690768489f;
		def.angularDamping = body_data.angular_damping;
		def.linearDamping = body_data.linear_damping;
		def.fixedRotation = body_data.fixed_rotation;
		def.gravityScale = body_data.gravity_scale;

		auto& physics_component = subject->add<components::physics>();
		physics_component.body = physics.b2world.CreateBody(&def);
		physics_component.body->SetAngledDampingEnabled(body_data.angled_damping);

		return physics_component;
	}

	std::vector<b2Vec2> get_world_vertices(augs::entity_id subject, bool meters, int fixture_num) {
		std::vector<b2Vec2> output;

		auto b = subject->get<components::physics>().body;

		auto& verts = subject->get<components::fixtures>().convex_polys[fixture_num];
		/* for every vertex in given fixture's shape */
		for (auto& v : verts) {
			auto position = METERS_TO_PIXELSf * b->GetPosition();
			/* transform angle to degrees */
			auto rotation = b->GetAngle() / 0.01745329251994329576923690768489f;
			
			/* transform vertex to current entity's position and rotation */
			vec2 out_vert = (vec2(v).rotate(rotation, b2Vec2(0, 0)) + position);
			
			if (meters) out_vert *= PIXELS_TO_METERSf;
			
			output.push_back(out_vert);
		}

		return output;
	}

	augs::entity_id body_to_entity(b2Body* b) {
		return static_cast<augs::entity_id>(b->GetUserData());
	}
}