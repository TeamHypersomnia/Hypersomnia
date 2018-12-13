#pragma once
#include "application/setups/editor/editor_player.h"
#include "game/modes/on_mode_with_input.hpp"

template <class E, class A, class C, class F>
decltype(auto) editor_player::on_mode_with_input_impl(
	E& self,
	const A& all_vars,
	C& cosm,
	F&& callback
) {
	ensure(self.has_testing_started());

	return general_on_mode_with_input(
		self.current_mode.state,
		all_vars,
		self.current_mode.rules_id,
		self.before_start.commanded->work.world.get_solvable().significant,
		cosm,
		std::forward<F>(callback)
	);
}

