#pragma once
#include "../../detail/state_for_drawing.h"
#include "entity_system/entity_id.h"

struct aabb_highlighter {
	float timer = 0.f;
	float cycle_duration_ms = 400.f;

	float base_gap = 2.f;
	float smallest_length = 8.f;
	float biggest_length = 16.f;
	float scale_down_when_aabb_no_bigger_than = 40.f;

	void update(float delta);
	void draw(shared::state_for_drawing_camera, augs::entity_id subject);
};