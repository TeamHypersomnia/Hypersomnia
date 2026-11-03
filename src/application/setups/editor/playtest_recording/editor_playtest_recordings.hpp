#pragma once
#include <algorithm>
#include "augs/filesystem/file.h"
#include "augs/readwrite/byte_file.h"
#include "augs/templates/chrono_templates.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/maybe_different.h"
#include "augs/graphics/renderer.h"
#include "application/main/game_frame_buffer.h"
#include "game/cosmos/cosmic_functions.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recorder.h"
#include "application/setups/editor/playtest_recording/editor_playtest_replay.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording_files.h"
#include "application/setups/editor/playtest_recording/playtest_render_command.h"
#include "hypersomnia_version.h"

void snap_interpolated_to_logical(cosmos&);

editor_playtest_recordings::editor_playtest_recordings() = default;
editor_playtest_recordings::~editor_playtest_recordings() = default;

bool editor_playtest_recordings::is_replay_paused() const {
	return is_replaying() && replay->paused;
}

bool editor_playtest_recordings::replay_uses_free_camera() const {
	return is_replaying() && replay->free_camera;
}

bool editor_playtest_recordings::replay_requires_cursor() const {
	return is_replaying() && !replay->controls_hidden;
}

bool editor_playtest_recordings::capture_complete() const {
	if (capture_failed) {
		return true;
	}

	return capture_output.has_value() && capture_output->complete();
}

void editor_playtest_recordings::begin_recording(
	editor_setup& editor,
	const editor_playtest_start& start,
	const bool built_showing_navmesh
) {
	recorder.reset();

	const auto when = augs::date_time();
	const auto folder = ::first_free_recording_folder(editor.get_recorded_playtests_folder(), when);

	try {
		augs::create_directories(folder);

		auto new_recorder = std::make_unique<editor_playtest_recorder>();
		new_recorder->paths = editor_playtest_recording_paths(folder);

		auto& meta = new_recorder->meta;
		meta.recorded_at_unix = static_cast<uint64_t>(when.t);
		meta.game_version = hypersomnia_version().get_version_string();
		meta.game_mode = editor.get_name(editor.project.playtesting.mode);
		meta.nickname = std::string(start.nickname);
		meta.initial_spawn_pos = start.initial_spawn_pos;
		meta.rng_offset = start.rng_offset;
		meta.built_showing_navmesh = built_showing_navmesh;

		new_recorder->inputs.dynamic_vars_before_setup = editor.dummy_dynamic_vars;

		editor_project_readwrite::write_project_json(
			new_recorder->paths.map_json,
			editor.project,
			editor.official.resources,
			editor.official.resource_map
		);

		recorder = std::move(new_recorder);
	}
	catch (const std::exception& err) {
		LOG("Could not start recording the playtest into %x: %x", folder, err.what());
	}
}

void editor_playtest_recordings::before_recorded_step(editor_setup& editor) {
	if (recorder != nullptr && recorder->next_step_reinfers()) {
		cosmic::reinfer_solvable(editor.scene.world);
	}
}

void editor_playtest_recordings::record_step(
	const editor_setup& editor,
	const mode_entropy& total,
	const synced_dynamic_vars& dynamic_vars_in_effect
) {
	if (recorder == nullptr) {
		return;
	}

	recorder->record_step(
		total,
		dynamic_vars_in_effect,
		editor.scene.world.calculate_solvable_signi_hash<uint32_t>(),
		editor.get_inv_tickrate()
	);
}

void editor_playtest_recordings::finish_recording(editor_setup& editor) {
	if (recorder == nullptr) {
		return;
	}

	const auto finished = std::move(recorder);
	const auto& recording_paths = finished->paths;

	try {
		if (finished->too_short_to_keep()) {
			::remove_recorded_playtest_folder(recording_paths);
			return;
		}

		augs::save_as_bytes(finished->inputs, recording_paths.inputs_bin);
		augs::save_as_json(finished->meta, recording_paths.meta_json);

		::remove_oldest_unnamed_recordings(editor.get_recorded_playtests_folder());

		auto& recordings_gui = editor.gui.recorded_playtests;
		recordings_gui.rescan_requested = true;
		recordings_gui.selected_folder = recording_paths.folder;
	}
	catch (const std::exception& err) {
		LOG("Could not save the recorded playtest into %x: %x", recording_paths.folder, err.what());
	}
}

