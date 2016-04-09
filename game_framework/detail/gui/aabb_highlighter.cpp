#include "aabb_highlighter.h"
#include "game_framework/components/sprite_component.h"
#include "entity_system/entity.h"

void aabb_highlighter::update(float delta) {
	timer += delta;
	timer = fmod(timer, cycle_duration_ms);
}

void aabb_highlighter::draw(shared::state_for_drawing_camera camera, augs::entity_id subject) {
	biggest_length = 16.f;
	smallest_length = 8.f;
	cycle_duration_ms = 400.f;
	scale_down_when_aabb_no_bigger_than = 40.f;

	rects::ltrb<float> aabb;
	vec2i as;
	vec2i ap;

	float gap_x = 2.f;
	float gap_y = 2.f;

	float size_decrease = 0.f;

	auto* sprite = subject->find<components::sprite>();

	if (sprite) {
		aabb = sprite->get_aabb(subject->get<components::transform>());
	}

	auto lesser_dimension = std::min(aabb.w(), aabb.h());
	
	if (lesser_dimension < scale_down_when_aabb_no_bigger_than) {
		size_decrease = std::min(smallest_length - 4, scale_down_when_aabb_no_bigger_than - lesser_dimension);
	}

	auto adjusted_biggest_length = biggest_length - size_decrease;
	auto adjusted_smallest = smallest_length - size_decrease;

	int current_length = augs::interp(adjusted_smallest, adjusted_biggest_length, timer / cycle_duration_ms);
	int gap_animated_expansion = current_length - adjusted_smallest;
	gap_x += gap_animated_expansion+size_decrease;
	gap_y += gap_animated_expansion+size_decrease;
	
	aabb.l -= gap_x;
	aabb.r += gap_x;
	aabb.t -= gap_y;
	aabb.b += gap_y;

	as = aabb.get_size();
	ap = aabb.get_position();

	if (aabb.good()) {
		shared::state_for_drawing_renderable state;
		state.setup_camera_state(camera);
		state.position_is_left_top_corner = true;
		state.renderable_transform.rotation = 0;

		components::sprite border;
		border.set(assets::BLANK, cyan);

		auto& pos = state.renderable_transform.pos;

		pos = ap;
		border.size.set(current_length, 1);
		border.draw(state);
		border.size.set(1, current_length);
		border.draw(state);

		pos = ap + vec2i(as.x - current_length, 0);
		border.size.set(current_length, 1);
		border.draw(state);
		pos = ap + vec2i(as.x - 1, 0);
		border.size.set(1, current_length);
		border.draw(state);

		pos = ap + vec2i(0, as.y - current_length);
		border.size.set(1, current_length);
		border.draw(state);
		pos = ap + vec2i(0, as.y - 1);
		border.size.set(current_length, 1);
		border.draw(state);

		pos = ap + vec2i(as.x - current_length, as.y - 1);
		border.size.set(current_length, 1);
		border.draw(state);
		pos = ap + vec2i(as.x - 1, as.y - current_length);
		border.size.set(1, current_length);
		border.draw(state);
	}
}
