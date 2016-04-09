#include "aabb_highlighter.h"
#include "game_framework/components/sprite_component.h"
#include "entity_system/entity.h"

void aabb_highlighter::update(float delta) {
	timer += delta;
	timer = fmod(timer, cycle_duration_ms);
}

void aabb_highlighter::draw(shared::state_for_drawing_camera camera, augs::entity_id subject) {
	auto* sprite = subject->find<components::sprite>();

	rects::ltrb<float> aabb;

	if (sprite) {
		aabb = sprite->get_aabb(subject->get<components::transform>());
	}

	if (aabb.good()) {
		shared::state_for_drawing_renderable state;
		state.setup_camera_state(camera);
		state.position_is_left_top_corner = true;
		state.renderable_transform.rotation = 0;

		components::sprite border;
		border.set(assets::BLANK, cyan);

		auto& pos = state.renderable_transform.pos;
		pos = vec2i(aabb.get_position() - vec2i(1, 1));
		border.size.set(biggest_length, 1);
		border.draw(state);
		border.size.set(1, biggest_length);
		border.draw(state);

	}
}
