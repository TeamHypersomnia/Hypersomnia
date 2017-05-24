#include "pure_color_highlight_system.h"
#include "augs/templates/container_templates.h"
#include "game/detail/camera_cone.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/render_component.h"
#include "game/systems_stateless/render_system.h"

void pure_color_highlight_system::add(const highlight::input new_in) {
	bool found = false;

	highlight new_highlight;
	new_highlight.in = new_in;
	new_highlight.time_of_occurence_seconds = global_time_seconds;

	for (auto& r : highlights) {
		if (r.in.target == new_in.target) {
			r = new_highlight;
		}
	}

	if (!found) {
		highlights.push_back(new_highlight);
	}
}

void pure_color_highlight_system::advance(const augs::delta dt) {
	global_time_seconds += dt.in_seconds();
	
	erase_if(
		highlights, 
		[this](const highlight& h) {
			return (global_time_seconds - h.time_of_occurence_seconds) > h.in.maximum_duration_seconds;
		}
	);
}

void pure_color_highlight_system::draw_highlights(
	augs::vertex_triangle_buffer& triangles,
	const camera_cone camera,
	const cosmos& cosmos,
	const interpolation_system& interp
) const {
	for (const auto& r : highlights) {
		const auto subject = cosmos[r.in.target];

		if (subject.dead() || !subject.has<components::sprite>()) {
			continue;
		}

		auto sprite = subject.get<components::sprite>();
		auto& col = sprite.color;
		auto prevcol = col;
		col = r.in.color;

		auto passed = global_time_seconds - r.time_of_occurence_seconds;
		auto ratio = std::max(0.f, 1.f - static_cast<float>(passed / r.in.maximum_duration_seconds));

		col.a = static_cast<rgba_channel>(255.f * sqrt(sqrt(ratio)) * r.in.starting_alpha_ratio);

		render_system().draw_renderable(
			triangles,
			global_time_seconds,
			sprite,
			subject.get_viewing_transform(interp, true),
			subject.get<components::render>(),
			camera,
			renderable_drawing_type::NORMAL
		);

		col = prevcol;
	}

	//step.state.output->triangles.insert(step.state.output->triangles.begin(), pure_color_highlights.begin(), pure_color_highlights.end());
}
