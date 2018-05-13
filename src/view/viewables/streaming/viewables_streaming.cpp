#include "augs/graphics/renderer.h"
#include "augs/templates/thread_templates.h"
#include "view/viewables/streaming/viewables_streaming.h"
#include "view/audiovisual_state/systems/sound_system.h"

viewables_streaming::viewables_streaming(const unsigned max_texture_size) {
	LOG_NVPS(std::thread::hardware_concurrency());

	auto scope = measure_scope(performance.pbo_allocation);
#if USE_PBO
	uploading_pbo.reserve_for_texture_square(max_texture_size);
#else
	no_pbo.resize(max_texture_size * max_texture_size);
#endif
}

void viewables_streaming::load_all(const viewables_load_input in) {
	const auto& new_defs = in.new_defs;
	const auto& gui_font = in.gui_font;
	const auto& necessary_image_definitions = in.necessary_image_definitions;
	const auto settings = in.settings;
	const auto& unofficial_content_dir = in.unofficial_content_dir;
	const auto max_atlas_size = in.max_atlas_size;

	/* Atlas pass */

	{
		bool new_atlas_required = settings.regenerate_every_time;

		{
			{
				/* Check for unloaded and changed resources */
				for_each_id_and_object(now_loaded_viewables_defs.image_definitions, [&](const auto key, const auto& old_definition) {
					if (const auto new_definition = mapped_or_nullptr(new_defs.image_definitions, key)) {
						if (new_definition->loadables != old_definition.loadables) {
							/* Changed, so reload. */
							new_atlas_required = true;
						}
					}
					else {
						/* Missing, unload */
						new_atlas_required = true;
					}
				});

				for_each_id_and_object(new_defs.image_definitions, [&](const auto key, const auto&) {
					if (nullptr == mapped_or_nullptr(now_loaded_viewables_defs.image_definitions, key)) {
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

		if (new_atlas_required && !future_general_atlas.valid()) {
#if USE_PBO
			uploading_pbo.set_as_current();
			auto* const buffer_for_atlas = reinterpret_cast<rgba*>(uploading_pbo.map_buffer());

			auto unset_scope = augs::scope_guard([&]() { uploading_pbo.set_current_to_none(); });
#else
			auto* const buffer_for_atlas = no_pbo.data();
#endif

			auto general_atlas_in = general_atlas_input {
				{
					settings,
					necessary_image_definitions,
					new_defs.image_definitions,
					gui_font,
					unofficial_content_dir
				},

				max_atlas_size,
				buffer_for_atlas
			};

			future_general_atlas = std::async(
				std::launch::async,
				[general_atlas_in, this]() { 
					return create_general_atlas(general_atlas_in, general_atlas_performance, performance.neon_regeneration);
				}
			);

			future_image_definitions = new_defs.image_definitions;
			future_gui_font = gui_font;
		}
	}

	/* Sounds pass */

	{
		auto total = measure_scope_additive(performance.reloading_sounds);

		auto make_sound_loading_input = [&](const sound_definition& def) {
			const auto def_view = sound_definition_view(unofficial_content_dir, def);
			const auto input = def_view.make_sound_loading_input();

			return input;
		};

		/* Check for unloaded and changed resources */
		for_each_id_and_object(now_loaded_viewables_defs.sounds, [&](const auto& key, const auto& now_loaded) {
			auto unload = [&](){
				in.sounds.clear_sources_playing(key);
				loaded_sounds.erase(key);
			};

			if (const auto fresh = mapped_or_nullptr(new_defs.sounds, key)) {
				if (fresh->loadables != now_loaded.loadables) {
					auto scope = measure_scope(total);
					/* Different from the fresh one, reload */
					unload();

					try {
						loaded_sounds.try_emplace(key, make_sound_loading_input(*fresh));
					}
					catch (...) {

					}
				}
			}
			else {
				auto scope = measure_scope(total);
				/* Missing, unload */
				unload();
			}
		});

		/* Check for new resources */
		for_each_id_and_object(new_defs.sounds, [&](const auto& fresh_key, const auto& fresh_def) {
			if (nullptr == mapped_or_nullptr(now_loaded_viewables_defs.sounds, fresh_key)) {
				auto scope = measure_scope(total);

				try {
					loaded_sounds.try_emplace(fresh_key, make_sound_loading_input(fresh_def));
				}
				catch (...) {

				}
			}
			/* Otherwise it's already taken care of */
		});

		/* Done, overwrite */
		now_loaded_viewables_defs.sounds = new_defs.sounds;
	}

	//auto scope = measure_scope(performance.viewables_readback);
}

void viewables_streaming::finalize_load(viewables_finalize_input in) {
	/* Unpack asynchronous asset loading results */

	if (valid_and_is_ready(future_general_atlas)) {
		const bool measure_atlas_uploading = in.measure_atlas_upload;

		if (measure_atlas_uploading) {
			in.renderer.finish();
		}

		auto scope = cond_measure_scope(measure_atlas_uploading, performance.atlas_upload_to_gpu);

#if USE_PBO
		uploading_pbo.set_as_current();
		uploading_pbo.unmap_buffer();
#endif

		auto result = future_general_atlas.get();

		images_in_atlas = std::move(result.atlas_entries);
		necessary_images_in_atlas = std::move(result.necessary_atlas_entries);
		get_loaded_gui_font() = std::move(result.gui_font);

		now_loaded_gui_font_def = future_gui_font;

		auto& now_loaded_defs = now_loaded_viewables_defs.image_definitions;
		auto& new_loaded_defs = future_image_definitions;

		now_loaded_defs = new_loaded_defs;

		const auto atlas_size = result.atlas_size;

#if USE_PBO
		general_atlas.emplace(atlas_size);
		general_atlas->start_upload_from(uploading_pbo);

		augs::graphics::pbo::set_current_to_none();
#else
		general_atlas.emplace(atlas_size, no_pbo.data());
#endif
		if (measure_atlas_uploading) {
			in.renderer.finish();
		}
	}
}
