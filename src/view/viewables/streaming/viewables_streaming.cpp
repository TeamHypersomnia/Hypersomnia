#include "augs/graphics/renderer.h"
#include "augs/templates/thread_templates.h"
#include "view/viewables/streaming/viewables_streaming.h"
#include "view/audiovisual_state/systems/sound_system.h"
#include "augs/templates/introspection_utils/introspective_equal.h"

#include "view/viewables/regeneration/atlas_progress_structs.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

void viewables_streaming::finalize_pending_tasks() {
	if (future_loaded_buffers.valid()) {
		future_loaded_buffers.get();
	}

	if (future_general_atlas.valid()) {
		future_general_atlas.get();
	}

	if (future_avatar_atlas.valid()) {
		future_avatar_atlas.get();
	}
}

viewables_streaming::~viewables_streaming() {
	finalize_pending_tasks();
}

viewables_streaming::viewables_streaming(augs::renderer& renderer) {
	LOG_NVPS(std::thread::hardware_concurrency());
	(void)renderer;

#if EXPERIMENTAL_USE_PBO
	{
		auto scope = measure_scope(performance.pbo_allocation);
		uploading_pbo.reserve_for_texture_square(renderer.get_max_texture_size());
	}

	uploading_pbo.set_current_to_none();

	/* Warm up the PBO on startup, so that we can avoid freezes later */
	pbo_ready_to_use = true;
#endif
}

bool viewables_streaming::finished_loading_player_metas() const {
	return !future_avatar_atlas.valid();
}

