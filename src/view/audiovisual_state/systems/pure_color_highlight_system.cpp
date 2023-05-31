#include "augs/templates/container_templates.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "game/components/render_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "view/viewables/image_in_atlas.h"
#include "view/viewables/images_in_atlas_map.h"

#include "view/rendering_scripts/draw_entity.h"

#include "view/audiovisual_state/systems/pure_color_highlight_system.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void pure_color_highlight_system::clear() {
	highlights.clear();
}

void pure_color_highlight_system::add(const entity_id target, const pure_color_highlight_input new_in) {
	auto new_highlight = highlight();
	new_highlight.in = new_in;
	new_highlight.time_of_occurence_seconds = global_time_seconds;

	highlights[target] = new_highlight;
}

void pure_color_highlight_system::advance(const augs::delta dt) {
	global_time_seconds += dt.in_seconds();
	
	erase_if(
		highlights, 
		[this](const auto& entry) {
			const auto& h = entry.second;

			return (global_time_seconds - h.time_of_occurence_seconds) > h.in.maximum_duration_seconds;
		}
	);
}

void pure_color_highlight_system::draw_highlights(
	const cosmos& cosm,
	const draw_renderable_input& in
) const {
	for (const auto& entry : highlights) {
		const auto subject = cosm[entry.first];
		const auto& r = entry.second;

		if (subject.dead()) {
			continue;
		}

		float teleport_alpha = 1.0f;

		if (auto rigid_body = subject.find<components::rigid_body>()) {
			teleport_alpha = rigid_body.get_teleport_alpha();
		}

		const auto passed = global_time_seconds - r.time_of_occurence_seconds;
		const auto ratio = std::max(0.f, 1.f - static_cast<float>(passed / r.in.maximum_duration_seconds));
		
		auto time_ratio = std::sqrt(std::sqrt(ratio));

		if (!r.in.use_sqrt) {
			time_ratio = ratio * ratio;
		}

		const auto target_color = rgba { 
			r.in.color.rgb(),
			static_cast<rgba_channel>(255.f * time_ratio * r.in.starting_alpha_ratio * teleport_alpha)
		};

		draw_color_highlight(subject, target_color, in);
	}
}
