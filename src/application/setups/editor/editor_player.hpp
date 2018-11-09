#pragma once
#include "application/setups/editor/editor_player.h"
#include "application/intercosm.h"
#include "augs/templates/snapshotted_player.hpp"
#include "application/setups/editor/detail/on_mode_with_input.hpp"
#include "application/setups/editor/editor_settings.h"

template <class C, class E>
auto editor_player::make_snapshotted_advance_input(const player_advance_input_t<C> in, E&& extract_collected_entropy) {
	auto& folder = in.cmd_in.folder;
	auto& history = folder.history;
	auto& cosm = folder.commanded->work.world;

	return augs::snapshotted_advance_input(
		[this, &folder, &history, &cosm, in](const auto& applied_entropy) {
			/* step */
			on_mode_with_input(
				folder.commanded->rulesets.all,
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
						
						//	PLR_LOG("Next happens at %x, now at %x, so breaking", when_happened, current_step);

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

		[&](auto& existing_entropy) {
			/* record_entropy */
			adjust_entropy(folder, existing_entropy, false);

			auto extracted = extract_collected_entropy();
			adjust_entropy(folder, extracted, true);

			existing_entropy += extracted;
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

template <class C, class E>
void editor_player::advance_player(
	augs::delta frame_delta,
	const player_advance_input_t<C>& in,
	E&& extract_collected_entropy
) {
	const auto performed_steps = base::advance(
		make_snapshotted_advance_input(in, std::forward<E>(extract_collected_entropy)),
		frame_delta,
		in.cmd_in.get_cosmos().get_fixed_delta()
	);

	if (performed_steps) {
		set_dirty();
		in.cmd_in.clear_dead_entities();
	}
}
