#include "view/viewables/atlas_distributions.h"
#include "view/viewables/game_image.h"

#include "view/viewables/regeneration/game_image_loadables.h"

augs::graphics::texture standard_atlas_distribution(const standard_atlas_distribution_input in) {
	thread_local auto atlas_input = atlas_regeneration_input();

	atlas_input.clear();

	for (const auto& r : in.necessary_image_loadables) {
		atlas_input.images.emplace_back(r.second.get_source_image_path());
	}

	for (const auto& r : in.game_image_loadables) {
		const auto& def = r.second;
		def.regenerate_all_needed(in.settings.force_regenerate);

		atlas_input.images.emplace_back(def.get_source_image_path());

		auto add_if_exists = [&](const auto path) {
			if (path.has_value()) {
				atlas_input.images.emplace_back(path.value());
			}
		};

		add_if_exists(def.get_neon_map_path());
		add_if_exists(def.get_desaturation_path());
	}

	atlas_input.fonts.emplace_back(in.gui_font_input);

	thread_local auto atlas_image = augs::image();

	const auto atlas = regenerated_atlas(
		atlas_input,
		in.settings,
		atlas_image
	);

	in.output_gui_font.unpack(atlas.stored_baked_fonts.at(in.gui_font_input));

	{
		const auto& baked = atlas.baked_images;

		for (const auto& r : in.necessary_image_loadables) {
			in.output_necessary_images[r.first] = baked.at(r.second.get_source_image_path());
		}

		for (const auto& d : in.game_image_loadables) {
			auto& output_viewable = in.output_game_images[d.first];
			auto& maps = output_viewable.texture_maps;
			
			const auto& def = d.second;

			maps[texture_map_type::DIFFUSE] = baked.at(def.get_source_image_path());

			auto set_if_exists = [&](const texture_map_type t, const auto path) {
				if (path.has_value()) {
					maps[t] = baked.at(path.value());
				}
			};

			set_if_exists(texture_map_type::NEON, def.get_neon_map_path());
			set_if_exists(texture_map_type::DESATURATED, def.get_desaturation_path());
		}
	}

	return atlas_image;
}