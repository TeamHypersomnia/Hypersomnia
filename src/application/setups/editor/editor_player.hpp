#pragma once
#include "application/setups/editor/editor_player.h"
#include "application/intercosm.h"
#include "augs/templates/snapshotted_player.hpp"
#include "application/setups/editor/editor_settings.h"

template <class E, class A, class C, class F>
decltype(auto) editor_player::on_mode_with_input_impl(
	E& self,
	const A& all_vars,
	C& cosm,
	F&& callback
) {
	if (self.has_testing_started()) {
		std::visit(
			[&](auto& typed_mode) {
				using M = remove_cref<decltype(typed_mode)>;
				using I = typename M::input;
				
				if (const auto vars = mapped_or_nullptr(all_vars.template get_for<M>(), self.current_mode_vars_id)) {
					if constexpr(M::needs_initial_signi) {
						const auto& initial = self.before_start.commanded->work.world.get_solvable().significant;
						const auto in = I { *vars, initial, cosm };

						callback(typed_mode, in);
					}
					else {
						const auto in = I { *vars, cosm };
						callback(typed_mode, in);
					}
				}
			},
			self.current_mode
		);
	}
}

template <class C>
auto editor_player::make_snapshotted_advance_input(const player_advance_input_t<C> in) {
	auto& folder = in.cmd_in.folder;
	auto& history = folder.history;
	auto& cosm = folder.commanded->work.world;

	return augs::snapshotted_advance_input(
		total_collected_entropy,

		[this, &folder, &history, &cosm, in](const auto& applied_entropy) {
			/* step */
			on_mode_with_input(
				folder.commanded->mode_vars.vars,
				cosm,
				[&](auto& typed_mode, const auto& mode_in) {
					while (history.has_next_command()) {
						const auto when_happened = std::visit(
							[&](const auto& typed_command) {
								return typed_command.common.when_happened;
							},
							history.next_command()
						);

						const auto current_step = get_current_step();

						ensure_leq(current_step, when_happened);

						if (current_step == when_happened) {
							PLR_LOG("Plain redo of a command at %x", current_step);
							history.editor_history_base::redo(in.cmd_in);
							continue;
						}
						
						PLR_LOG("Next happens at %x, now at %x, so breaking", when_happened, current_step);

						break;
					}

					typed_mode.advance(
						mode_in,
						applied_entropy,
						in.callbacks
					);
				}
			);
		},

		[this, &folder, &history](const auto n) -> editor_solvable_snapshot {
			/* make_snapshot */
			
			cosmic::reinfer_solvable(folder.commanded->work.world);

			if constexpr(is_nullopt_v<decltype(n)>) {
				return {};
			}
			else {
				/* make_snapshot */
				if (n == 0) {
					return {};
				}
				
				augs::memory_stream ms;

				augs::write_bytes(ms, history.get_current_revision());

				augs::write_bytes(ms, current_mode);
				augs::write_bytes(ms, *folder.commanded);

				return std::move(ms);//ms.operator std::vector<std::byte>&&();
			}
		},

		in.cmd_in.settings.player
	);
}

template <class C>
auto editor_player::make_set_snapshot(const player_advance_input_t<C> in) {
	auto& folder = in.cmd_in.folder;
	auto& history = folder.history;

	return [&](const auto n, const auto& snapshot) {
		if (n == 0) {
			reset_mode();
			*folder.commanded = *before_start.commanded;
			history.force_set_current_revision(history.get_first_revision());
			return;
		}

		auto ss = augs::cref_memory_stream(snapshot);

		{
			decltype(history.get_current_revision()) revision;
			augs::read_bytes(ss, revision);

			history.force_set_current_revision(revision);
		}

		augs::read_bytes(ss, current_mode);
		augs::read_bytes(ss, *folder.commanded);
	};
}

template <class C>
void editor_player::seek_to(
	const editor_player::step_type step, 
	const player_advance_input_t<C> in
) {
	if (step == get_current_step()) {
		return;
	}

	set_dirty();

	base::seek_to(
		step,
		make_snapshotted_advance_input(in),
		make_set_snapshot(in)
	);

	in.cmd_in.clear_dead_entities();
}

template <class C>
void editor_player::advance_player(
	augs::delta frame_delta,
	const player_advance_input_t<C>& in
) {
	const auto performed_steps = base::advance(
		make_snapshotted_advance_input(in),
		frame_delta,
		in.cmd_in.get_cosmos().get_fixed_delta()
	);

	if (performed_steps) {
		set_dirty();
		in.cmd_in.clear_dead_entities();
	}
}
