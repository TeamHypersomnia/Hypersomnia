#pragma once
#include <memory>
#include <map>
#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_entropy.h"

#include "application/setups/editor/player/editor_player_step_type.h"
#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_view.h"

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

using editor_solvable_snapshot = std::vector<std::byte>;
using editor_player_entropy_type = mode_entropy;
using editor_player_base = augs::snapshotted_player<
	editor_player_entropy_type,
	editor_solvable_snapshot
>;

template <class A, class C>
struct player_advance_input_t {
	const A& all_vars;
	cosmos& cosm;
	const C& callbacks;
};

template <class A, class C>
auto player_advance_input(
	const A& all_vars,
	cosmos& cosm,
	const C& callbacks
) {
	return player_advance_input_t<A, C> {
		all_vars,
		cosm,
		callbacks
	};
}

class editor_player : public editor_player_base {
	using base = editor_player_base;
	using introspect_base = base;
	using step_type = base::step_type;

	// GEN INTROSPECTOR class editor_player
	editor_player_entropy_type total_collected_entropy;

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

	template <class I>
	void advance_single_step(const I& input);

	template <class I>
	auto make_snapshotted_advance_input(const I& input);

	template <class I>
	auto make_set_snapshot(const I& input);

public:

	bool is_editing_mode() const;
	bool has_testing_started() const;

	void control(const cosmic_entropy& entropy);
	void control(const mode_entropy& entropy);

	void finish_testing(editor_command_input, finish_testing_type);

	void start_resume(editor_folder&);
	void start_pause_resume(editor_folder&);

	template <class M>
	void choose_mode(const mode_vars_id& vars_id);

	void ensure_handler();

	template <class I>
	void advance_player(
		augs::delta frame_delta,
		const I& input
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

	revision_type get_revision_when_started_testing() const;

	double get_current_secs() const;
	double get_total_secs() const;

	template <class I>
	void seek_to(
		step_type, 
		const I& input
	) const;
};
