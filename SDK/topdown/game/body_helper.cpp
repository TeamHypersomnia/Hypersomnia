#include "stdafx.h"
#include "body_helper.h"
#include "../components/physics_component.h"
#include "../components/render_component.h"
#include "../resources/render_info.h"

#include "entity_system/entity_system.h"

namespace topdown {
	physics_info::physics_info() 
		: rect_size(augmentations::vec2<>()), type(RECT), density(1.f), angular_damping(0.f), linear_damping(0.f), fixed_rotation(false) {
	}

	b2World* current_b2world = nullptr;
	
	void create_physics_component(const physics_info& body_data, augmentations::entity_system::entity& subject, int body_type) {
		b2BodyDef def;
		def.type = b2BodyType(body_type);
		def.angle = 0;
		def.userData = (void*) &subject;

		b2PolygonShape shape;
		if (body_data.type == physics_info::RECT) {
			shape.SetAsBox(static_cast<float>(body_data.rect_size.x) / 2.f * PIXELS_TO_METERSf, static_cast<float>(body_data.rect_size.y) / 2.f * PIXELS_TO_METERSf);
		}
		else {
			assert("not implemented");
		}

		//b2Vec2 v[4] = {
		//	vec2<>(0.f, 0.f),
		//	vec2<>(size.w*PIXELS_TO_METERS, 0.f),
		//	vec2<>(size.w*PIXELS_TO_METERS, size.h*PIXELS_TO_METERS),
		//	vec2<>(0.f, size.h*PIXELS_TO_METERS)
		//};

		//shape.Set(v, 4);

		b2FixtureDef fixdef;
		fixdef.shape = &shape;
		fixdef.density = body_data.density;
		fixdef.friction = 1.0;

		b2Body* body = current_b2world->CreateBody(&def);
		body->CreateFixture(&fixdef);
		auto& transform = subject.get<components::transform>();

		body->SetTransform(transform.current.pos*PIXELS_TO_METERSf, transform.current.rotation*0.01745329251994329576923690768489);
		body->GetFixtureList()->SetFilterData(body_data.filter);

		body->SetAngularDamping(body_data.angular_damping);
		body->SetLinearDamping(body_data.linear_damping);
		body->SetFixedRotation(body_data.fixed_rotation);

		auto physics = components::physics(body);
		subject.add(physics);
	}
}