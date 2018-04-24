#include "view/viewables/atlas_distributions.h"
#include "view/viewables/image_in_atlas.h"

#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/regeneration/image_definition.h"

augs::graphics::texture standard_atlas_distribution(const standard_atlas_distribution_input in) {
	thread_local auto atlas_input = atlas_regeneration_input();

	auto make_view = [&in](const auto& def) {
		return image_definition_view(in.unofficial_project_dir, def);
	};

	atlas_input.clear();

	for (const auto& r : in.necessary_image_loadables) {
		atlas_input.images.emplace_back(r.second.get_source_path().path);
	}

	for (const auto& d : in.image_definitions) {
		const auto def = make_view(d);

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

		auto add = [&](const auto path) {
			atlas_input.images.emplace_back(path);
		};

		auto add_if = [&](const auto path) {
			if (path.has_value()) {
				add(path.value());
				return true;
			}

			return false;
		};

		if (!add_if(def.find_custom_neon_map_path())) {
			add_if(def.find_generated_neon_map_path());
		}

		add_if(def.find_desaturation_path());
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
			in.output_necessary_atlas_entries[r.first] = baked.at(r.second.get_source_path().path);
		}

		in.image_definitions.for_each_object_and_id([&](const auto& d, const auto id) {
			const auto def = make_view(d);

			auto& output_viewable = in.output_atlas_entries[id];
			auto& maps = output_viewable;

			maps.diffuse = baked.at(def.get_source_image_path());

			auto set_if_baked = [&](auto& m, const auto path) {
				if (auto found = mapped_or_nullptr(baked, path)) {
					m = *found;
					return true;
				}

				return false;
			};

			set_if_baked(maps.neon_map, def.calc_generated_neon_map_path());
			set_if_baked(maps.neon_map, def.calc_custom_neon_map_path());

			set_if_baked(maps.desaturated, def.calc_desaturation_path());
		});
	}

	return atlas_image;
}