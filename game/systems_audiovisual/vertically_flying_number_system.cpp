#include "vertically_flying_number_system.h"
#include "augs/templates/container_templates.h"

void vertically_flying_number_system::add(const number::input new_in) {
	number new_number;
	new_number.in = new_in;
	new_number.time_of_occurence_seconds = global_time_seconds;

	numbers.push_back(new_number);
}

void vertically_flying_number_system::advance(const augs::delta dt) {
	global_time_seconds += dt.in_seconds();

	erase_remove(
		numbers,
		[this](const number& n) {
			return (global_time_seconds - n.time_of_occurence_seconds) > n.in.maximum_duration_seconds;
		}
	);
}

void vertically_flying_number_system::draw_numbers(
	augs::vertex_triangle_buffer& triangles,
	const camera_cone camera
) const {
	for (const auto& r : numbers) {
		const auto passed = global_time_seconds - r.time_of_occurence_seconds;
		const auto ratio = passed / r.in.maximum_duration_seconds;

		auto text = r.in.text;

		text.pos = camera[r.in.pos - vec2(0, static_cast<float>(sqrt(passed)) * 120.f)];

		text.draw_stroke(triangles);
		text.draw(triangles);
	}
}