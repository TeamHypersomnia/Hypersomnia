#pragma once
#include "setup_base.h"

#include "game/view/debug_character_selection.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/cosmic_movie_director.h"

class director_setup : public setup_base {
	void push_snapshot_if_needed();
	void advance_player_by_single_step(viewing_session& session);

public:
	enum class recording_type {
		ALL,
		ONLY_KEYS,
		ONLY_MOUSE,

		COUNT
	};

	enum class recording_flags {
		ALLOW_KEYS,
		ALLOW_MOUSE,

		COUNT
	};
	
	using recording_boolset = augs::enum_boolset<recording_flags>;

	recording_boolset get_flags();

	enum class director_state {
		PLAYING,
		RECORDING
	};
	
	all_logical_assets logical_assets;

	director_state current_director_state = director_state::PLAYING;
	recording_type recording_mode = recording_type::ALL;

	augs::event::state events;
	cosmos hypersomnia = cosmos(3000);
	debug_character_selection characters;

	std::string input_director_path;
	std::string output_director_path;

	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);

	bool unsaved_changes_exist = false;

	unsigned bookmarked_step = 0;

	cosmic_movie_director director;

	double basic_playback_speed = 1.0;
	double requested_playing_speed = 0.0;

	std::vector<cosmos> snapshots_for_rewinding;
	unsigned initial_step_number = 0u;
	unsigned snapshot_frequency_in_steps = 0u;
	
	unsigned get_step_number(const cosmos&) const;

	void set_snapshot_frequency_in_seconds(const double);

	augs::gui::text::formatted_string get_status_text() const;

	void init(
		augs::window&,
		viewing_session&
	);

	augs::machine_entropy control_player(
		augs::window& window,
		viewing_session& session
	);

	void seek_to_step(
		const unsigned step_number,
		viewing_session& session
	);

	void clear_accumulated_inputs();

	void advance_player(viewing_session& session);

	void process(
		augs::window&,
		viewing_session&
	);

	void advance_audiovisuals(
		viewing_session&
	);

	void view(
		viewing_session&
	);
	
	bool should_show_editor_gui() const;

	void save_unsaved_changes();

	~director_setup();
};