void editor_playtest_recordings::rebuild_arena_for_replay(
	editor_setup& editor,
	const editor_project& recorded_project,
	const bool showing_navmesh
) const {
	const bool for_playtesting = true;
	const bool editor_preview = false;

	::build_arena_from_editor_project<editor_arena_handle<false>>(
		editor.get_arena_handle(),
		{
			recorded_project,
			game_mode_name_type(""),
			editor.paths.project_folder,
			editor.official,
			nullptr,
			std::addressof(editor.clean_round_state),
			for_playtesting,
			editor_preview,
			showing_navmesh
		}
	);
}

bool editor_playtest_recordings::start_replaying(editor_setup& editor, const augs::path_type& recording_folder) {
	if (editor.is_playtesting()) {
		editor.stop_playtesting();
	}

	auto new_replay = std::make_unique<editor_playtest_replay>();
	const auto recording_paths = editor_playtest_recording_paths(recording_folder);

	auto recorded_project = editor_project();

	try {
		new_replay->recording = { recording_paths, augs::from_json_file<editor_playtest_recording_meta>(recording_paths.meta_json) };
		augs::load_from_bytes(new_replay->inputs, recording_paths.inputs_bin);

		auto reading_settings_like_editor = editor_project_readwrite::reading_settings();
		reading_settings_like_editor.read_scratchpad = true;

		recorded_project = editor_project_readwrite::read_project_json(
			editor.paths.project_folder,
			augs::file_to_string_crlf_to_lf(recording_paths.map_json),
			editor.official.resources,
			editor.official.resource_map,
			reading_settings_like_editor
		);
	}
	catch (const std::exception& err) {
		editor.recent_message.set("Could not load the recorded playtest %x:\n%x", recording_folder.filename(), err.what());
		LOG("Could not load the recorded playtest %x: %x", recording_folder, err.what());
		return false;
	}

	const auto& meta = new_replay->recording.meta;

	rebuild_arena_for_replay(editor, recorded_project, meta.built_showing_navmesh);

	new_replay->dynamic_vars_before_replay = editor.dummy_dynamic_vars;
	editor.dummy_dynamic_vars = new_replay->inputs.dynamic_vars_before_setup;

	auto start = editor_playtest_start();
	start.initial_spawn_pos = meta.initial_spawn_pos;
	start.rng_offset = meta.rng_offset;
	start.nickname = client_nickname_type(meta.nickname);

	editor.setup_playtest_session(recorded_project.playtesting, start);

	new_replay->snapshots[0] = { editor.scene.world.get_solvable().significant, editor.current_mode_state };

	editor.recent_message.set("Replaying %x. Press ESC to quit.", new_replay->recording.get_display_name());
	editor.timer = augs::fixed_delta_timer { 5, augs::lag_spike_handling_type::DISCARD };

	replay = std::move(new_replay);

	return true;
}

void editor_playtest_recordings::stop_replaying(editor_setup& editor) {
	if (is_replaying()) {
		editor.dummy_dynamic_vars = replay->dynamic_vars_before_replay;
		replay.reset();
	}
}

std::optional<mode_entropy> editor_playtest_recordings::begin_replayed_step(editor_setup& editor) {
	if (replay->reached_last_step()) {
		replay->paused = true;

		if (!replay->reported_last_step) {
			replay->reported_last_step = true;
			LOG("The replay reached step %x - %x.", replay->current_step, replay->describe_sync());
		}

		return std::nullopt;
	}

	if (::is_recorded_playtest_reinference_step(replay->current_step)) {
		cosmic::reinfer_solvable(editor.scene.world);

		if (replay->snapshot_missing_at_current_step()) {
			replay->snapshots[replay->current_step] = { editor.scene.world.get_solvable().significant, editor.current_mode_state };
		}
	}

	editor.dummy_dynamic_vars = replay->dynamic_vars_at(replay->current_step);

	return replay->entropy_at(replay->current_step);
}

void editor_playtest_recordings::end_replayed_step(const editor_setup& editor) {
	const bool was_in_sync = !replay->first_desync_step.has_value();

	replay->verify_state_after_current_step(editor.scene.world.calculate_solvable_signi_hash<uint32_t>());

	if (was_in_sync && replay->first_desync_step.has_value()) {
		LOG("The recorded playtest %x desynced at step %x.", replay->recording.paths.folder, replay->current_step);
	}

	++replay->current_step;
}

