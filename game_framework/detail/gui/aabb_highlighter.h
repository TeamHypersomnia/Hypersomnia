#pragma once
#include "../../detail/state_for_drawing.h"
#include "entity_system/entity_id.h"

struct aabb_highlighter {
	float timer = 0.f;
	float cycle_duration_ms = 400.f;

	float expansion_length = 10.f;
	float smallest_length = 5.f;
	float biggest_length = 20.f;

	void update(float delta);
	void draw(shared::state_for_drawing_camera, augs::entity_id subject);
};