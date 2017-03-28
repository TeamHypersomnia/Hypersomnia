#pragma once
#include "setup_base.h"
#include "game/view/viewing_session.h"
#include "game/scene_builders/testbed.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/cosmic_movie_director.h"

class game_window;

class director_setup : public setup_base {
	void push_snapshot_if_needed();
	void advance_player_by_single_step(viewing_session& session);

public:
	enum class recording_replacement_type {
		ALL,
		ONLY_KEYS,
		ONLY_MOUSE,

		COUNT
	};

	enum class director_state {
		PLAYING,
		RECORDING
	};
	
	director_state current_director_state = director_state::PLAYING;
	recording_replacement_type recording_replacement_mode = recording_replacement_type::ALL;

	augs::window::event::state events;
	cosmos hypersomnia = cosmos(3000);
	scene_builders::testbed testbed;

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
		const config_lua_table& cfg, 
		game_window&,
		viewing_session&
	);

	void control_player(
		const config_lua_table& cfg,
		game_window& window,
		viewing_session& session
	);

	void seek_to_step(
		const unsigned step_number,
		viewing_session& session
	);

	void clear_accumulated_inputs();

	void advance_player(viewing_session& session);

	void process(
		const config_lua_table& cfg, 
		game_window&,
		viewing_session&
	);

	void view(
		const config_lua_table& cfg,
		viewing_session&
	);

	void save_unsaved_changes();

	~director_setup();
};