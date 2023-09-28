#pragma once
#include "application/setups/debugger/debugger_player.h"
#include "application/intercosm.h"
#include "augs/templates/snapshotted_player.hpp"
#include "application/setups/debugger/debugger_settings.h"
#include "application/setups/debugger/debugger_paths.h"
#include "augs/templates/traits/is_nullopt.h"

#include "application/arena/arena_handle.h"

template <class C, class E>
auto debugger_player::make_snapshotted_advance_input(const player_advance_input_t<C> in, E&& extract_collected_entropy) {
	auto& folder = in.cmd_in.folder;
	auto& history = folder.history;
	auto& settings = in.cmd_in.settings;

	auto step = [this, &folder, &history, in](const auto& applied_entropy) {
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
				history.debugger_history_base::redo(in.cmd_in);
				continue;
			}
			
			//	PLR_LOG("Next happens at %x, now at %x, so breaking", when_happened, current_step);

			break;
		}

		get_arena_handle(folder).advance(applied_entropy, in.callbacks, solve_settings());
	};

	auto record_entropy = [&](auto& existing_entropy) {
		adjust_entropy(folder, existing_entropy, false);

		auto extracted = extract_collected_entropy();
		adjust_entropy(folder, extracted, true);

		existing_entropy += extracted;

		if (settings.save_entropies_to_live_file) {
			const auto current_step = get_current_step();
			const auto paths = folder.get_paths();
			const auto live_file_path = paths.entropies_live_file;

			auto s = augs::open_binary_output_stream_append(live_file_path);

			augs::write_bytes(s, current_step);
			augs::write_bytes(s, existing_entropy);
		}
	};

	auto generate_snapshot = [this, &folder, &history](const auto n) -> debugger_solvable_snapshot {
		cosmic::reinfer_solvable(folder.commanded->work.world);

		if constexpr(is_nullopt_v<decltype(n)>) {
			return {};
		}
		else {
			if (n == 0) {
				return {};
			}
			
			augs::memory_stream ms;

			augs::write_bytes(ms, history.get_current_revision());

			augs::write_bytes(ms, current_mode_state);
			augs::write_bytes(ms, *folder.commanded);

			return std::move(ms);//ms.operator std::vector<std::byte>&&();
		}
	};

	return augs::snapshotted_advance_input(
		std::move(step),
		std::move(record_entropy),
		std::move(generate_snapshot),
		in.cmd_in.settings.player
	);
}

template <class C>
auto debugger_player::make_load_snapshot(const player_advance_input_t<C> in) {
	auto& folder = in.cmd_in.folder;
	auto& history = folder.history;

	return [&](const auto n, const auto& snapshot) {
		if (n == 0) {
			reset_mode();
			*folder.commanded = *before_start.commanded;
			force_add_bots_if_quota_zero(folder);
			history.force_set_current_revision(history.get_first_revision());
			return;
		}

		auto ss = augs::cref_memory_stream(snapshot);

		{
			decltype(history.get_current_revision()) revision;
			augs::read_bytes(ss, revision);

			history.force_set_current_revision(revision);
		}

		augs::read_bytes(ss, current_mode_state);
		augs::read_bytes(ss, *folder.commanded);
	};
}

template <class C, class E>
void debugger_player::advance_player(
	augs::delta frame_delta,
	const player_advance_input_t<C>& in,
	E&& extract_collected_entropy
) {
	const auto performed_steps = base::advance(
		make_snapshotted_advance_input(in, std::forward<E>(extract_collected_entropy)),
		frame_delta,
		in.cmd_in.folder.get_inv_tickrate()
	);

	if (performed_steps) {
		set_dirty();
		in.cmd_in.clear_dead_entities();
	}
}
