#include "augs/graphics/renderer.h"
#include "augs/templates/thread_templates.h"
#include "view/viewables/streaming/viewables_streaming.h"
#include "view/audiovisual_state/systems/sound_system.h"

viewables_streaming::viewables_streaming(augs::renderer& renderer) {
	LOG_NVPS(std::thread::hardware_concurrency());

	{
		auto scope = measure_scope(performance.pbo_allocation);
		uploading_pbo.reserve_for_texture_square(renderer.get_max_texture_size() / 4);
	}

	uploading_pbo.set_current_to_none();
}

void viewables_streaming::load_all(const viewables_load_input in) {
	const auto& new_all_defs = in.new_defs;
	auto& now_all_defs = now_loaded_viewables_defs;

	const auto& gui_font = in.gui_font;
	const auto& necessary_image_definitions = in.necessary_image_definitions;
	const auto settings = in.settings;
	const auto& unofficial_content_dir = in.unofficial_content_dir;
	const auto max_atlas_size = in.max_atlas_size;

	/* Atlas pass */

	if (!future_general_atlas.valid()) {
		bool new_atlas_required = settings.regenerate_every_time;

		auto& now_defs = now_all_defs.image_definitions;
		auto& new_defs = new_all_defs.image_definitions;

		{
			{
				/* Check for unloaded and changed resources */
				for_each_id_and_object(now_defs, [&](const auto key, const auto& old_def) {
					if (const auto new_def = mapped_or_nullptr(new_defs, key)) {
						if (new_def->loadables != old_def.loadables) {
							/* Changed, so reload. */
							new_atlas_required = true;
						}
					}
					else {
						/* Missing, unload */
						new_atlas_required = true;
					}
				});

				for_each_id_and_object(new_defs, [&](const auto key, const auto&) {
					if (nullptr == mapped_or_nullptr(now_defs, key)) {
						/* New one, include. */
						new_atlas_required = true;
					}
				});
			}

			if (gui_font != now_loaded_gui_font_def) {
				new_atlas_required = true;
			}
		}

		if (necessary_images_in_atlas.empty()) {
			new_atlas_required = true;
		}

		if (!general_atlas.has_value()) {
			new_atlas_required = true;
		}

		if (new_atlas_required) {
			auto scope = measure_scope(performance.launching_atlas_reload);

			const auto pbo_buffer = [this]() -> rgba* {
				if (pbo_ready_to_use) {
					auto scope = measure_scope(performance.pbo_map_buffer);
					uploading_pbo.set_as_current();
					return reinterpret_cast<rgba*>(uploading_pbo.map_buffer());
				}

				return nullptr;
			}();

			if (pbo_buffer == nullptr) {
				/* Prepare fallback */
				pbo_fallback.clear();
			}

			auto general_atlas_in = general_atlas_input {
				{
					settings,
					necessary_image_definitions,
					new_defs,
					gui_font,
					unofficial_content_dir
				},

				max_atlas_size,

				pbo_buffer,
				pbo_fallback
			};

			future_general_atlas = std::async(
				std::launch::async,
				[general_atlas_in, pbo_buffer, this]() { 
					if (pbo_buffer != nullptr) {
						/* Asynchronously clear the pbo_fallback if we will no longer use it */
						pbo_fallback.clear();
						pbo_fallback.shrink_to_fit();
					}

					return create_general_atlas(general_atlas_in, general_atlas_performance, performance.neon_maps_regeneration);
				}
			);

			future_image_definitions = new_defs;
			future_gui_font = gui_font;

			if (pbo_buffer != nullptr) {
				uploading_pbo.set_current_to_none(); 
			}
		}
	}

	/* Sounds pass */

	if (!future_loaded_buffers.valid()) {
		auto total = measure_scope(performance.launching_sounds_reload);

		auto make_sound_loading_input = [&](const sound_definition& def) {
			const auto def_view = sound_definition_view(unofficial_content_dir, def);
			const auto input = def_view.make_sound_loading_input();

			return input;
		};

		auto& now_defs = now_all_defs.sounds;
		auto& new_defs = new_all_defs.sounds;

		/* Request to, on finalization, unload no longer existent sounds. */
		for_each_id_and_object(now_defs, [&](const auto& key, const auto&) {
			if (nullptr == mapped_or_nullptr(new_defs, key)) {
				sound_requests.emplace_back(key, augs::sound_buffer_loading_input{ {}, {} });
			}
		});

		/* Gather loading requests for new and changed definitions. */
		for_each_id_and_object(new_defs, [&](const auto& fresh_key, const auto& new_def) {
			auto request_new = [&]() {
				sound_requests.emplace_back(fresh_key, make_sound_loading_input(new_def));
			};

			if (const auto now_def = mapped_or_nullptr(now_defs, fresh_key)) {
				if (new_def.loadables != now_def->loadables) {
					/* Found, but a different one. Reload. */
					request_new();
				}
			}
			else {
				/* Not found, load it then. */
				request_new();
			}
		});

		if (sound_requests.size() > 0) {
			future_loaded_buffers = std::async(std::launch::async,
				[&](){
					using value_type = decltype(future_loaded_buffers.get());

					auto scope = measure_scope(performance.reloading_sounds);

					value_type result;

					for (const auto& r : sound_requests) {
						if (r.second.source_sound.empty()) {
							/* A request to unload. */
							result.push_back(std::nullopt);
							continue;
						}

						try {
							augs::sound_buffer b = r.second;
							result.emplace_back(std::move(b));
						}
						catch (...) {
							result.push_back(std::nullopt);
						}
					}

					return result;
				}
			);

			future_sound_definitions = new_all_defs.sounds;
		}
	}
}

