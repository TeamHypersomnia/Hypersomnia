#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_player.hpp"
#include "application/intercosm.h"
#include "application/setups/editor/commands/editor_command_traits.h"
#include "application/setups/editor/editor_history.hpp"
#include "game/modes/all_mode_includes.h"

#include "augs/readwrite/byte_readwrite.h"

template <class C>
void make_redoable_for_different_solvable(
	const editor_command_input& in,
	C& command
) {
	auto sanitize = [&](auto& typed_command) {
		using T = remove_cref<decltype(typed_command)>;

		if constexpr(has_member_sanitize_v<T, editor_command_input>) {
			typed_command.sanitize(in);
		}
		else {
			static_assert(!has_affected_entities_v<T>);
		}

		typed_command.common.when_happened = 0;
	};

	std::visit(sanitize, command);
}

void editor_player::save_state_before_start(editor_folder& folder) {
	ensure(!has_testing_started());

	auto& current = folder.commanded;
	auto& backup = before_start.commanded;

	ensure(backup == nullptr);

	/* Move current to backup so it is left untouched */
	backup = std::move(current);

	/* Generate a clone for the current state */
	current = std::make_unique<editor_commanded_state>(*backup);

	before_start.revision = folder.history.get_current_revision();
}

void editor_player::restore_saved_state(editor_folder& folder) {
	ensure(has_testing_started());

	auto& backup = before_start.commanded;
	auto& current = folder.commanded;

	ensure(backup != nullptr);

	current = std::move(backup);
	ensure(backup == nullptr);

	before_start.revision = -1;
}

bool editor_player::is_editing_mode() const {
	return is_paused();
}

bool editor_player::has_testing_started() const {
	return before_start.commanded != nullptr;
}

void editor_player::control(const cosmic_entropy& entropy) {
	total_collected_entropy.cosmic += entropy;
}

void editor_player::control(const mode_entropy& entropy) {
	total_collected_entropy += entropy;
}

void editor_player::finish_testing(const editor_command_input in, const finish_testing_type mode) {
	if (!has_testing_started()) {
		return;
	}

	const auto start_revision = before_start.revision;

	auto& f = in.folder;
	auto& h = f.history;

	restore_saved_state(f);

	base::finish();
	total_collected_entropy.clear();

	h.force_set_current_revision(start_revision);

	if (mode == finish_testing_type::DISCARD_CHANGES) {
		h.discard_later_revisions();
	}
	else if (mode == finish_testing_type::REAPPLY_CHANGES) {
		while (h.has_next_command()) {
			::make_redoable_for_different_solvable(in, h.next_command());
			h.redo(in);
		}
	}
	else {
		ensure(false);
	}
}

#if !TODO_DELTA
augs::delta editor_player::get_chosen_delta() const {
	return before_start.commanded->work.world.get_fixed_delta();
}
#endif

double editor_player::get_current_secs() const {
	return base::get_current_step() * get_chosen_delta().in_seconds();
}

double editor_player::get_total_secs() const {
	return base::get_total_steps() * get_chosen_delta().in_seconds();
}

void editor_player::initialize_testing(editor_folder& f) {
	save_state_before_start(f);
	reset_mode();
}

void editor_player::begin_recording(editor_folder& f) {
	if (!has_testing_started()) {
		initialize_testing(f);
	}

	base::clear_later_entropies();
	base::begin_recording();
}

void editor_player::begin_replaying(editor_folder& f) {
	if (!has_testing_started()) {
		initialize_testing(f);
	}

	base::begin_replaying();
}

void editor_player::ensure_handler() {
	base::pause();

	/* 
		The editor step is not yet incremented,
	   	however the entropy for this step has already been recorded. 

		Therefore, we can leave it at that.
	*/
}

entity_guid editor_player::lookup_character(const mode_player_id id) const {
	return std::visit(
		[&](const auto& typed_mode) { 
			return typed_mode.lookup(id);
		},
		current_mode
	);
}

editor_player::revision_type editor_player::get_revision_when_started_testing() const {
	ensure(has_testing_started());

	return before_start.revision;
}

void editor_player::seek_to(
	const step_type step, 
	const editor_command_input in
) {
	seek_to(step, player_advance_input(in, solver_callbacks()));
}

void editor_player::reset_mode() {
	std::visit(
		[&](auto& typed_mode) {
			typed_mode = {};
		},
		current_mode
	);
}

editor_player::step_type editor_player::get_current_step() const {
	return base::get_current_step();
}

