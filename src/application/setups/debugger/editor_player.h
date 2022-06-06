#pragma once
#include <memory>
#include <map>
#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_entropy.h"

#include "application/setups/debugger/editor_history.h"
#include "application/setups/debugger/editor_view.h"
#include "application/setups/debugger/editor_commanded_state.h"
#include "game/cosmos/entropy_recording_options.h"

#include "application/arena/mode_and_rules.h"
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

template <bool C, class ModeVariantType>
class basic_arena_handle;

template <bool C>
using editor_arena_handle = basic_arena_handle<C, mode_and_rules>;

class editor_player : public editor_player_base {
	using base = editor_player_base;

public:
	using introspect_base = base;

private:
	using step_type = base::step_type;

	// GEN INTROSPECTOR class editor_player
	mode_and_rules current_mode;
	player_before_start_state before_start;

public:
	cosmic_entropy_recording_options cosmic_recording_options;
	mode_entropy_recording_options mode_recording_options;

	bool dirty = false;
private:
	// END GEN INTROSPECTOR

	friend augs::introspection_access;
	friend struct editor_property_accessors;

	template <class H, class S, class E>
	static decltype(auto) get_arena_handle_impl(S& self, E& folder) {
		return H {
			self.current_mode,
			folder.commanded->work,
			folder.commanded->work.world,
			folder.commanded->rulesets,
			self.before_start.commanded->work.world.get_solvable().significant
		};
	}

	void save_state_before_start(editor_folder&);
	void restore_saved_state(editor_folder&);

	void initialize_testing(editor_folder&);

	template <class C, class E>
	auto make_snapshotted_advance_input(player_advance_input_t<C> input, E&& extract_collected_entropy);

	template <class C>
	auto make_load_snapshot(player_advance_input_t<C> input);

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

	mode_entity_id lookup_character(mode_player_id) const;

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
		return current_mode.state;
	}

	void set_dirty();

	void choose_mode(const ruleset_id& id);
	void read_live_entropies(const augs::path_type& from);

	double get_audiovisual_speed(const editor_folder&) const;

	void force_add_bots_if_quota_zero(editor_folder&);

	std::size_t estimate_step_to_entropy_size() const;
	const step_to_entropy_type& get_step_to_entropy() const;

	editor_arena_handle<false> get_arena_handle(editor_folder&);
	editor_arena_handle<true> get_arena_handle(const editor_folder&) const;
};
