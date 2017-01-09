#include "aabb_highlighter.h"
#include "game/components/sprite_component.h"
#include "game/components/sub_entities_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/cosmos.h"
#include "augs/graphics/renderer.h"
#include "game/systems_audiovisual/interpolation_system.h"

void aabb_highlighter::update(const float delta_ms) {
	timer += delta_ms;
	timer = fmod(timer, cycle_duration_ms);
}

void aabb_highlighter::draw(viewing_step& step, const const_entity_handle& subject) const {
	ltrb aabb;
	
	auto aabb_expansion_lambda = [&aabb, &step](const const_entity_handle e) {
		static const sub_entity_name dont_expand_aabb_for_sub_entities[] = {
			sub_entity_name::CHARACTER_CROSSHAIR,
			sub_entity_name::CROSSHAIR_RECOIL_BODY,
			sub_entity_name::MUZZLE_SMOKE,
			sub_entity_name::BULLET_SHELL,
			sub_entity_name::BULLET_ROUND,
			sub_entity_name::CORPSE_OF_SENTIENCE,
		};

		const auto name_as_sub_entity = e.get_name_as_sub_entity();

		for (const auto forbidden : dont_expand_aabb_for_sub_entities) {
			if (name_as_sub_entity == forbidden) {
				return;
			}
		}

		const auto new_aabb = e.get_aabb(step.session.systems_audiovisual.get<interpolation_system>());

		if (aabb.good()) {
			aabb.contain(new_aabb);
		}
		else {
			aabb = new_aabb;
		}
	};

	aabb_expansion_lambda(subject);
	subject.for_each_sub_entity_recursive(aabb_expansion_lambda);

	const auto lesser_dimension = std::min(aabb.w(), aabb.h());
	
	float length_decrease = 0.f;

	if (lesser_dimension < scale_down_when_aabb_no_bigger_than) {
		length_decrease = std::min(smallest_length - 4, scale_down_when_aabb_no_bigger_than - lesser_dimension);
	}
	
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
		state.camera = step.camera;
		state.positioning = renderable_positioning_type::LEFT_TOP_CORNER;
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
