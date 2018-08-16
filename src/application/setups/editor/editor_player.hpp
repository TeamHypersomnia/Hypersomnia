#pragma once
#include "application/setups/editor/editor_player.h"

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

	std::visit(
		[&](auto& typed_mode) {
			using M = remove_cref<decltype(typed_mode)>;
			using V = typename M::vars_type;
			
			if (const auto vars = mapped_or_nullptr(all_vars.get_for<V>(), self.current_mode_vars_id)) {
				while (steps--) {
					self.total_collected_entropy.clear_dead_entities(cosm);

					if constexpr(M::needs_initial_signi) {
						const auto& initial = self.mode_initial_signi;
						const auto& chosen_initial = 
							initial != nullptr ? 
							*initial :
							cosm.get_solvable().significant
						;

						typed_mode.advance(
							{ *vars, chosen_initial, cosm },
							{ self.total_collected_entropy },
							std::forward<Callbacks>(callbacks)...
						);
					}
					else {
						typed_mode.advance(
							{ *vars, cosm },
							{ self.total_collected_entropy },
							std::forward<Callbacks>(callbacks)...
						);
					}

					self.total_collected_entropy.clear();
				}
			}
		},
		self.current_mode
	);

	self.additional_steps = 0;
}
