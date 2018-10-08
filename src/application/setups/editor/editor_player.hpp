#pragma once
#include "application/setups/editor/editor_player.h"
#include "application/intercosm.h"
#include "augs/templates/snapshotted_player.hpp"

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
				using V = typename M::vars_type;
				using I = typename M::input;
				
				if (const auto vars = mapped_or_nullptr(all_vars.template get_for<V>(), self.current_mode_vars_id)) {
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

		[&](const auto& applied_entropy) {
			/* step */
			on_mode_with_input(
				folder.commanded->mode_vars,
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
							history.redo(in.cmd_in);
							continue;
						}
						
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

		[&](const auto n) -> editor_solvable_snapshot {
			/* make_snapshot */
			if (n == 0) {
				return {};
			}
			
			augs::memory_stream ms;

			augs::write_bytes(ms, current_mode);
			augs::write_bytes(ms, *folder.commanded);

			return std::move(ms);//ms.operator std::vector<std::byte>&&();
		}
	);
}

template <class C>
auto editor_player::make_set_snapshot(const player_advance_input_t<C> in) {
	auto& folder = in.cmd_in.folder;

	return [&](const auto n, const auto& snapshot) {
		if (n == 0) {
			current_mode = {};
			*folder.commanded = *before_start.commanded;
			return;
		}

		auto ss = augs::cref_memory_stream(snapshot);

		augs::read_bytes(ss, current_mode);
		augs::read_bytes(ss, *folder.commanded);
	};
}

template <class C>
void editor_player::seek_to(
	const editor_player::step_type step, 
	const player_advance_input_t<C> in
) {
	base::seek_to(
		step,
		make_snapshotted_advance_input(in),
		make_set_snapshot(in)
	);
}

template <class C>
void editor_player::advance_player(
	augs::delta frame_delta,
	const player_advance_input_t<C>& in
) {
	base::advance(
		make_snapshotted_advance_input(in),
		frame_delta,
		in.cmd_in.get_cosmos().get_fixed_delta()
	);
}

template <class M>
void editor_player::choose_mode(const mode_vars_id& vars_id) {
	ensure(!has_testing_started());

	current_mode_vars_id = vars_id;
	current_mode.emplace<M>();
}
