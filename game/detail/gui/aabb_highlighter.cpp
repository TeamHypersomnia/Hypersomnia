#include "aabb_highlighter.h"
#include "game/components/sprite_component.h"
#include "game/components/sub_entities_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "augs/graphics/renderer.h"

void aabb_highlighter::update(const float delta_ms) {
	timer += delta_ms;
	timer = fmod(timer, cycle_duration_ms);
}

void aabb_highlighter::draw(viewing_step& step, const_entity_handle subject) const {
	rects::ltrb<float> aabb;

	subject.for_each_sub_entity_recursive([&aabb](const_entity_handle e) {
		auto* sprite = e.find<components::sprite>();

		if (e.get_name_as_sub_entity() == sub_entity_name::CHARACTER_CROSSHAIR
			|| e.get_name_as_sub_entity() == sub_entity_name::CROSSHAIR_RECOIL_BODY)
			return;

		if (sprite) {
			auto new_aabb = sprite->get_aabb(e.get<components::transform>());

			if (aabb.good())
				aabb.contain(new_aabb);
			else
				aabb = new_aabb;
		}
	});

	const auto lesser_dimension = std::min(aabb.w(), aabb.h());
	
	float length_decrease = 0.f;

	if (lesser_dimension < scale_down_when_aabb_no_bigger_than)
		length_decrease = std::min(smallest_length - 4, scale_down_when_aabb_no_bigger_than - lesser_dimension);
	
	const auto adjusted_biggest_length = biggest_length - length_decrease;
	const auto adjusted_smallest = smallest_length - length_decrease;

	const int current_length{ static_cast<int>(augs::interp(adjusted_biggest_length, adjusted_smallest, timer / cycle_duration_ms)) };
	const int gap_animated_expansion{ static_cast<int>(current_length - adjusted_smallest) };

	const float gap_x = base_gap + gap_animated_expansion+length_decrease;
	const float gap_y = base_gap + gap_animated_expansion+length_decrease;
	
	aabb.l -= gap_x;
	aabb.r += gap_x;
	aabb.t -= gap_y;
	aabb.b += gap_y;

	vec2i as = aabb.get_size();
	vec2i ap = aabb.get_position();

	if (aabb.good()) {
		components::sprite::drawing_input state(step.renderer.triangles);
		state.setup_from(step.camera_state);
		state.positioning = components::sprite::drawing_input::positioning_type::LEFT_TOP_CORNER;
		state.renderable_transform.rotation = 0;

		components::sprite border;
		border.set(assets::texture_id::BLANK, cyan);

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