void editor_playtest_recordings::perform_requested_seek(editor_setup& editor) {
	if (!replay->requested_seek.has_value()) {
		return;
	}

	const auto target = *replay->requested_seek;
	replay->requested_seek = std::nullopt;

	if (const auto snapshot = replay->find_snapshot_to_restore_for(target)) {
		editor.scene.world.set(snapshot->second.solvable);
		editor.current_mode_state = snapshot->second.mode;
		replay->current_step = snapshot->first;
	}

	while (replay->current_step < target) {
		const auto replayed = begin_replayed_step(editor);

		if (!replayed.has_value()) {
			break;
		}

		editor.advance_playtest_step(*replayed, solver_callbacks());
		end_replayed_step(editor);
	}

	::snap_interpolated_to_logical(editor.scene.world);
	editor.scene.world.request_resample();
}

void editor_playtest_recordings::perform_replay_controls(editor_setup& editor, augs::window& window) {
	using namespace augs::imgui;

	if (!is_replaying() || replay->controls_hidden) {
		return;
	}

	auto& shown = *replay;

	const auto viewport_size = ImGui::GetMainViewport()->WorkSize;
	ImGui::SetNextWindowPos(ImVec2(viewport_size.x / 2, viewport_size.y - 20), ImGuiCond_FirstUseEver, ImVec2(0.5f, 1.0f));
	ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_FirstUseEver);

	auto controls = scoped_window("Playtest replay", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

	text_color(shown.recording.get_display_name(), cyan);

	if (!shown.recording.meta.game_mode.empty()) {
		ImGui::SameLine();
		text_disabled(typesafe_sprintf("(%x)", shown.recording.meta.game_mode));
	}

	{
		auto scope = maybe_disabled_cols({}, shown.current_step == 0);

		if (ImGui::Button("Rewind")) {
			shown.seek_to(0);
		}
	}

	ImGui::SameLine();

	if (shown.paused) {
		if (ImGui::Button("Resume")) {
			shown.resume_or_restart_at_end();
		}
	}
	else {
		if (ImGui::Button("Pause")) {
			shown.paused = true;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Hide (Alt+P)")) {
		shown.controls_hidden = true;
	}

	if (shown.get_num_steps() > 0) {
		auto shown_secs = static_cast<float>(shown.get_current_secs());
		const auto duration_secs = static_cast<float>(shown.recording.meta.duration_secs);

		if (slider("Time", shown_secs, 0.f, duration_secs, "%.1f s")) {
			shown.seek_to(shown.secs_to_step(std::clamp(shown_secs, 0.f, duration_secs)));
		}
	}

	text(
		"%x / %x   Step %x / %x   Speed %xx",
		::format_mins_secs(shown.get_current_secs()),
		::format_mins_secs(shown.recording.meta.duration_secs),
		shown.current_step,
		shown.get_num_steps(),
		editor.playtest_speed
	);

	if (shown.first_desync_step.has_value()) {
		text_color(
			typesafe_sprintf(
				"Desynced at %x (step %x) - the game changed since the recording.",
				::format_mins_secs(*shown.first_desync_step * shown.get_secs_per_step()),
				*shown.first_desync_step
			),
			red
		);
	}

	if (checkbox("Free camera (drag with RMB, zoom with the wheel)", shown.free_camera)) {
		if (shown.free_camera) {
			if (const auto viewed = editor.get_viewed_character()) {
				editor.view.panned_camera.transform.pos = viewed.get_logic_transform().pos;
			}
		}
	}

	if (ImGui::Button("Copy render command")) {
		auto request = playtest_render_request();
		request.recording_folder = shown.recording.paths.folder;
		request.from_secs = shown.get_current_secs();
		request.to_secs = shown.recording.meta.duration_secs;

		if (shown.free_camera) {
			request.camera = editor.view.panned_camera;
		}

		ImGui::SetClipboardText(::make_playtest_render_command(request).c_str());
		editor.recent_message.set("Copied the command rendering this replay from %x to the end.", ::format_mins_secs(request.from_secs));
	}

	if (ImGui::IsItemHovered()) {
		text_tooltip("Renders from the current moment to the end at 60 fps, with sound,\nwith the free camera if it is on - otherwise following the player.");
	}

	ImGui::SameLine();

	if (ImGui::Button("Reveal in explorer")) {
		window.reveal_in_explorer(shown.recording.paths.meta_json);
	}

	text_disabled("Space - Pause/Resume   Left/Right - Seek 5 s (with Shift 1 s)\nNumpad keys - speed (0 resets to 1x)   Alt+P - Hide/show this window   Esc - Quit");
}

bool editor_playtest_recordings::handle_replay_input(editor_setup& editor, const handle_input_before_game_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	if (!is_replaying()) {
		return false;
	}

	const auto& state = in.common_input_state;
	const auto& e = in.e;

	if (e.was_any_key_pressed()) {
		const bool has_shift { state[key::LSHIFT] };
		const bool has_alt { state[key::LALT] };
		const auto seek_secs = has_shift ? 1.0 : 5.0;

		switch (e.data.key.key) {
			case key::SPACE:
				if (replay->paused) {
					replay->resume_or_restart_at_end();
				}
				else {
					replay->paused = true;
				}

				return true;
			case key::LEFT: replay->seek_by_secs(-seek_secs); return true;
			case key::RIGHT: replay->seek_by_secs(seek_secs); return true;
			case key::P:
				if (has_alt) {
					replay->controls_hidden = !replay->controls_hidden;
					return true;
				}

				break;
			default: break;
		}
	}

	if (replay->free_camera) {
		const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);

		return editor_detail::handle_camera_input(
			{},
			camera_cone(editor.get_camera_eye(), screen_size),
			state,
			e,
			editor.get_world_cursor_pos(),
			editor.view.panned_camera
		);
	}

	return false;
}

