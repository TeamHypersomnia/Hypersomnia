#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "augs/filesystem/path.h"
#include "game/modes/mode_entropy.h"
#include "application/arena/synced_dynamic_vars.h"
#include "application/setups/setup_common.h"
#include "application/setups/editor/playtest_recording/playtest_replay_capture_settings.h"
#include "application/setups/editor/playtest_recording/playtest_replay_capture_output.h"

class editor_setup;
struct editor_project;
struct editor_playtest_start;
struct editor_playtest_recorder;
struct editor_playtest_replay;
struct game_frame_buffer;

namespace augs {
	class window;
	class image;
}

class editor_playtest_recordings {
	std::unique_ptr<editor_playtest_recorder> recorder;
	std::unique_ptr<editor_playtest_replay> replay;

	std::optional<playtest_replay_capture_output> capture_output;
	bool capture_hides_hud = false;
	bool capture_waits_for_viewables = false;
	bool capture_failed = false;
	bool frame_requested_now = false;

	void rebuild_arena_for_replay(editor_setup&, const editor_project& recorded_project, bool showing_navmesh) const;

public:
	editor_playtest_recordings();
	~editor_playtest_recordings();

	bool is_replaying() const {
		return replay != nullptr;
	}

	bool is_replay_paused() const;
	bool replay_uses_free_camera() const;
	bool replay_requires_cursor() const;
	bool replay_hides_hud(bool hud_hidden_in_editor_replays) const;

	bool captures_frames() const {
		return capture_output.has_value();
	}

	bool capture_complete() const;

	void begin_recording(editor_setup&, const editor_playtest_start&, bool built_showing_navmesh);
	void before_recorded_step(editor_setup&);
	void record_step(const editor_setup&, const mode_entropy& total, const synced_dynamic_vars& dynamic_vars_in_effect);
	void finish_recording(editor_setup&);

	bool start_replaying(editor_setup&, const augs::path_type& recording_folder);
	void stop_replaying(editor_setup&);

	void perform_requested_seek(editor_setup&);
	std::optional<mode_entropy> begin_replayed_step(editor_setup&);
	void end_replayed_step(const editor_setup&);

	void perform_replay_controls(editor_setup&, augs::window&);
	bool handle_replay_input(editor_setup&, handle_input_before_game_input);

	void start_capturing(editor_setup&, const playtest_replay_capture_settings&);
	void advance_capture(bool all_viewables_loaded);
	void request_frame(game_frame_buffer&);
	void queue_audio_of_captured_frame(const std::vector<int16_t>& samples);
	void write_frame(const augs::image&);
};
