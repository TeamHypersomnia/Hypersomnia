#pragma once
#include "application/setups/editor/editor_player.h"

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
				
				if (const auto vars = mapped_or_nullptr(all_vars.template get_for<M>(), self.current_mode_rules_id)) {
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

