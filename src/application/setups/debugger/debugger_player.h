#pragma once
#include <cstddef>
#include <memory>
#include <map>
#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_entropy.h"

#include "application/setups/debugger/debugger_history.h"
#include "application/setups/debugger/debugger_view.h"
#include "application/setups/debugger/debugger_commanded_state.h"
#include "game/cosmos/entropy_recording_options.h"

#include "application/network/network_common.h"
#include "augs/templates/snapshotted_player.h"

struct cosmos_solvable_significant;

struct debugger_folder;
struct debugger_command_input;

namespace augs {
	struct introspection_access;
}

struct intercosm;

struct player_before_start_state {
	using revision_type = debugger_history::index_type;

	// GEN INTROSPECTOR struct player_before_start_state
	std::unique_ptr<debugger_commanded_state> commanded;
	debugger_history history;
	// END GEN INTROSPECTOR
};

enum class finish_testing_type {
	REAPPLY_CHANGES,
	DISCARD_CHANGES
};

using debugger_solvable_snapshot = std::vector<std::byte>;
using debugger_player_entropy_type = mode_entropy;
using debugger_player_base = augs::snapshotted_player<
	debugger_player_entropy_type,
	debugger_solvable_snapshot
>;

template <class C>
struct player_advance_input_t {
	debugger_command_input cmd_in;
	C callbacks;
};

template <class C>
auto player_advance_input(
	const debugger_command_input& in,
	const C& callbacks
) {
	return player_advance_input_t<C> { in, callbacks };
}

template <bool C>
using debugger_arena_handle = online_arena_handle<C>;

class debugger_player : public debugger_player_base {
	using base = debugger_player_base;

public:
	using introspect_base = base;

private:
	using step_type = base::step_type;

	// GEN INTROSPECTOR class debugger_player
	all_modes_variant current_mode_state;
	player_before_start_state before_start;

public:
	cosmic_entropy_recording_options cosmic_recording_options;
	mode_entropy_recording_options mode_recording_options;

	bool dirty = false;
private:
	// END GEN INTROSPECTOR

	friend augs::introspection_access;
	friend struct debugger_property_accessors;

	template <class H, class S, class E>
	static decltype(auto) get_arena_handle_impl(S& self, E& folder) {
		return H {
			self.current_mode_state,
			folder.commanded->work,
			folder.commanded->work.world,
			folder.commanded->rulesets,
			self.before_start.commanded->work.world.get_solvable().significant
		};
	}

	void save_state_before_start(debugger_folder&);
	void restore_saved_state(debugger_folder&);

	void initialize_testing(debugger_folder&);

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

	void adjust_entropy(const debugger_folder&, debugger_player_entropy_type&, bool neg) const;

	using base::request_steps;

	using base::get_current_step;
	using base::get_total_steps;

public:
	bool is_editing_mode() const;
	bool has_testing_started() const;

	void finish_testing(debugger_command_input, finish_testing_type);

	void pause();
	void begin_recording(debugger_folder&);
	void begin_replaying(debugger_folder&);

	void ensure_handler();

	template <class C, class E>
	void advance_player(
		augs::delta frame_delta,
		const player_advance_input_t<C>& input,
		E&& extract_collected_entropy
	);

	mode_entity_id lookup_character(mode_player_id) const;

	using revision_type = debugger_history::index_type;

	double get_current_secs() const;
	double get_total_secs(const debugger_folder&) const;

	step_type get_current_step() const;
	step_type get_total_steps(const debugger_folder&) const;

	void seek_to(
		step_type, 
		debugger_command_input input,
		bool trim_to_total_steps = true
	);

	void seek_backward(
		step_type, 
		debugger_command_input input
	);

	void request_steps(step_type amount);

	const auto& get_before_start() const {
		return before_start;
	}

	const auto& get_current_mode() const {
		return current_mode_state;
	}

	void set_dirty();

	void read_live_entropies(const augs::path_type& from);

	double get_audiovisual_speed(const debugger_folder&) const;

	void force_add_bots_if_quota_zero(debugger_folder&);

	std::size_t estimate_step_to_entropy_size() const;
	const step_to_entropy_type& get_step_to_entropy() const;

	debugger_arena_handle<false> get_arena_handle(debugger_folder&);
	debugger_arena_handle<true> get_arena_handle(const debugger_folder&) const;
};
