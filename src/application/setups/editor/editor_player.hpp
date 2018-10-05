#pragma once
#include "application/setups/editor/editor_player.h"
#include "game/cosmos/cosmos.h"

template <class E, class A, class C, class F>
decltype(auto) editor_player::on_mode_with_input_impl(
	E& self,
	const A& all_vars,
	C& cosm,
	F&& callback
) {
	return std::visit(
		[&](auto& typed_mode) {
			using M = remove_cref<decltype(typed_mode)>;
			using V = typename M::vars_type;
			using I = typename M::input;
			
			if (const auto vars = mapped_or_nullptr(all_vars.template get_for<V>(), self.current_mode_vars_id)) {
				if constexpr(M::needs_initial_signi) {
					const auto& initial = self.mode_initial_signi;
					const auto& chosen_initial = 
						initial != nullptr ? 
						*initial :
						cosm.get_solvable().significant
					;

					callback(typed_mode, I{ *vars, chosen_initial, cosm });
				}
				else {
					callback(typed_mode, I{ *vars, cosm });
				}
			}
		},
		self.current_mode
	);
}

template <class A, class... Callbacks>
void editor_player::advance_player(
	augs::delta frame_delta,
	const A& all_vars,
	cosmos& cosm,
	Callbacks&&... callbacks
) {
	auto& self = *this;

	if (!self.paused) {
		self.timer.advance(frame_delta *= self.speed);
	}

	auto steps = self.additional_steps + self.timer.extract_num_of_logic_steps(cosm.get_fixed_delta());

	if (self.has_testing_started()) {
		on_mode_with_input(
			all_vars,
			cosm,
			[&](auto& typed_mode, const auto& in) {
				while (steps--) {
					auto& applied_entropy = self.total_collected_entropy;
					auto& step_i = self.current_step;

					applied_entropy.clear_dead_entities(cosm);

					switch (self.advance_mode) {
						case advance_type::REPLAYING:
							if (const auto found_entropy = mapped_or_nullptr(step_to_entropy, step_i)) {
								applied_entropy = *found_entropy;
							}
							
						case advance_type::RECORDING:
							if (!applied_entropy.empty()) {
								step_to_entropy[step_i] = applied_entropy;
							}

						default: break;
					}

					typed_mode.advance(
						in,
						applied_entropy,
						std::forward<Callbacks>(callbacks)...
					);

					applied_entropy.clear();
					++step_i;
				}
			}
		);
	}

	self.additional_steps = 0;
}

template <class M>
void editor_player::init_mode(M&& mode, const mode_vars_id& vars_id) {
	ensure(!has_testing_started());

	current_mode_vars_id = vars_id;
	current_mode.emplace<remove_cref<M>>(std::move(mode));
}
