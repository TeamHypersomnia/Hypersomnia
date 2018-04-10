#include "augs/templates/container_templates.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/render_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "view/viewables/image_structs.h"

#include "view/rendering_scripts/draw_entity.h"

#include "view/audiovisual_state/systems/pure_color_highlight_system.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void pure_color_highlight_system::clear() {
	highlights.clear();
}

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
	const augs::drawer output,
	const cosmos& cosmos,
	const interpolation_system& interp,
	const images_in_atlas_map& game_images
) const {
	for (const auto& r : highlights) {
		const auto subject = cosmos[r.in.target];

		if (subject.dead()) {
			continue;
		}

		const auto passed = global_time_seconds - r.time_of_occurence_seconds;
		const auto ratio = std::max(0.f, 1.f - static_cast<float>(passed / r.in.maximum_duration_seconds));
		
		const auto target_color = rgba { 
			r.in.color.rgb(),
			static_cast<rgba_channel>(255.f * sqrt(sqrt(ratio)) * r.in.starting_alpha_ratio)
		};

		draw_color_highlight(
			subject,
			target_color,
			{
				output,
				game_images,
				global_time_seconds
			},
			interp
		);
	}
}
