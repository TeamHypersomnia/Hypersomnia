#include <unordered_set>

#include "view/viewables/atlas_distributions.h"
#include "view/viewables/image_in_atlas.h"

#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/image_definition.h"
#include "augs/templates/thread_pool.h"
#include "augs/templates/introspect.h"
#include "view/viewables/regeneration/atlas_progress_structs.h"
#include "augs/log.h"

#include "augs/filesystem/file.h"
#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/to_bytes.h"
#include "augs/filesystem/directory.h"

static bool should_regenerate(
	const augs::path_type& source,
	const augs::path_type& stamp
) {
	try {
		const auto new_stamp_bytes = augs::to_bytes(augs::last_write_time(source));
		const auto existent_stamp_bytes = augs::file_to_bytes(stamp);
		const bool are_stamps_identical = (new_stamp_bytes == existent_stamp_bytes);

		if (!are_stamps_identical) {
			return true;
		}
	}
	catch (...) {
		return true;
	}

	return false;
}

void regenerate_and_gather_subjects(
	const subjects_gathering_input in,
	atlas_input_subjects& output
) {
	const auto num_workers = std::size_t(in.settings.neon_regeneration_threads - 1);

	static augs::thread_pool workers = 0;
	workers.resize(num_workers);

	auto make_view = [&in](const auto& def) {
		return image_definition_view(in.unofficial_project_dir, def);
	};

	/* Unpack GIFs into PNGs */

	//make_view

	std::unordered_set<augs::path_type> gifs_to_regenerate;

	for (const auto& d : in.image_definitions) {
		const auto def = make_view(d);
		auto path = def.get_source_image_path();

		if (path.extension() == ".png") {
			if (path.replace_extension("").replace_extension("").extension() == ".gif") {
				auto stamp_path = path;
				stamp_path += ".stamp";

				auto cache_preffix = std::string(GENERATED_FILES_DIR);
				auto original_gif_path = path.string();

				/* Remove first folder */

				if (begins_with(original_gif_path, cache_preffix)) {
					cut_preffix(original_gif_path, cache_preffix);

					if (original_gif_path.size() > 0) {
						/* Remove separator */
						original_gif_path.erase(original_gif_path.begin());

						if (!found_in(gifs_to_regenerate, original_gif_path)) {
							if (should_regenerate(original_gif_path, stamp_path)) {
								gifs_to_regenerate.emplace(original_gif_path);
							}
						}
					}
				}
			}
		}
	}

	if (!gifs_to_regenerate.empty()) {
		for (const auto& gif : gifs_to_regenerate) {
			auto new_job = [&gif]() {
				const auto frames = augs::image::gif_to_frames(gif);

				LOG("Regenerate gif: %x (%x frames)", gif, frames.size());

				for (std::size_t i = 0; i < frames.size(); ++i) {
					const auto& frame = frames[i];

					LOG("Reading frame %x, %x bytes", i, frame.serialized_frame.size());

					augs::image img; 
					img.from_bytes(frame.serialized_frame, "dummy.bin");

					auto generated_png_path = augs::path_type(GENERATED_FILES_DIR) / gif;
					generated_png_path += typesafe_sprintf(".%x.png", i);
					LOG("Generated_png_path: %x. Creating directories.", generated_png_path);

					augs::create_directories_for(generated_png_path);
					LOG("Saving as png: %x", generated_png_path);
					img.save_as_png(generated_png_path);
				};

				auto stamp_path = augs::path_type(GENERATED_FILES_DIR) / gif;
				stamp_path += ".stamp";

				augs::save_as_bytes(augs::last_write_time(gif), stamp_path);
			};

			workers.enqueue(new_job);
		}

		workers.submit();
		workers.help_until_no_tasks();
		workers.wait_for_all_tasks_to_complete();
	}

	{
		output.clear();

		for (auto&& r : in.necessary_image_definitions) {
			output.images.emplace_back(r.second.get_source_path().path);
		}

		int total_to_regenerate = 0;

		std::vector<std::optional<cached_neon_map_in>> neon_regen_inputs;

		for (const auto& d : in.image_definitions) {
			const bool force = in.settings.regenerate_every_time;

			const auto def = make_view(d);

			auto result = def.should_regenerate_neon_map(force);

			if (result.has_value()) {
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
		for (auto&& r : subjects.necessary_image_definitions) {
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
	thread_local baked_atlas baked;

	atlas_subjects.clear();
	baked.clear();

	std::unordered_map<augs::path_type, ad_hoc_entry_id> input_path_to_id;
	std::unordered_map<augs::path_type, augs::image::gif_data> gif_datas;
	std::unordered_map<augs::path_type, augs::image::gif_data> png_seq_datas;
	std::unordered_map<augs::path_type, std::size_t> starting_loading_image;

	for (const auto& entry : in.subjects) {
		if (entry.image_path.extension() == ".gif" || ends_with(entry.image_path.string(), "_*.png")) {
			continue;
		}

		try {
			atlas_subjects.images.push_back(entry.image_path);
			input_path_to_id[entry.image_path] = entry.id;
		}
		catch (...) {

		}
	}

	for (const auto& entry : in.subjects) {
		if (!ends_with(entry.image_path.string(), "_*.png")) {
			continue;
		}

		auto frames = augs::image::gif_data();
		auto basic_path = entry.image_path.string();
		basic_path.erase(basic_path.end() - std::strlen("_*.png"), basic_path.end());

		try {
			for (std::size_t i = 1; ; ++i) {
				auto next_path = basic_path;
				next_path += typesafe_sprintf("_%x.png", i);

				if (augs::exists(next_path)) {
					atlas_subjects.images.push_back(next_path);

					/*
						The fact that it won't be accurate is not a problem.
						PNG sequences will only be used for official resources.
					*/

					constexpr int default_duration_milliseconds = 40;
					frames.push_back({ {}, default_duration_milliseconds });
				}
				else {
					break;
				}
			}

			input_path_to_id[entry.image_path] = entry.id;
			png_seq_datas[entry.image_path] = std::move(frames);
		}
		catch (...) {

		}
	}

	for (const auto& entry : in.subjects) {
		if (entry.image_path.extension() != ".gif") {
			continue;
		}

		try {
			auto frames = augs::image::gif_to_frames(entry.image_path);
			starting_loading_image[entry.image_path] = atlas_subjects.loaded_images.size();

			for (std::size_t i = 0; i < frames.size(); ++i) {
				auto& frame = frames[i];

				atlas_subjects.loaded_images.emplace_back(std::move(frame.serialized_frame));
			}

			input_path_to_id[entry.image_path] = entry.id;
			gif_datas[entry.image_path] = std::move(frames);
		}
		catch (...) {

		}
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

	for (auto& it : input_path_to_id) {
		const auto& baked_path = it.first;
		const auto entry_id = it.second;

		if (const auto gif_data = mapped_or_nullptr(gif_datas, baked_path)) {
			const auto start_i = starting_loading_image[baked_path];

			for (std::size_t i = 0; i < gif_data->size(); ++i) {
				const auto baked_image = baked.loaded_images[start_i + i];
				const auto ms = gif_data->at(i).duration_milliseconds;

				out.atlas_entries.add_frame(entry_id, baked_image, ms);
			}

			continue;
		}

		if (const auto png_seq_data = mapped_or_nullptr(png_seq_datas, baked_path)) {
			auto basic_path = baked_path.string();
			basic_path.erase(basic_path.end() - std::strlen("_*.png"), basic_path.end());
			basic_path += "_%x.png";

			for (std::size_t i = 0; i < png_seq_data->size(); ++i) {
				const auto frame_path = typesafe_sprintf(basic_path, i + 1);
				const auto baked_image = baked.images[frame_path];
				const auto ms = png_seq_data->at(i).duration_milliseconds;

				out.atlas_entries.add_frame(entry_id, baked_image, ms);
			}

			continue;
		}

		const auto& baked_image = baked.images.at(baked_path);

		out.atlas_entries.add_frame(entry_id, baked_image, 0.0f);
	}

	return out;
}
