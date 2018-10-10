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

	before_start.history = std::move(folder.history);
	folder.history = {};
}

void editor_player::restore_saved_state(editor_folder& folder) {
	ensure(has_testing_started());

	auto& backup = before_start.commanded;
	auto& current = folder.commanded;

	ensure(backup != nullptr);

	current = std::move(backup);
	ensure(backup == nullptr);

	folder.history = std::move(before_start.history);
	before_start.history = {};
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

	auto& f = in.folder;
	auto playtested_history = editor_history(std::move(f.history));

	restore_saved_state(f);

	base::finish();
	total_collected_entropy.clear();

	set_dirty();

	if (mode == finish_testing_type::DISCARD_CHANGES) {

	}
	else if (mode == finish_testing_type::REAPPLY_CHANGES) {
		auto& current_history = f.history;
		current_history.discard_later_revisions();

		using R = editor_history::index_type;

		auto& commands = playtested_history.get_commands();

		for (R i = 0; i <= playtested_history.get_current_revision(); ++i) {
			auto& cmd = commands[i];

			::make_redoable_for_different_solvable(in, cmd);

			std::visit(
				[&](auto& typed_cmd) {
					current_history.execute_new(std::move(typed_cmd), in);
				},
				cmd
			);
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

	set_dirty();
}

void editor_player::begin_replaying(editor_folder& f) {
	if (!has_testing_started()) {
		initialize_testing(f);
	}

	base::begin_replaying();

	set_dirty();
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

void editor_player::seek_to(
	const step_type step, 
	const editor_command_input in
) {
	seek_to(step, player_advance_input(in, solver_callbacks()));
}

void editor_player::reset_mode() {
	set_dirty();

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


void editor_player::request_steps(const int amount) {
	if (has_testing_started()) {
		set_dirty();
		base::request_steps(amount);
	}
}

void editor_player::set_dirty() {
	dirty = true;
}
