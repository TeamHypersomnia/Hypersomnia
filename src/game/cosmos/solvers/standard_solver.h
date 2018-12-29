#pragma once
#include "augs/misc/scope_guard.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmic_functions.h"
#include "augs/misc/randomization.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/solvers/solve_structs.h"

#include "game/cosmos/solvers/solver_callbacks.h"
#include "game/organization/all_messages_includes.h"

void standard_solve(const logic_step step);

struct standard_solver {
	template <class C>
	solve_result operator()(
		logic_step_input input,
		C&& callbacks
	) {
		thread_local data_living_one_step queues;
		auto step_rng = randomization(input.cosm.get_total_steps_passed());

		solve_result result;
		const auto step = logic_step(input, queues, step_rng, result);

		auto scope = augs::scope_guard([](){
			queues.clear();
		});

		cosmic::increment_step(input.cosm);
		callbacks.pre_solve(step);
		standard_solve(step);
		callbacks.post_solve(const_logic_step(step));
		step.perform_deletions();
		callbacks.post_cleanup(const_logic_step(step));

		return result;
	}
};


