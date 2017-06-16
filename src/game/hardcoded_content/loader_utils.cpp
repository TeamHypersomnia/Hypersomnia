#include "augs/gui/button_corners.h"
#include "loader_utils.h"

void make_button_with_corners(
	game_image_requests& into,
	const game_image_id first,
	const std::string& filename_template,
	const bool request_lb_complement
) {
	const auto first_i = static_cast<int>(first);
	const auto last_i = first_i + static_cast<int>(button_corner_type::COUNT);

	for (int i = first_i; i < last_i; ++i) {
		const auto type = static_cast<button_corner_type>(i - first_i);

		if (!request_lb_complement && is_lb_complement(type)) {
			continue;
		}

		const auto full_filename = typesafe_sprintf(filename_template, get_filename_for(type));

		{
			auto& in = into[static_cast<game_image_id>(i)];
			in.texture_maps[DIFFUSE] = { full_filename, GAME_WORLD_ATLAS };
		}
	}
}

void make_indexed_images(
	game_image_requests& into,
	const game_image_id first,
	const game_image_id last,
	const std::string& filename_template,
	const std::string& neon_filename_template
) {
	const auto first_i = static_cast<int>(first);
	const auto last_i = static_cast<int>(last);

	for (int i = first_i; i < last_i; ++i) {
		auto& in = into[static_cast<game_image_id>(i)];
		in.texture_maps[DIFFUSE] = { typesafe_sprintf(filename_template, 1 + i - first_i), GAME_WORLD_ATLAS };

		if (neon_filename_template.size() > 0) {
			in.texture_maps[NEON] = { typesafe_sprintf(neon_filename_template, 1 + i - first_i), GAME_WORLD_ATLAS };
		}
	}
}