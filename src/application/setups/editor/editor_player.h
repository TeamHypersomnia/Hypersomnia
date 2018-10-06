#pragma once
#include <memory>
#include <map>
#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_entropy.h"

#include "application/setups/editor/player/editor_player_step_type.h"
#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_view.h"

struct cosmos_solvable_significant;

struct editor_folder;
struct editor_command_input;

namespace augs {
	struct introspection_access;
}

struct intercosm;

struct player_before_start_state {
	using revision_type = editor_history::index_type;

	// GEN INTROSPECTOR struct player_before_start_state
	std::unique_ptr<intercosm> work;
	editor_view_ids view_ids;
	all_mode_vars_maps mode_vars;
	revision_type revision = -1;
	// END GEN INTROSPECTOR
};

enum class finish_testing_type {
	REAPPLY_CHANGES,
	DISCARD_CHANGES
};

class editor_player {
	using entropy_type = mode_entropy;

	enum class advance_type {
		RECORDING,
		REPLAYING
	};

	// GEN INTROSPECTOR class editor_player
	double speed = 1.0;
	int additional_steps = 0;
	bool paused = true;
	advance_type advance_mode = advance_type::RECORDING;

	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	editor_player_step_type current_step = 0;
	std::map<editor_player_step_type, entropy_type> step_to_entropy;
	mode_entropy total_collected_entropy;

	all_modes_variant current_mode;
	mode_vars_id current_mode_vars_id = mode_vars_id();

	std::optional<player_before_start_state> before_start;
	// END GEN INTROSPECTOR

	friend augs::introspection_access;
	friend struct editor_property_accessors;

	template <class E, class A, class C, class F>
	static decltype(auto) on_mode_with_input_impl(
		E& self,
		const A& all_vars,
		C& cosm,
		F&& callback
	);

	void save_state_before_start(editor_folder&);
	void restore_saved_state(editor_folder&);

	void initialize_testing(editor_folder&);

public:

	double get_speed() const;
	void set_speed(double);

	bool is_editing_mode() const;
	bool has_testing_started() const;

	void control(const cosmic_entropy& entropy);
	void control(const mode_entropy& entropy);

	void request_additional_step();

	void begin_replaying();
	void begin_recording();

	void pause();
	void finish_testing(editor_command_input, finish_testing_type);

	void start_resume(editor_folder&);
	void start_pause_resume(editor_folder&);

	template <class M>
	void choose_mode(const mode_vars_id& vars_id);

	void ensure_handler();

	template <class A, class... Callbacks>
	void advance_player(
		augs::delta frame_delta,
		const A& all_vars,
		cosmos& cosm,
		Callbacks&&... callbacks
	);

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) {
		return on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) const {
		return on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	const auto& get_timer() const {
		return timer;
	}

	entity_guid lookup_character(mode_player_id) const;

	using revision_type = editor_history::index_type;

	revision_type get_revision_when_started_testing() const;
};
