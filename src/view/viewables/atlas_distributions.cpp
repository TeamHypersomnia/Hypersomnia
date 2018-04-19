#include "view/viewables/atlas_distributions.h"
#include "view/viewables/image_structs.h"

#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/regeneration/image_loadables_def.h"

augs::graphics::texture standard_atlas_distribution(const standard_atlas_distribution_input in) {
	thread_local auto atlas_input = atlas_regeneration_input();

	atlas_input.clear();

	for (const auto& r : in.necessary_image_loadables) {
		atlas_input.images.emplace_back(r.second.get_source_image_path());
	}

	for (const auto& def : in.image_loadables) {
		try {
			def.regenerate_all_needed(in.settings.force_regenerate);
		}
		catch (...) {
			try {
				def.delete_regenerated_files();
			}
			catch (...) {

			}
		}

		atlas_input.images.emplace_back(def.get_source_image_path());

		auto add_if_specified = [&](const auto path) {
			if (path.has_value()) {
				atlas_input.images.emplace_back(path.value());
			}
		};

		add_if_specified(def.find_neon_map_path());
		add_if_specified(def.find_desaturation_path());
	}

	atlas_input.fonts.emplace_back(in.gui_font_input);

	thread_local auto atlas_image = augs::image();

	const auto atlas = regenerated_atlas(
		atlas_input,
		in.settings,
		atlas_image
	);

	in.output_gui_font.unpack_from(atlas.stored_baked_fonts.at(in.gui_font_input));

	{
		const auto& baked = atlas.baked_images;

		for (const auto& r : in.necessary_image_loadables) {
			in.output_necessary_atlas_entries[r.first] = baked.at(r.second.get_source_image_path());
		}

		in.image_loadables.for_each_object_and_id([&](const auto& def, const auto id) {
			auto& output_viewable = in.output_atlas_entries[id];
			auto& maps = output_viewable;

			maps.diffuse = baked.at(def.get_source_image_path());

			auto set_if_specified = [&](auto& m, const auto path) {
				if (path.has_value()) {
					m = baked.at(path.value());
				}
			};

			set_if_specified(maps.neon_map, def.find_neon_map_path());
			set_if_specified(maps.desaturated, def.find_desaturation_path());
		});
	}

	return atlas_image;
}