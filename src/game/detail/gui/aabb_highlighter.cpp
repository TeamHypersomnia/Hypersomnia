#include "aabb_highlighter.h"
#include "game/components/sprite_component.h"
#include "game/components/crosshair_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/view/viewing_session.h"
#include "game/transcendental/cosmos.h"
#include "game/systems_audiovisual/interpolation_system.h"

#include "game/components/all_inferred_state_component.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "generated_introspectors.h"
#include "augs/graphics/drawers.h"

void aabb_highlighter::update(const float delta_ms) {
	timer += delta_ms;
	timer = fmod(timer, cycle_duration_ms);
}

void aabb_highlighter::draw(
	augs::vertex_triangle_buffer& output,
	const const_entity_handle subject,
	const interpolation_system& interp,
	const camera_cone camera
) const {
	ltrb aabb;

	const auto aabb_expansion_lambda = [&aabb, &interp](const const_entity_handle e) {
		if (!e.is_inferred_state_activated()) {
			return false;
		}

		if (e.has<components::particles_existence>() || e.has<components::crosshair>()) {
			return false;
		}

		const auto new_aabb = e.get_aabb(get_assets_manager(), interp);

		if (aabb.good() && new_aabb.good()) {
			aabb.contain(new_aabb);
		}
		else if (new_aabb.good()) {
			aabb = new_aabb;
		}
		
		return true;
	};

	aabb_expansion_lambda(subject);
	subject.for_each_child_entity_recursive(aabb_expansion_lambda);

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

	// const auto& manager = get_assets_manager();

	if (aabb.good()) {
		components::sprite::drawing_input state(output);
		state.camera = camera;
		state.renderable_transform.rotation = 0;

		components::sprite border;
		border.tex = assets::game_image_id::BLANK;
		border.color = cyan;

		auto& pos = state.renderable_transform.pos;

		pos = ap;
		border.overridden_size.set(current_length, 1);
		border.draw_from_lt(state);
		border.overridden_size.set(1, current_length);
		border.draw_from_lt(state);

		pos = ap + vec2i(as.x - current_length, 0);
		border.overridden_size.set(current_length, 1);
		border.draw_from_lt(state);
		pos = ap + vec2i(as.x - 1, 0);
		border.overridden_size.set(1, current_length);
		border.draw_from_lt(state);

		pos = ap + vec2i(0, as.y - current_length);
		border.overridden_size.set(1, current_length);
		border.draw_from_lt(state);
		pos = ap + vec2i(0, as.y - 1);
		border.overridden_size.set(current_length, 1);
		border.draw_from_lt(state);

		pos = ap + vec2i(as.x - current_length, as.y - 1);
		border.overridden_size.set(current_length, 1);
		border.draw_from_lt(state);
		pos = ap + vec2i(as.x - 1, as.y - current_length);
		border.overridden_size.set(1, current_length);
		border.draw_from_lt(state);
	}
}
