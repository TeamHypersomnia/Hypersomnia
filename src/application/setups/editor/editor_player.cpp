#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_player.hpp"
#include "application/intercosm.h"
#include "application/setups/editor/commands/editor_command_traits.h"
#include "application/setups/editor/editor_history.hpp"

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
	};

	std::visit(sanitize, command);
}

void editor_player::save_state_before_start(editor_folder& folder) {
	LOG("Save state");
	ensure(!before_start.has_value());
	before_start.emplace();

	auto& backup = *before_start;

	backup.work = std::move(folder.work);
	folder.work = std::make_unique<intercosm>(*backup.work);

	backup.view_ids = folder.view.ids;
	backup.mode_vars = folder.mode_vars;
	backup.revision = folder.history.get_current_revision();
}

void editor_player::restore_saved_state(editor_folder& folder) {
	LOG("Restore state");
	ensure(before_start.has_value());
	auto& backup = *before_start;

	folder.work = std::move(backup.work);
	folder.view.ids = std::move(backup.view_ids);
	folder.mode_vars = std::move(backup.mode_vars);

	before_start.reset();
}

double editor_player::get_speed() const {
	return paused ? 0.0 : speed;
}

void editor_player::set_speed(const double new_speed) {
	speed = new_speed;
}

bool editor_player::is_editing_mode() const {
	return paused;
}

bool editor_player::has_testing_started() const {
	return before_start.has_value();
}

void editor_player::control(const cosmic_entropy& entropy) {
	total_collected_entropy.cosmic += entropy;
}

void editor_player::control(const mode_entropy& entropy) {
	total_collected_entropy += entropy;
}

void editor_player::request_additional_step() {
	if (has_testing_started()) {
		++additional_steps;
	}
}

void editor_player::finish_testing(const editor_command_input in, const finish_testing_type mode) {
	if (!has_testing_started()) {
		return;
	}

	const auto start_revision = before_start->revision;

	auto& f = in.folder;
	auto& h = f.history;

	restore_saved_state(f);

	paused = true;
	current_step = 0;

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

void editor_player::initialize_testing(editor_folder& f) {
	begin_recording();
	save_state_before_start(f);

	current_step = 0;
	step_to_entropy.clear();
	total_collected_entropy.clear();

	std::visit(
		[&](auto& typed_mode) {
			typed_mode = {};
		},
		current_mode
	);
}

void editor_player::start_resume(editor_folder& f) {
	if (!has_testing_started()) {
		initialize_testing(f);
	}

	paused = false;
}

void editor_player::pause() {
	paused = true;
}

void editor_player::start_pause_resume(editor_folder& f) {
	if (paused) {
		start_resume(f);
	}
	else {
		paused = true;
	}
}

void editor_player::begin_replaying() {
	advance_mode = advance_type::REPLAYING;
}

void editor_player::begin_recording() {
	advance_mode = advance_type::RECORDING;
}

void editor_player::ensure_handler() {
	/* So that we don't accidentally overwrite the repro: */
	begin_replaying();
	pause();

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

	return before_start.value().revision;
}