void viewables_streaming::finalize_load(viewables_finalize_input in) {
	auto& now_all_defs = now_loaded_viewables_defs;

	/* Unpack asynchronous asset loading results */

	if (valid_and_is_ready(future_general_atlas)) {
		const bool measure_atlas_uploading = in.measure_atlas_upload;
		const bool was_written_to_pbo = pbo_ready_to_use;

		if (measure_atlas_uploading) {
			in.renderer.finish();
		}

		auto scope = measure_scope(performance.atlas_upload_to_gpu);

		if (was_written_to_pbo) {
			auto scope = measure_scope(performance.pbo_unmap_buffer);

			uploading_pbo.set_as_current();
			uploading_pbo.unmap_buffer();
		}

		auto result = future_general_atlas.get();

		images_in_atlas = std::move(result.atlas_entries);
		necessary_images_in_atlas = std::move(result.necessary_atlas_entries);
		get_loaded_gui_font() = std::move(result.gui_font);

		now_loaded_gui_font_def = future_gui_font;

		auto& now_loaded_defs = now_all_defs.image_definitions;
		auto& new_loaded_defs = future_image_definitions;

		/* Done, overwrite */
		now_loaded_defs = new_loaded_defs;

		const auto atlas_size = result.atlas_size;

		if (was_written_to_pbo) {
			general_atlas.emplace(atlas_size);
			general_atlas->start_upload_from(uploading_pbo);

			augs::graphics::pbo::set_current_to_none();
		}
		else {
			general_atlas.emplace(atlas_size, pbo_fallback.data());
		}

		if (measure_atlas_uploading) {
			in.renderer.finish();
		}

		pbo_ready_to_use = true;
	}

	if (valid_and_is_ready(future_loaded_buffers)) {
		auto& now_loaded_defs = now_all_defs.sounds;
		auto& new_loaded_defs = future_sound_definitions;

		auto unload = [&](const assets::sound_id key){
			in.sounds.clear_sources_playing(key);
			loaded_sounds.erase(key);
		};

		/* Reload the sounds that at the time of launch were found to be new or changed. */

		{
			auto loaded = future_loaded_buffers.get();

			for (const auto& r : sound_requests) {
				const auto id = r.first;
				unload(id);

				const auto i = index_in(sound_requests, r);

				if (auto& loaded_sound = loaded[i]) {
					/* Loading was successful. */
					loaded_sounds.try_emplace(id, std::move(loaded_sound.value()));
				}
			}
		}

		/* Done, overwrite */
		now_loaded_defs = new_loaded_defs;
		sound_requests.clear();
	}
}