void viewables_streaming::load_all(const viewables_load_input in) {
	const auto& new_all_defs = in.new_defs;
	auto& now_all_defs = now_loaded_viewables_defs;

	const auto& gui_fonts = in.gui_fonts;
	const auto& necessary_image_definitions = in.necessary_image_definitions;
	const auto settings = in.settings;
	const auto& unofficial_content_dir = in.unofficial_content_dir;
	const auto max_atlas_size = in.max_atlas_size;

	/* Avatar atlas pass */
	if (finished_loading_player_metas()) {
		if (in.new_player_metas != std::nullopt) {
			avatar_pbo_fallback.clear();

			auto avatar_atlas_in = avatar_atlas_input {
				std::move(*in.new_player_metas),
				max_atlas_size,

				nullptr,
				avatar_pbo_fallback
			};

			future_avatar_atlas = std::async(
				std::launch::async,
				[avatar_atlas_in]() { 
					return create_avatar_atlas(avatar_atlas_in);
				}
			);
		}
	}

	/* General atlas pass */

	if (!future_general_atlas.valid()) {
		bool new_atlas_required = settings.regenerate_every_time;

		auto& now_defs = now_all_defs.image_definitions;
		auto& new_defs = new_all_defs.image_definitions;

		{
			auto scope = measure_scope(performance.detecting_changed_viewables);

			{
				/* Check for unloaded and changed resources */
				for_each_id_and_object(now_defs, [&](const auto key, const auto& old_def) {
					if (const auto new_def = mapped_or_nullptr(new_defs, key)) {
						if (new_def->loadables_differ(old_def)) {
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

			if (!augs::introspective_equal(gui_fonts, now_loaded_gui_font_defs)) {
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

#if EXPERIMENTAL_USE_PBO
			const auto pbo_buffer = [this]() -> rgba* {
				if (pbo_ready_to_use) {
					auto scope = measure_scope(performance.pbo_map_buffer);
					uploading_pbo.set_as_current();
					return reinterpret_cast<rgba*>(uploading_pbo.map_buffer());
				}

				return nullptr;
			}();
#else
			rgba* pbo_buffer = nullptr;
#endif

			if (pbo_buffer == nullptr) {
				/* Prepare fallback */
				pbo_fallback.clear();
			}

			general_atlas_progress.emplace();

			auto general_atlas_in = general_atlas_input {
				{
					settings,
					necessary_image_definitions,
					new_defs,
					gui_fonts,
					unofficial_content_dir,

					std::addressof(*general_atlas_progress)
				},

				max_atlas_size,

				pbo_buffer,
				pbo_fallback
			};

			future_general_atlas = std::async(
				std::launch::async,
				[general_atlas_in, pbo_buffer, this]() { 
#if EXPERIMENTAL_USE_PBO
					if (pbo_buffer != nullptr) {
						/* Asynchronously clear the pbo_fallback if we will no longer use it */
						pbo_fallback.clear();
						pbo_fallback.shrink_to_fit();
					}
#endif
					(void)pbo_buffer;

					return create_general_atlas(general_atlas_in, general_atlas_performance, performance.neon_maps_regeneration);
				}
			);

			future_image_definitions = new_defs;
			future_gui_fonts = gui_fonts;

#if EXPERIMENTAL_USE_PBO
			if (pbo_buffer != nullptr) {
				uploading_pbo.set_current_to_none(); 
			}
#endif
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
				if (new_def.loadables_differ(*now_def)) {
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

	if (valid_and_is_ready(future_avatar_atlas)) {
		auto result = future_avatar_atlas.get();

		avatars_in_atlas = std::move(result.atlas_entries);

		const auto previous_texture = augs::graphics::texture::find_current();

		const auto atlas_size = result.atlas_size;
		avatar_atlas.emplace(atlas_size, avatar_pbo_fallback.data());
		avatar_atlas->set_filtering(augs::filtering_type::LINEAR);

		augs::graphics::texture::set_current_to(previous_texture);
	}

	/* Unpack results of asynchronous asset loading */

	if (valid_and_is_ready(future_general_atlas)) {
		const bool measure_atlas_uploading = in.measure_atlas_upload;

#if EXPERIMENTAL_USE_PBO
		const bool was_written_to_pbo = pbo_ready_to_use;
#endif

		if (measure_atlas_uploading) {
			in.renderer.finish();
		}

		auto scope = measure_scope(performance.atlas_upload_to_gpu);

#if EXPERIMENTAL_USE_PBO
		if (was_written_to_pbo) {
			auto scope = measure_scope(performance.pbo_unmap_buffer);

			uploading_pbo.set_as_current();
			uploading_pbo.unmap_buffer();
		}
#endif

		auto result = future_general_atlas.get();

		general_atlas_progress = std::nullopt;

		images_in_atlas = std::move(result.atlas_entries);
		necessary_images_in_atlas = std::move(result.necessary_atlas_entries);
		loaded_gui_fonts = std::move(result.gui_fonts);

		now_loaded_gui_font_defs = future_gui_fonts;

		auto& now_loaded_defs = now_all_defs.image_definitions;
		auto& new_loaded_defs = future_image_definitions;

		/* Done, overwrite */
		now_loaded_defs = new_loaded_defs;

		const auto atlas_size = result.atlas_size;

#if EXPERIMENTAL_USE_PBO
		if (was_written_to_pbo) {
			general_atlas.emplace(atlas_size);
			general_atlas->start_upload_from(uploading_pbo);

			augs::graphics::pbo::set_current_to_none();
		}
		else 
#endif
		{
			general_atlas.emplace(atlas_size, pbo_fallback.data());
		}

		if (measure_atlas_uploading) {
			in.renderer.finish();
		}

#if EXPERIMENTAL_USE_PBO
		pbo_ready_to_use = true;
#endif
	}

	if (valid_and_is_ready(future_loaded_buffers)) {
		auto& now_loaded_defs = now_all_defs.sounds;
		auto& new_loaded_defs = future_sound_definitions;

		auto unload = [&](const assets::sound_id key) {
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

void viewables_streaming::display_loading_progress() const {
	using namespace augs::imgui;

	std::string loading_message;

	const auto& progress = general_atlas_progress;
	float progress_percent = -1.f;

	if (progress != std::nullopt) {
		const auto& a_neon_i = progress->current_neon_map_num;
		const auto neon_i = a_neon_i.load();

		const auto& a_neon_max_i = progress->max_neon_maps;
		const auto neon_max_i = a_neon_max_i.load();

		const auto neons_finished = neon_i == 0 || neon_i == neon_max_i;

		if (!neons_finished) {
			loading_message = typesafe_sprintf("Regenerating neon map %x of %x...", neon_i, neon_max_i);

			progress_percent = float(neon_i) / neon_max_i;
		}
		else {
			loading_message = "Loading the game atlas...";
		}
	}

	if (loading_message.size() > 0) {
		loading_message += "\n";

		ImGui::SetNextWindowPosCenter();
		auto loading_window = scoped_window("Loading in progress", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

		text_color("The game is regenerating resources. Please be patient.\n", yellow);
		ImGui::Separator();

		text(loading_message);

		if (progress_percent >= 0.f) {
			ImGui::ProgressBar(progress_percent, ImVec2(-1.0f,0.0f));
		}
	}
}
