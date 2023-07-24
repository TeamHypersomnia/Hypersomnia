#include "augs/log.h"
#include <unordered_set>
#include "augs/graphics/renderer.h"
#include "augs/templates/thread_templates.h"
#include "view/viewables/streaming/viewables_streaming.h"
#include "view/audiovisual_state/systems/sound_system.h"
#include "augs/misc/imgui/imgui_utils.h"

#include "augs/audio/audio_command_buffers.h"
#include "augs/audio/audio_backend.h"
#include "view/viewables/regeneration/atlas_progress_structs.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/filesystem/file.h"

void viewables_streaming::request_rescan() {
	if (!general_atlas.empty()) {
		rescan_for_modified_images = true;
	}

	rescan_for_modified_sounds = true;
}

void viewables_streaming::finalize_pending_tasks() {
	if (future_loaded_buffers.valid()) {
		future_loaded_buffers.get();
	}

	if (future_general_atlas.valid()) {
		future_general_atlas.get();
	}

	avatars.finalize_tasks_if_any();
	ad_hoc.finalize_tasks_if_any();
}

viewables_streaming::~viewables_streaming() {
	finalize_pending_tasks();
}

template <class D, class F>
static auto make_current_registry_of_write_times(const D& dir, const F& from_defs) {
	auto get_write_time = [](const auto& dir, const auto& def) {
		const auto path = image_definition_view(dir, def).get_source_image_path();

		try {
			return augs::last_write_time(path);
		}
		catch (const augs::filesystem_error&) {
			return augs::file_time_type();
		}
	};

	std::vector<augs::file_time_type> output;

	output.reserve(from_defs.size());

	for (const auto& v : from_defs) {
		output.emplace_back(get_write_time(dir, v));
	}

	return output;
};

bool viewables_streaming::finished_generating_atlas() const {
	return !future_general_atlas.valid();
}

