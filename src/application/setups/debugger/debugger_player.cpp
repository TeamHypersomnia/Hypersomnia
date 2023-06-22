#include "application/setups/debugger/debugger_player.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/setups/debugger/debugger_player.hpp"
#include "application/intercosm.h"
#include "application/setups/debugger/commands/debugger_command_traits.h"
#include "application/setups/debugger/debugger_history.hpp"
#include "game/modes/all_mode_includes.h"

#include "augs/readwrite/byte_readwrite.h"

template <class C>
void make_redoable_for_different_solvable(
	const debugger_command_input& in,
	C& command
) {
	auto sanitize = [&](auto& typed_command) {
		using T = remove_cref<decltype(typed_command)>;

		if constexpr(has_member_sanitize_v<T, debugger_command_input>) {
			typed_command.sanitize(in);
		}
		else {
			static_assert(!has_affected_entities_v<T>);
		}

		typed_command.common.when_happened = 0;
	};

	std::visit(sanitize, command);
}

void debugger_player::force_add_bots_if_quota_zero(debugger_folder& folder) {
	/* So that we have something to play with */
	auto& current = folder.commanded;
	const auto playtest_default = current->rulesets.meta.playtest_default;

	playtest_default.type_id.dispatch([&](auto e) {
		using E = decltype(e);

		if constexpr(std::is_same_v<arena_mode, E>) {
			auto& rs = current->rulesets.all.get_for<E>()[playtest_default.raw];

			if (rs.bot_quota == 0) {
				rs.bot_quota = rs.bot_names.size();
			}
		}
	});
}

void debugger_player::save_state_before_start(debugger_folder& folder) {
	ensure(!has_testing_started());

	auto& current = folder.commanded;
	auto& backup = before_start.commanded;

	ensure(backup == nullptr);

	/* Move current to backup so it is left untouched */
	backup = std::move(current);

	/* Generate a clone for the current state */
	current = std::make_unique<debugger_commanded_state>(*backup);

	before_start.history = std::move(folder.history);
	folder.history = {};
}

void debugger_player::restore_saved_state(debugger_folder& folder) {
	ensure(has_testing_started());

	auto& backup = before_start.commanded;
	auto& current = folder.commanded;

	ensure(backup != nullptr);

	current = std::move(backup);
	ensure(backup == nullptr);

	folder.history = std::move(before_start.history);
	before_start.history = {};
}

bool debugger_player::is_editing_mode() const {
	return is_paused();
}

bool debugger_player::has_testing_started() const {
	return before_start.commanded != nullptr;
}

