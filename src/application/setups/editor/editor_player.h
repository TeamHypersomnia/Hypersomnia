#pragma once
#include <memory>
#include <map>
#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_entropy.h"

#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_view.h"
#include "application/setups/editor/editor_commanded_state.h"
#include "game/cosmos/entropy_recording_options.h"

#include "augs/templates/snapshotted_player.h"

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
	std::unique_ptr<editor_commanded_state> commanded;
	editor_history history;
	// END GEN INTROSPECTOR
};

enum class finish_testing_type {
	REAPPLY_CHANGES,
	DISCARD_CHANGES
};

using editor_solvable_snapshot = std::vector<std::byte>;
using editor_player_entropy_type = mode_entropy;
using editor_player_base = augs::snapshotted_player<
	editor_player_entropy_type,
	editor_solvable_snapshot
>;

template <class C>
struct player_advance_input_t {
	editor_command_input cmd_in;
	C callbacks;
};

template <class C>
auto player_advance_input(
	const editor_command_input& in,
	const C& callbacks
) {
	return player_advance_input_t<C> { in, callbacks };
}

class editor_player : public editor_player_base {
	using base = editor_player_base;

public:
	using introspect_base = base;

private:
	using step_type = base::step_type;

	// GEN INTROSPECTOR class editor_player
	all_modes_variant current_mode;
	raw_ruleset_id current_mode_rules_id = raw_ruleset_id();

	player_before_start_state before_start;

public:
	cosmic_entropy_recording_options cosmic_recording_options;
	mode_entropy_recording_options mode_recording_options;

	bool dirty = false;
private:
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

	template <class C, class E>
	auto make_snapshotted_advance_input(player_advance_input_t<C> input, E&& extract_collected_entropy);

	template <class C>
	auto make_set_snapshot(player_advance_input_t<C> input);

	augs::delta get_chosen_delta() const;

	void reset_mode();

	template <class C>
	void seek_to(
		step_type, 
		player_advance_input_t<C> input,
		bool trim_to_total_steps = true
	);

	void adjust_entropy(const editor_folder&, editor_player_entropy_type&, bool neg) const;

	using base::request_steps;

	using base::get_current_step;
	using base::get_total_steps;

public:
	bool is_editing_mode() const;
	bool has_testing_started() const;

	void finish_testing(editor_command_input, finish_testing_type);

	void pause();
	void begin_recording(editor_folder&);
	void begin_replaying(editor_folder&);

	void ensure_handler();

	template <class C, class E>
	void advance_player(
		augs::delta frame_delta,
		const player_advance_input_t<C>& input,
		E&& extract_collected_entropy
	);

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) {
		return on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) const {
		return on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	entity_guid lookup_character(mode_player_id) const;

	using revision_type = editor_history::index_type;

	double get_current_secs() const;
	double get_total_secs(const editor_folder&) const;

	step_type get_current_step() const;
	step_type get_total_steps(const editor_folder&) const;

	void seek_to(
		step_type, 
		editor_command_input input,
		bool trim_to_total_steps = true
	);

	void seek_backward(
		step_type, 
		editor_command_input input
	);

	void request_steps(step_type amount);

	const auto& get_before_start() const {
		return before_start;
	}

	const auto& get_current_mode() const {
		return current_mode;
	}

	void set_dirty();

	void choose_mode(const ruleset_id& id);
	void read_live_entropies(const augs::path_type& from);

	double get_audiovisual_speed(const editor_folder&) const;
};