void viewables_streaming::load_all(const viewables_load_input in) {
	const auto current_frame = in.current_frame;
	const auto& new_all_defs = in.new_defs;
	auto& now_all_defs = now_loaded_viewables_defs;

	const auto& gui_fonts = in.gui_fonts;
	const auto& necessary_image_definitions = in.necessary_image_definitions;
	const auto settings = in.settings;
	const auto& unofficial_content_dir = in.unofficial_content_dir;
	const auto max_atlas_size = in.max_atlas_size;

	if (in.new_player_metas.has_value()) {
		last_requested_player_metas = std::move(in.new_player_metas);
	}

	/* Avatar atlas pass */
	if (avatars.work_slot_free(current_frame)) {
		if (last_requested_player_metas.has_value()) {
			auto avatar_atlas_in = avatar_atlas_input {
				std::move(*last_requested_player_metas),
				max_atlas_size,

				nullptr,
				avatars.pbo_fallback
			};

			last_requested_player_metas = std::nullopt;

			avatars.submit_work([avatar_atlas_in]() { 
				return create_avatar_atlas(avatar_atlas_in);
			});
		}
	}

	if (in.ad_hoc_subjects.has_value()) {
		last_requested_ad_hoc_atlas = std::move(in.ad_hoc_subjects);
	}

	if (ad_hoc.work_slot_free(current_frame)) {
		if (last_requested_ad_hoc_atlas.has_value()) {
			auto ad_hoc_atlas_in = ad_hoc_atlas_input {
				std::move(*last_requested_ad_hoc_atlas),
				max_atlas_size,

				nullptr,
				ad_hoc.pbo_fallback
			};

			last_requested_ad_hoc_atlas = std::nullopt;

			ad_hoc.submit_work([ad_hoc_atlas_in]() { 
				return create_ad_hoc_atlas(ad_hoc_atlas_in);
			});
		}
	}

	/* General atlas pass */

	const bool atlas_upload_complete = augs::has_completed(current_frame, general_atlas_submitted_when);
	const bool atlas_generation_complete = finished_generating_atlas();

	if (atlas_upload_complete && atlas_generation_complete) {
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

			if (gui_fonts != now_loaded_gui_font_defs) {
				new_atlas_required = true;
			}
		}

		if (necessary_images_in_atlas.empty()) {
			new_atlas_required = true;
		}

		if (general_atlas.empty()) {
			new_atlas_required = true;
		}

		if (!new_atlas_required) {
			if (rescan_for_modified_images) {
				augs::timer t;

				const auto current_times = make_current_registry_of_write_times(
					unofficial_content_dir,
					now_loaded_viewables_defs.image_definitions
				);

				if (current_times != image_write_times) {
					LOG("Detected modified image file(s). Reloading atlas.");

					new_atlas_required = true;
					image_write_times = current_times;
				}

				LOG("Rescanning for modified images took %x ms", t.get<std::chrono::milliseconds>());
			}
		}

		rescan_for_modified_images = false;

		if (new_atlas_required) {
			auto scope = measure_scope(performance.launching_atlas_reload);

			rgba* pbo_buffer = nullptr;

			if (pbo_buffer == nullptr) {
				/* Prepare fallback */
				general_atlas_pbo_fallback.clear();
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
				general_atlas_pbo_fallback
			};

			future_general_atlas = launch_async(
				[general_atlas_in, this]() { 
					if (!rescan_for_modified_images) {
						/* 
							The request was not due to source image modification, 
							so we have yet to re-scan the modified times. 
						*/

						image_write_times = make_current_registry_of_write_times(
							general_atlas_in.subjects.unofficial_project_dir,
							general_atlas_in.subjects.image_definitions
						);
					}

					return create_general_atlas(general_atlas_in, general_atlas_performance);
				}
			);

			future_image_definitions = new_defs;
			future_gui_fonts = gui_fonts;
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
			future_loaded_buffers = launch_async(
				[&](){
					using value_type = decltype(future_loaded_buffers.get());

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

		rescan_for_modified_sounds = false;
	}
}

template <class output_type>
void texture_in_progress<output_type>::finalize_load(augs::renderer& renderer) {
	if (valid_and_is_ready(future_output)) {
		auto result = future_output.get();

		in_atlas = std::move(result.atlas_entries);

		const auto atlas_size = result.atlas_size;
		texture.texImage2D(renderer, augs::image(std::move(pbo_fallback), atlas_size));
		pbo_fallback = {};
		renderer.finish();
		texture.set_filtering(renderer, augs::filtering_type::LINEAR);

		augs::graphics::texture::set_current_to_previous(renderer);
	}
}

void viewables_streaming::finalize_load(viewables_finalize_input in) {
	const auto current_frame = in.current_frame;
	auto& now_all_defs = now_loaded_viewables_defs;

	avatars.finalize_load(in.renderer);
	ad_hoc.finalize_load(in.renderer);

	/* Unpack results of asynchronous asset loading */

	if (valid_and_is_ready(future_general_atlas)) {
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

		general_atlas.texImage2D(in.renderer, augs::image(std::move(general_atlas_pbo_fallback), result.atlas_size));
		general_atlas_pbo_fallback = {};
		general_atlas_submitted_when = current_frame;
	}

	if (valid_and_is_ready(future_loaded_buffers)) {
		auto& now_loaded_defs = now_all_defs.sounds;
		auto& new_loaded_defs = future_sound_definitions;

		{
			thread_local std::unordered_set<ALuint> all_unloaded_buffers;

			in.audio_buffers.finish();

			all_unloaded_buffers.clear();

			auto buffer_unloaded = [&](const ALuint buffer_id) {
				return found_in(all_unloaded_buffers, buffer_id);
			};

			for (const auto& r : sound_requests) {
				if (const auto loaded_sound = mapped_or_nullptr(loaded_sounds, r.first)) {
					for (const auto& v : loaded_sound->get_variations()) {
						all_unloaded_buffers.emplace(v.get_id());
					}
				}
			}

			// TODO_PERFORMANCE
			in.audio_buffers.stop_sources_if(buffer_unloaded);
		}

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

	if (progress.has_value()) {
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

		center_next_window(ImGuiCond_Always);
		auto loading_window = scoped_window("Loading in progress", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing);

		text_color("The game is regenerating resources. Please be patient.\n", yellow);
		ImGui::Separator();

		text(loading_message);

		if (progress_percent >= 0.f) {
			ImGui::ProgressBar(progress_percent, ImVec2(-1.0f,0.0f));
		}
	}
}
