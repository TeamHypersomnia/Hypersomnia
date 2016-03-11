#pragma once
#include "math/vec2.h"
#include <Box2D/Box2D.h>

#include "physics_setup_helpers.h"
#include "../components/physics_component.h"
#include "../components/fixtures_component.h"
#include "../components/polygon_component.h"
#include "../components/render_component.h"
#include "../components/sprite_component.h"
#include "../detail/state_for_drawing.h"

#include "../systems/physics_system.h"

#include "entity_system/entity_system.h"

#include "3rdparty/polypartition/polypartition.h"

void fixture_definition::add_convex_polygon(const std::vector <vec2>& verts) {
	convex_polys.push_back(verts);
}

void fixture_definition::offset_vertices(components::transform transform) {
	for (auto& c : convex_polys) {
		for (auto& v : c) {
			v.rotate(transform.rotation, vec2(0, 0));
			v += transform.pos;
		}
	}
}

void fixture_definition::mult_vertices(vec2 mult) {
	for (auto& c : convex_polys) {
		for (auto& v : c) { 
			v *= mult;
		}

		std::reverse(c.begin(), c.end());
	}

	for (auto& v : debug_original) {
		v *= mult;
	}
}

#include "texture_baker/texture_baker.h"
#include "../resources/manager.h"

void fixture_definition::from_renderable(augs::entity_id e, bool polygonize_sprite) {
	auto* sprite = e->find<components::sprite>();
	auto* polygon = e->find<components::polygon>();

	if (sprite) {
		auto& polygonized_sprite_verts = resource_manager.find(sprite->tex)->polygonized;
		auto& image_to_polygonize = resource_manager.find(sprite->tex)->img;

		if (polygonized_sprite_verts.size() > 0 && polygonize_sprite) {
			type = POLYGON;

			auto image_size = image_to_polygonize.get_size();
			vec2 polygonized_size(image_size.w, image_size.h);  

			std::vector<vec2> new_concave;

			for (auto v : polygonized_sprite_verts) {
				vec2 new_v = v;
				vec2 scale = sprite->size / polygonized_size;
				
				new_v *= scale;
				new_v.y = -new_v.y;
				new_concave.push_back(new_v);
			}

			auto origin = augs::get_aabb(new_concave).center();
			
			for (auto& v : new_concave)
				v -= origin;

			debug_original = new_concave;
			add_concave_polygon(new_concave);

			mult_vertices(vec2(1, -1));
		}
		else {
			type = RECT;
			rect_size = sprite->size;

			b2PolygonShape shape;
			shape.SetAsBox(static_cast<float>(rect_size.x) / 2.f * PIXELS_TO_METERSf, static_cast<float>(rect_size.y) / 2.f * PIXELS_TO_METERSf);

			std::vector<vec2> new_convex_polygon;

			for (int i = 0; i < shape.GetVertexCount(); ++i)
				new_convex_polygon.push_back(vec2(shape.GetVertex(i))*METERS_TO_PIXELSf);

			add_convex_polygon(new_convex_polygon);
		}
	}
	else if (polygon) {
		type = POLYGON;
		auto& p = *polygon;

		add_concave_polygon(p.original_polygon);
	}

	// TODO: remove the visual components server-side
}

void fixture_definition::add_concave_polygon(const std::vector <vec2> &verts) {
	list<TPPLPoly> inpolys, outpolys;
	TPPLPoly subject_poly;
	subject_poly.Init(verts.size());
	subject_poly.SetHole(false);

	for (size_t i = 0; i < verts.size(); ++i) {
		vec2 p(verts[i]);
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
		
		auto first_v = new_convex[0];

		const int max_vertices = 8;

		if (new_convex.size() > max_vertices) {
			int first = 1;

			while (first + max_vertices - 2 < new_convex.size() - 1) {
				std::vector<vec2> new_poly;
				new_poly.push_back(new_convex[0]);
				new_poly.insert(new_poly.end(), new_convex.begin() + first, new_convex.begin() + first + max_vertices - 2 + 1);
				convex_polys.push_back(new_poly);
				first += max_vertices - 2;
			}

			std::vector<vec2> last_poly;
			last_poly.push_back(new_convex[0]);
			last_poly.push_back(new_convex[first]);
			last_poly.insert(last_poly.end(), new_convex.begin() + first, new_convex.end());

			convex_polys.push_back(last_poly);
		}
		else
			convex_polys.push_back(new_convex);
	}
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

	physics.b2world.CreateJoint(&def);
}

void remove_joints_by_name(augs::entity_id e, joint_name name) {
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

components::fixtures& add_fixtures(fixture_definition fixture_data, augs::entity_id subject) {
	return add_fixtures_to_other_body(fixture_data, subject, subject);
}

components::fixtures& add_fixtures_to_other_body(fixture_definition fixture_data, augs::entity_id subject, augs::entity_id existing_body) {
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

	fixture_data.offset_vertices(fixture_data.transform_vertices);
	auto& fixtures = subject->add<components::fixtures>();
	fixtures.convex_polys = fixture_data.convex_polys;
	fixtures.shape_offset = fixture_data.transform_vertices;
	fixtures.is_friction_ground = fixture_data.is_friction_ground;

	for (auto convex : fixture_data.convex_polys) {
		std::vector<b2Vec2> b2verts(convex.begin(), convex.end());

		for (auto& v : b2verts)
			v *= PIXELS_TO_METERSf;

		shape.Set(b2verts.data(), b2verts.size());
		
		components::fixtures::fixture_state new_convex_part;
		new_convex_part.fixture = body->CreateFixture(&fixdef);
		new_convex_part.index_in_fixture_definitions = fixtures.added_fixture_definitions;

		fixtures.list_of_fixtures.push_back(new_convex_part);
	}

	++fixtures.added_fixture_definitions;

	return fixtures;
}

components::physics& create_physics_component(body_definition body_data, augs::entity_id subject) {
	physics_system& physics = subject->owner_world.get_system<physics_system>();

	auto& transform = subject->get<components::transform>();
	if (subject->find<components::render>())
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
	def.active = body_data.active;

	auto& physics_component = subject->add<components::physics>();
	physics_component.body = physics.b2world.CreateBody(&def);
	physics_component.body->SetAngledDampingEnabled(body_data.angled_damping);

	physics_component.set_velocity(body_data.velocity);
	physics_component.angular_air_resistance = body_data.angular_air_resistance;

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
