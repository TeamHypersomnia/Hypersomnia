#include "view/viewables/atlas_distributions.h"
#include "view/viewables/image_in_atlas.h"

#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/image_definition.h"
#include "augs/templates/thread_pool.h"
#include "augs/templates/introspect.h"
#include "view/viewables/regeneration/atlas_progress_structs.h"

void regenerate_and_gather_subjects(
	const subjects_gathering_input in,
	atlas_input_subjects& output
) {
	auto make_view = [&in](const auto& def) {
		return image_definition_view(in.unofficial_project_dir, def);
	};

	{
		output.clear();

		for (const auto& r : in.necessary_image_definitions) {
			output.images.emplace_back(r.second.get_source_path().path);
		}

		int total_to_regenerate = 0;

		std::vector<std::optional<cached_neon_map_in>> neon_regen_inputs;

		for (const auto& d : in.image_definitions) {
			const bool force = in.settings.regenerate_every_time;

			const auto def = make_view(d);

			auto result = def.should_regenerate_neon_map(force);

			if (result != std::nullopt) {
				++total_to_regenerate;
			}

			neon_regen_inputs.emplace_back(std::move(result));
		}

		if (in.progress) {
			in.progress->max_neon_maps.store(total_to_regenerate);
		}

		auto worker = [make_view, &in, &neon_regen_inputs](const image_definition& d) {
			const auto this_i = index_in(in.image_definitions.get_objects(), d);

			const auto this_cached_in = neon_regen_inputs[this_i];
			const auto def = make_view(d);

			const bool force = in.settings.regenerate_every_time;
			def.regenerate_desaturation(force);

			if (this_cached_in) {
				if (in.progress) {
					in.progress->current_neon_map_num.fetch_add(1, std::memory_order_relaxed);
				}

				def.regenerate_neon_map(*this_cached_in);
			}
		};

		{
			const auto num_workers = std::size_t(in.settings.neon_regeneration_threads - 1);

			static augs::thread_pool workers = 0;
			workers.resize(num_workers);

			for (const auto& d : in.image_definitions) {
				workers.enqueue([&d, worker]() { worker(d); });
			}

			workers.submit();
			workers.help_until_no_tasks();
			workers.wait_for_all_tasks_to_complete();
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

		augs::introspect([&](auto, auto& in) {
			output.fonts.emplace_back(in);	
		}, in.gui_font_inputs);
	}
}

general_atlas_output create_general_atlas(
	const general_atlas_input in,
	atlas_profiler& performance
) {
	auto total = measure_scope(performance.whole_regeneration);

	thread_local auto atlas_subjects = atlas_input_subjects();

	{
		auto scope = measure_scope(performance.gathering_subjects);
		regenerate_and_gather_subjects(in.subjects, atlas_subjects);
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

	augs::introspect(
		[](auto, auto& output, const auto& input) {
			output.unpack_from(baked.fonts.at(input));
		}, 
		out.gui_fonts, 
		subjects.gui_font_inputs
	);

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

#include "augs/readwrite/byte_file.h"

ad_hoc_atlas_output create_ad_hoc_atlas(ad_hoc_atlas_input in) {
	thread_local atlas_input_subjects atlas_subjects;
	thread_local std::vector<ad_hoc_entry_id> identificators;
	thread_local baked_atlas baked;

	identificators.clear();
	atlas_subjects.clear();
	baked.clear();

	const auto num_subjects = static_cast<int>(in.subjects.size());

	atlas_subjects.loaded_images.resize(num_subjects);
	identificators.resize(num_subjects);

	for (int i = 0; i < num_subjects; ++i) {
		const auto& entry = in.subjects[i];

		augs::file_to_bytes(entry.image_path, atlas_subjects.loaded_images[i]);
		identificators[i] = entry.id;
	}

	atlas_profiler performance;

	bake_fresh_atlas(
		{
			atlas_subjects,
			in.max_atlas_size,
			1
		},
		{
			in.atlas_image_output,
			in.fallback_output,
			baked,
			performance
		}
	);

	ad_hoc_atlas_output out;
	out.atlas_size = baked.atlas_image_size;

	ensure_eq(identificators.size(), baked.loaded_images.size());

	for (int i = 0; i < static_cast<int>(identificators.size()); ++i) {
		const auto& id = identificators[i];
		const auto& baked_image = baked.loaded_images[i];

		out.atlas_entries[id] = baked_image;
	}

	return out;
}