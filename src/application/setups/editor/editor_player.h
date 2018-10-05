#pragma once
#include <memory>
#include <map>
#include "application/setups/editor/editor_history.h"
#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_entropy.h"
#include "augs/misc/timing/fixed_delta_timer.h"
#include "view/mode_gui/arena/arena_mode_gui.h"

#include "application/setups/editor/player/editor_player_step_type.h"

struct cosmos_solvable_significant;

struct editor_folder;

class cosmos;

namespace augs {
	struct introspection_access;
}

struct player_before_start_state {
	// GEN INTROSPECTOR struct player_before_start_state
	std::unique_ptr<intercosm> work;
	editor_view_ids view_ids;
	all_mode_vars_maps mode_vars;
	revision_type revision = -1;
	// END GEN INTROSPECTOR
};

class editor_player {
	using revision_type = editor_history::index_type;
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

	editor_player_step_type current_step = 0;
	std::map<editor_player_step_type, entropy_type> step_to_entropy;
	mode_entropy total_collected_entropy;

	all_modes_variant current_mode;
	mode_vars_id current_mode_vars_id = mode_vars_id();
	std::optional<player_before_start_state> before_start;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	// END GEN INTROSPECTOR

	friend augs::introspection_access;

	template <class E, class A, class F>
	static decltype(auto) on_mode_with_input_impl(
		E& self,
		const all_mode_vars_maps& all_vars,
		cosmos& cosm,
		F&& callback
	);

	void save_state_before_start(const editor_folder&);
	void restore_saved_state(editor_folder&);

public:

	double get_speed() const;
	bool is_editing_mode() const;
	bool has_testing_started() const;
	void control(const cosmic_entropy& entropy);
	void request_additional_step();

	void begin_replaying();
	void begin_recording();

	void pause();
	void quit_testing_and_reapply(editor_folder&);
	void start_pause_resume(editor_folder&);

	template <class M>
	void init_mode(M&& mode, const mode_vars_id& vars_id);

	void ensure_handler();

	template <class A, class... Callbacks>
	void advance_player(
		augs::delta frame_delta,
		const A& all_vars,
		cosmos& cosm,
		Callbacks&&... callbacks
	);

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&& args...) {
		return on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&& args...) const {
		return on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}
};
