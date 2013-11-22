#include "stdafx.h"
#include "body_helper.h"
#include "../components/physics_component.h"
#include "../components/render_component.h"
#include "../resources/render_info.h"

#include "../systems/physics_system.h"

#include "entity_system/entity_system.h"

#include "b2Separator/b2Separator.h"

namespace topdown {
	physics_info::physics_info() 
		: rect_size(augmentations::vec2<>()), type(RECT), density(1.f), angular_damping(0.f), linear_damping(0.f), fixed_rotation(false), sensor(false) {
	}

	b2World* current_b2world = nullptr;

	void physics_info::add_convex(const std::vector < augmentations::vec2 < >>& verts) {
		convex_polys.push_back(verts);
	}

	void physics_info::from_renderable(const resources::polygon& p) {
		std::vector<augmentations::vec2<>> concave_poly;

		for (auto& v : p.model)
			concave_poly.push_back(augmentations::vec2<>(v.position.x, v.position.y));

		add_concave(concave_poly);
	}

	void physics_info::add_concave(const std::vector < augmentations::vec2 < >> &verts) {
		b2Separator separator;
		std::vector<std::vector<b2Vec2>> output;
		auto& input = reinterpret_cast<const std::vector<b2Vec2>&>(verts);
		int res = separator.Validate(input);
		
		if (res != 0) {
			auto reversed_input = input;
			std::reverse(reversed_input.begin(), reversed_input.end());
			separator.calcShapes(reversed_input, output);
		}
		else separator.calcShapes(input, output);
		
		for (auto& convex : output)
			add_convex(std::vector<augmentations::vec2<>>(convex.begin(), convex.end()));
	}

	void create_physics_component(const physics_info& body_data, augmentations::entity_system::entity& subject, int body_type) {
		physics_system& physics = subject.owner_world.get_system<physics_system>();
		
		b2BodyDef def;
		def.type = b2BodyType(body_type);
		def.angle = 0;
		def.userData = (void*) &subject;

		b2PolygonShape shape;

		b2FixtureDef fixdef;
		fixdef.density = body_data.density;
		fixdef.friction = body_data.friction;
		fixdef.isSensor = body_data.sensor;
		fixdef.filter = body_data.filter;
		fixdef.shape = &shape;

		b2Body* body = physics.b2world.CreateBody(&def);

		if (body_data.type == physics_info::RECT) {
			shape.SetAsBox(static_cast<float>(body_data.rect_size.x) / 2.f * PIXELS_TO_METERSf, static_cast<float>(body_data.rect_size.y) / 2.f * PIXELS_TO_METERSf);
			body->CreateFixture(&fixdef);
		}
		else {
			for (auto convex : body_data.convex_polys) {
				std::vector<b2Vec2> b2verts(convex.begin(), convex.end());
				
				for (auto& v : b2verts)
					v *= PIXELS_TO_METERSf;

				shape.Set(b2verts.data(), b2verts.size());
				body->CreateFixture(&fixdef);
			}
		}

		//b2Vec2 v[4] = {
		//	vec2<>(0.f, 0.f),
		//	vec2<>(size.w*PIXELS_TO_METERS, 0.f),
		//	vec2<>(size.w*PIXELS_TO_METERS, size.h*PIXELS_TO_METERS),
		//	vec2<>(0.f, size.h*PIXELS_TO_METERS)
		//};

		//shape.Set(v, 4);

		auto& transform = subject.get<components::transform>().current;

		body->SetTransform(transform.pos*PIXELS_TO_METERSf, transform.rotation*0.01745329251994329576923690768489f);

		body->SetAngularDamping(body_data.angular_damping);
		body->SetLinearDamping(body_data.linear_damping);
		body->SetFixedRotation(body_data.fixed_rotation);

		auto physics_component = components::physics(body);
		subject.add(physics_component);
	}

	std::vector<b2Vec2> get_transformed_shape_verts(augmentations::entity_system::entity& subject) {
		std::vector<b2Vec2> output;

		auto b = subject.get<components::physics>().body;
		auto verts = reinterpret_cast<entity*>(b->GetUserData())->get<components::render>().model->get_vertices();
		/* for every vertex in given fixture's shape */
		for (auto& v : verts) {
			v *= PIXELS_TO_METERSf;

			auto position = b->GetPosition();
			/* transform angle to degrees */
			auto rotation = b->GetAngle() / 0.01745329251994329576923690768489f;

			/* transform vertex to current entity's position and rotation */
			output.push_back(v.rotate(rotation, b2Vec2(0, 0)) + position);
		}

		return output;
	}
}