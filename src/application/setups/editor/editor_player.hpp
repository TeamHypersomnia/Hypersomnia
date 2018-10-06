#pragma once
#include "application/setups/editor/editor_player.h"
#include "application/intercosm.h"

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
						const auto& initial = self.before_start.value().work->world.get_solvable().significant;
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

template <class I>
auto editor_player::make_snapshotted_advance_input(const I& in) {
	auto step_callback = [&](const auto& applied_entropy) {
		on_mode_with_input(
			in.all_vars,
			in.cosm,
			[&](auto& typed_mode, const auto& in) {
				typed_mode.advance(
					in,
					applied_entropy,
					in.callbacks
				);
			}
		);
	};

	auto make_snapshot = [&](const auto n) -> editor_solvable_snapshot {
		if (n == 0) {
			return {};
		}
		
		augs::memory_stream ms;

		augs::write_bytes(ms, current_mode);
		augs::write_bytes(ms, in.cosm.get_solvable().significant);

		return std::move(ms);//ms.operator std::vector<std::byte>&&();
	};

	return augs::snapshotted_advance_input(
		total_collected_entropy,
		step_callback,
		make_snapshot
	);
}

template <class I>
auto editor_player::make_set_snapshot(const I& in) {
	return [&](const auto n, const auto& snapshot) {
		if (n == 0) {
			in.cosm = before_start.value().work->world;
		}

		auto ss = augs::cref_memory_stream(snapshot);

		augs::read_bytes(ss, current_mode);
		in.cosm.read_solvable_from(n, ss);
	};
}

template <class I>
void editor_player::seek_to(
	const editor_player::step_type step, 
	const I& in
) const {
	if constexpr(std::is_same_v<I, editor_folder>) {
		auto& all_vars = in.

		seek_to(
			step,
			player_advance_input(
				all_vars,
				in.work->world,
				solver_callbacks()
			)
		);
	}

	base::seek_to(
		step,
		make_snapshotted_advance_input(in),
		make_set_snapshot(in)
	);
}

template <class I>
void editor_player::advance_player(
	augs::delta frame_delta,
	const I& in
) {
	base::advance(
		make_snapshotted_advance_input(in),
		frame_delta,
		in.cosm.get_fixed_delta()
	);
}

template <class M>
void editor_player::choose_mode(const mode_vars_id& vars_id) {
	ensure(!has_testing_started());

	current_mode_vars_id = vars_id;
	current_mode.emplace<M>();
}
