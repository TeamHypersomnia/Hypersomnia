#include "aabb_highlighter.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/physics_definition_component.h"
#include "entity_system/entity.h"

void aabb_highlighter::update(float delta) {
	timer += delta;
	timer = fmod(timer, cycle_duration_ms);
}

void aabb_highlighter::draw(shared::state_for_drawing_camera camera, augs::entity_id subject) {
	vec2i as;
	vec2i ap;

	float gap_x = base_gap;
	float gap_y = base_gap;

	float length_decrease = 0.f;

	rects::ltrb<float> aabb;

	subject->for_each_sub_entity([&aabb](augs::entity_id e) {
		auto* sprite = e->find<components::sprite>();
		auto* physics = e->find<components::physics_definition>();

		if (e->get_name_as_sub_entity() == sub_entity_name::CHARACTER_CROSSHAIR
			|| e->get_name_as_sub_entity() == sub_entity_name::CROSSHAIR_RECOIL_BODY
			|| e->is_definition_entity())
			return;

		if (sprite) {
			auto new_aabb = sprite->get_aabb(e->get<components::transform>());

			if (aabb.good())
				aabb.contain(new_aabb);
			else
				aabb = new_aabb;
		}
	});

	auto lesser_dimension = std::min(aabb.w(), aabb.h());
	
	if (lesser_dimension < scale_down_when_aabb_no_bigger_than)
		length_decrease = std::min(smallest_length - 4, scale_down_when_aabb_no_bigger_than - lesser_dimension);
	
	auto adjusted_biggest_length = biggest_length - length_decrease;
	auto adjusted_smallest = smallest_length - length_decrease;

	int current_length = augs::interp(adjusted_biggest_length, adjusted_smallest, timer / cycle_duration_ms);
	int gap_animated_expansion = current_length - adjusted_smallest;
	gap_x += gap_animated_expansion+length_decrease;
	gap_y += gap_animated_expansion+length_decrease;
	
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
