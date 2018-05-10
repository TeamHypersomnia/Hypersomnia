#include "view/viewables/atlas_distributions.h"
#include "view/viewables/image_in_atlas.h"

#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/image_definition.h"

augs::graphics::texture standard_atlas_distribution(const standard_atlas_distribution_input in) {
	auto total = measure_scope(in.profiler.whole_regeneration);

	thread_local auto atlas_subjects = atlas_regeneration_subjects();

	auto make_view = [&in](const auto& def) {
		return image_definition_view(in.unofficial_project_dir, def);
	};

	{
		auto scope = measure_scope(in.profiler.gathering_subjects);

		atlas_subjects.clear();

		for (const auto& r : in.necessary_image_definitions) {
			atlas_subjects.images.emplace_back(r.second.get_source_path().path);
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

			atlas_subjects.images.emplace_back(def.get_source_image_path());

			auto add = [&](const auto path) {
				atlas_subjects.images.emplace_back(path);
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

		atlas_subjects.fonts.emplace_back(in.gui_font_input);
	}

	thread_local auto atlas_image = augs::image();

	{
		const auto atlas = baked_atlas({
			atlas_subjects,
			in.settings,
			atlas_image,
			in.profiler
		});

		auto scope = measure_scope(in.profiler.unpacking_results);

		in.output_gui_font.unpack_from(atlas.stored_baked_fonts.at(in.gui_font_input));

		{
			const auto& baked = atlas.baked_images;

			for (const auto& r : in.necessary_image_definitions) {
				in.output_necessary_atlas_entries[r.first] = baked.at(r.second.get_source_path().path);
			}

			for_each_id_and_object(in.image_definitions, [&](const auto id, const auto& d) {
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
	}

	auto scope = measure_scope(in.atlas_upload_to_gpu);

	auto output = augs::graphics::texture(atlas_image);
	return output;
}