void debugger_player::finish_testing(const debugger_command_input in, const finish_testing_type mode) {
	if (!has_testing_started()) {
		return;
	}

	auto& f = in.folder;
	auto playtested_history = debugger_history(std::move(f.history));

	restore_saved_state(f);

	f.view.overridden_viewed = {};

	base::finish();

	dirty = false;

	/* Forces autosave in the case that we returned to a saved revision */
	f.history.set_dirty_flag();

	if (mode == finish_testing_type::DISCARD_CHANGES) {

	}
	else if (mode == finish_testing_type::REAPPLY_CHANGES) {
		auto& current_history = f.history;
		current_history.discard_later_revisions();

		using R = debugger_history::index_type;

		auto& commands = playtested_history.get_commands();

		for (R i = 0; i <= playtested_history.get_current_revision(); ++i) {
			auto& cmd = commands[i];

			::make_redoable_for_different_solvable(in, cmd);

			std::visit(
				[&](auto& typed_cmd) {
					using T = remove_cref<decltype(typed_cmd)>;

					if constexpr(!is_playtest_specific_v<T>) {
						current_history.execute_new(std::move(typed_cmd), in);
					}
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
augs::delta debugger_player::get_chosen_delta() const {
	return before_start.commanded->work.world.get_fixed_delta();
}
#endif

double debugger_player::get_current_secs() const {
	return base::get_current_step() * get_chosen_delta().in_seconds();
}

double debugger_player::get_total_secs(const debugger_folder& folder) const {
	return get_total_steps(folder) * get_chosen_delta().in_seconds();
}

void debugger_player::initialize_testing(debugger_folder& f) {
	//choose_mode(f.commanded->rulesets.meta.playtest_default);

	save_state_before_start(f);
	reset_mode();

	force_add_bots_if_quota_zero(f);
}

void debugger_player::begin_recording(debugger_folder& f) {
	auto& h = f.history;

	if (!has_testing_started()) {
		if (h.on_first_revision()) {
			return;
		}

		initialize_testing(f);
	}

#if 0
	base::for_each_later_entropy(
		[&](auto& it) {
			LOG_NVPS(it.first);
			adjust_entropy(f, it.second, false);
		}
	);
#endif

	base::begin_recording();

	h.discard_later_revisions();
	set_dirty();
}

void debugger_player::begin_replaying(debugger_folder& f) {
	auto& h = f.history;

	if (!has_testing_started()) {
		if (h.on_first_revision()) {
			return;
		}

		initialize_testing(f);
	}

	base::begin_replaying();

	set_dirty();
}

void debugger_player::ensure_handler() {
	base::pause();

	/* 
		The editor step is not yet incremented,
	   	however the entropy for this step has already been recorded. 

		Therefore, we can leave it at that.
	*/
}

mode_entity_id debugger_player::lookup_character(const mode_player_id id) const {
	return std::visit(
		[&](const auto& typed_mode) { 
			return typed_mode.lookup(id);
		},
		current_mode_state
	);
}

void snap_interpolated_to_logical(cosmos&);

template <class C>
void debugger_player::seek_to(
	const debugger_player::step_type requested_step, 
	const player_advance_input_t<C> in,
	const bool trim_to_total_steps
) {
	const auto seeked_step = 
		trim_to_total_steps 
		? std::min(requested_step, get_total_steps(in.cmd_in.folder))
		: requested_step
	;

	if (seeked_step == get_current_step()) {
		return;
	}

	set_dirty();

	base::seek_to(
		seeked_step,
		make_snapshotted_advance_input(in, []() { return debugger_player_entropy_type(); }),
		make_load_snapshot(in)
	);

	in.cmd_in.clear_dead_entities();
	snap_interpolated_to_logical(in.cmd_in.get_cosmos());
}

void debugger_player::seek_to(
	const step_type step, 
	const debugger_command_input cmd_in,
	const bool trim_to_total_steps
) {
	seek_to(step, player_advance_input(cmd_in, solver_callbacks()), trim_to_total_steps);
}

void debugger_player::seek_backward(
	const step_type offset, 
	const debugger_command_input cmd_in
) {
	const auto current = get_current_step();
	const auto target = current >= offset ? current - offset : 0;

	seek_to(target, cmd_in, false);
}

void debugger_player::reset_mode() {
	set_dirty();

	std::visit(
		[&](auto& typed_mode) {
			typed_mode = {};
		},
		current_mode_state
	);
}

debugger_player::step_type debugger_player::get_current_step() const {
	return base::get_current_step();
}

debugger_player::step_type debugger_player::get_total_steps(const debugger_folder& f) const {
	const auto of_last_command = [&]() -> debugger_player::step_type {
		const auto& h = f.history;
		const auto& cmds = h.get_commands();

		const auto rev = h.get_last_revision();

		if (rev == h.get_first_revision()) {
			return 0;
		}

		return std::visit(
			[](const auto& typed_cmd) {
				return typed_cmd.common.when_happened;
			},
			cmds[rev]
		);
	}();
		
	const auto of_entropies = base::get_total_steps();

	return std::max(of_last_command, of_entropies);
}

void debugger_player::request_steps(const debugger_player::step_type amount) {
	if (has_testing_started()) {
		set_dirty();
		base::request_steps(amount);
	}
}

void debugger_player::set_dirty() {
	dirty = true;
}

void debugger_player::adjust_entropy(const debugger_folder& folder, debugger_player_entropy_type& entropy, const bool neg) const {
	const auto local_player_id = folder.view.local_player_id;

	if (auto p = mapped_or_nullptr(entropy.players, local_player_id)) {
		auto f = mode_recording_options;

		if (neg) {
			f.neg();
		}

		if (f.overwrite) {
			*p = {};
		}
	}

	const auto currently_viewed = folder.get_controlled_character_id();

	if (auto p = mapped_or_nullptr(entropy.cosmic.players, currently_viewed)) {
		auto opts = cosmic_recording_options;

		if (neg) {
			opts.neg();
		}

		p->commands.clear_relevant(opts);
	}
}

void debugger_player::pause() {
	if (!is_paused()) {
		set_dirty();
	}

	base::pause();
}

void debugger_player::read_live_entropies(const augs::path_type& from) {
	base::read_live_entropies(from);
}

std::size_t debugger_player::estimate_step_to_entropy_size() const {
	augs::byte_counter_stream counter_stream;
	augs::write_bytes(counter_stream, step_to_entropy);

	return counter_stream.size();
}

const debugger_player::step_to_entropy_type& debugger_player::get_step_to_entropy() const {
	return step_to_entropy;
}

double debugger_player::get_audiovisual_speed(const debugger_folder& f) const {
	if (has_testing_started()) {
		return get_speed() * get_arena_handle(f).get_audiovisual_speed();
	}

	return 0.0;
}

debugger_arena_handle<false> debugger_player::get_arena_handle(debugger_folder& folder) {
	return get_arena_handle_impl<debugger_arena_handle<false>>(*this, folder);
}

debugger_arena_handle<true> debugger_player::get_arena_handle(const debugger_folder& folder) const {
	return get_arena_handle_impl<debugger_arena_handle<true>>(*this, folder);
}

template class augs::snapshotted_player<
	debugger_player_entropy_type,
	debugger_solvable_snapshot
>;
