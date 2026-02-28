#pragma once
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmic_functions.h"
#include "augs/misc/randomization.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/solvers/solve_structs.h"

#include "game/cosmos/solvers/solver_callbacks.h"

void standard_solve(const logic_step step);

struct standard_solver {
	static data_living_one_step& get_thread_local_queues();

	template <class C>
	solve_result operator()(
		logic_step_input input,
		C&& callbacks
	) {
		auto& queues = get_thread_local_queues();

		auto step_rng = randomization(input.cosm.get_total_steps_passed());

		solve_result result;
		const auto step = logic_step(input, queues, step_rng, result);

		callbacks.pre_solve(step);
		standard_solve(step);
		step.flush_create_entity_requests();
		callbacks.post_solve(step);
		step.perform_deletions();
		callbacks.post_cleanup(const_logic_step(step));

		return result;
	}
};


