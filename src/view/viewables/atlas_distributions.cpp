#include "view/viewables/atlas_distributions.h"
#include "view/viewables/image_in_atlas.h"

#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/image_definition.h"
#include "augs/templates/range_workers.h"

void regenerate_and_gather_subjects(
	const subjects_gathering_input in,
	atlas_input_subjects& output,
	augs::time_measurements& neon_regeneration_performance
) {
	auto make_view = [&in](const auto& def) {
		return image_definition_view(in.unofficial_project_dir, def);
	};

	{
		output.clear();

		for (const auto& r : in.necessary_image_definitions) {
			output.images.emplace_back(r.second.get_source_path().path);
		}

		auto worker = [make_view, in](const image_definition& d) {
			const auto def = make_view(d);

			const bool force = in.settings.regenerate_every_time;

			def.regenerate_desaturation(force);
			def.regenerate_neon_map(force);
		};

		{
			auto scope = measure_scope(neon_regeneration_performance);

#if 1
			const auto num_workers = std::size_t(in.settings.neon_regeneration_threads);
			static augs::range_workers<decltype(worker)> workers = num_workers;
			workers.resize_workers(num_workers);
			workers.process(worker, in.image_definitions);
#else
			for (const auto& d : in.image_definitions) {
				worker(d);
			}
#endif
		}

		for (const auto& d : in.image_definitions) {
			const auto def = make_view(d);

			output.images.emplace_back(def.get_source_image_path());

			auto add = [&](const auto path) {
				output.images.emplace_back(path);
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

		output.fonts.emplace_back(in.gui_font_input);
	}
}

general_atlas_output create_general_atlas(
	const general_atlas_input in,
	atlas_profiler& performance,
	augs::time_measurements& neon_regeneration_performance
) {
	auto total = measure_scope(performance.whole_regeneration);

	thread_local auto atlas_subjects = atlas_input_subjects();

	{
		auto scope = measure_scope(performance.gathering_subjects);
		regenerate_and_gather_subjects(in.subjects, atlas_subjects, neon_regeneration_performance);
	}

	thread_local baked_atlas baked;
	baked.clear();

	bake_fresh_atlas(
		{
			atlas_subjects,
			in.max_atlas_size,
			in.subjects.settings.atlas_blitting_threads
		},
		{
			in.atlas_image_output,
			in.fallback_output,
			baked,
			performance
		}
	);

	auto scope = measure_scope(performance.unpacking_results);

	auto& subjects = in.subjects;

	general_atlas_output out;

	out.atlas_size = baked.atlas_image_size;
	out.gui_font.unpack_from(baked.fonts.at(subjects.gui_font_input));

	{
		for (const auto& r : subjects.necessary_image_definitions) {
			out.necessary_atlas_entries[r.first] = baked.images.at(r.second.get_source_path().path);
		}

		auto make_view = [&subjects](const auto& def) {
			return image_definition_view(subjects.unofficial_project_dir, def);
		};

		for_each_id_and_object(subjects.image_definitions, [&](const auto id, const auto& d) {
			const auto def = make_view(d);

			auto& output_viewable = out.atlas_entries[id];
			auto& maps = output_viewable;

			maps.diffuse = baked.images.at(def.get_source_image_path());

			auto set_if_baked = [&](auto& m, const auto path) {
				if (auto found = mapped_or_nullptr(baked.images, path)) {
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

	return out;
}