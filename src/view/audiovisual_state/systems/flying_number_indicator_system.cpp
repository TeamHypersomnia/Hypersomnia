#include "augs/templates/container_templates.h"
#include "augs/drawing/drawing.h"
#include "augs/math/camera_cone.h"

#include "augs/gui/text/printer.h"

#include "view/audiovisual_state/systems/flying_number_indicator_system.h"

void flying_number_indicator_system::clear() {
	numbers.clear();
}

void flying_number_indicator_system::add(const number::input new_in) {
	number new_number;
	new_number.in = new_in;
	new_number.time_of_occurence_seconds = global_time_seconds;

	numbers.push_back(new_number);
}

void flying_number_indicator_system::advance(const augs::delta dt) {
	global_time_seconds += dt.in_seconds();

	erase_if(
		numbers,
		[this](const number& n) {
			const auto passed_time_seconds = global_time_seconds - n.time_of_occurence_seconds;
			return passed_time_seconds > n.in.maximum_duration_seconds;
		}
	);
}

void flying_number_indicator_system::draw_numbers(
	const augs::baked_font& font,
	const augs::drawer output,
	const camera_cone cone
) const {
	for (const auto& r : numbers) {
		const auto passed = global_time_seconds - r.time_of_occurence_seconds;

		if (!r.first_camera_space_pos.has_value()) {
			r.first_camera_space_pos = cone.to_screen_space(r.in.pos);
		}

		const auto text_pos = r.first_camera_space_pos.value() + vec2(r.in.impact_velocity).set_length(static_cast<float>(sqrt(passed)) * 50.f);
		// text_pos = (r.in.pos - vec2(0, static_cast<float>(sqrt(passed)) * 120.f));

		augs::gui::text::print_stroked(
			output,
			cone.to_world_space(text_pos),
			{ r.in.text, { font, r.in.color } },
			{},
			black
		);
	}
}