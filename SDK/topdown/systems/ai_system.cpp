#include "stdafx.h"
#include "ai_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "physics_system.h"

struct my_ray_callback : public b2RayCastCallback {
	vec2<> intersection;
	bool hit;

	my_ray_callback() : hit(false) {}

	float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
		const b2Vec2& normal, float32 fraction) override {
			intersection = point;
			
			hit = true;
			return fraction;
	}
};

void ai_system::process_entities(world& owner) {
	physics_system& physics = owner.get_system<physics_system>();

	for (auto it : targets) {
		auto& ai = it->get<components::ai>();
		auto& transform = it->get<components::transform>();

		ai.lines.push_back(components::ai::debug_line(
			transform.current.pos*PIXELS_TO_METERSf,
			transform.current.pos*PIXELS_TO_METERSf + vec2<>(3.0, 0.0)));
		ai.lines.push_back(components::ai::debug_line(vec2<>(0, 0), vec2<>(2, 2)));


	}
}