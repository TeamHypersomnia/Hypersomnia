#pragma once
#include "application/setups/editor/editor_player.h"

template <class E, class C, class F>
void on_mode_with_input(
	E& self,
	const all_mode_vars_maps& all_vars,
	C& cosm,
	F callback
) {
	std::visit(
		[&](auto& typed_mode) {
			using M = remove_cref<decltype(typed_mode)>;
			using V = typename M::vars_type;
			using I = typename M::input;
			
			if (const auto vars = mapped_or_nullptr(all_vars.get_for<V>(), self.current_mode_vars_id)) {
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

template <class C, class... Callbacks>
void advance_player(
	editor_player& self,
	augs::delta frame_delta,
	const all_mode_vars_maps& all_vars,
	C& cosm,
	Callbacks&&... callbacks
) {
	if (!self.paused) {
		self.timer.advance(frame_delta *= self.speed);
	}

	auto steps = self.additional_steps + self.timer.extract_num_of_logic_steps(cosm.get_fixed_delta());

	on_mode_with_input(
		self,
		all_vars,
		cosm,
		[&](auto& typed_mode, const auto& in) {
			while (steps--) {
				self.total_collected_entropy.clear_dead_entities(cosm);

				typed_mode.advance(
					in,
					{ self.total_collected_entropy },
					std::forward<Callbacks>(callbacks)...
				);

				self.total_collected_entropy.clear();
			}
		}
	);

	self.additional_steps = 0;
}