void editor_playtest_recordings::start_capturing(editor_setup& editor, const playtest_replay_capture_settings& capture) {
	if (!start_replaying(editor, capture.recording_folder)) {
		capture_failed = true;
		return;
	}

	if (capture.to_secs.has_value()) {
		replay->stop_at_step = replay->secs_to_step(*capture.to_secs);
	}

	replay->seek_to(replay->secs_to_step(capture.from_secs));
	perform_requested_seek(editor);

	if (capture.camera.has_value()) {
		replay->free_camera = true;
		editor.view.panned_camera = *capture.camera;
	}

	if (capture.records_frames()) {
		replay->controls_hidden = true;
		capture_waits_for_viewables = true;
		capture_output.emplace(capture.frames_path, capture.audio_path);
	}

	LOG(
		"Replaying %x from step %x to step %x of %x.",
		capture.recording_folder,
		replay->current_step,
		replay->get_last_step(),
		replay->get_num_steps()
	);
}

void editor_playtest_recordings::advance_capture(const bool all_viewables_loaded) {
	if (!capture_output.has_value() || !is_replaying()) {
		return;
	}

	if (capture_waits_for_viewables == all_viewables_loaded) {
		LOG("Replay capture at step %x: %x.", replay->current_step, all_viewables_loaded ? "all viewables loaded" : "waiting for viewables");
	}

	capture_waits_for_viewables = !all_viewables_loaded;

	if (!replay->reached_last_step()) {
		replay->paused = capture_waits_for_viewables;
	}
}

void editor_playtest_recordings::request_frame(game_frame_buffer& write_buffer) {
	frame_requested_now = false;

	if (!capture_output.has_value() || !is_replaying()) {
		return;
	}

	if (capture_waits_for_viewables || !capture_output->wants_more_frames()) {
		return;
	}

	write_buffer.renderers.all[renderer_type::GENERAL].screenshot(xywhi(vec2i::zero, write_buffer.screen_size));
	capture_output->on_frame_requested(replay->reached_last_step());
	frame_requested_now = true;
}

void editor_playtest_recordings::queue_audio_of_captured_frame(const std::vector<int16_t>& samples) {
	if (frame_requested_now && capture_output.has_value()) {
		capture_output->queue_audio_of_requested_frame(samples);
	}
}

void editor_playtest_recordings::write_frame(const augs::image& frame) {
	if (capture_output.has_value()) {
		capture_output->write_frame(frame);
	}
}
