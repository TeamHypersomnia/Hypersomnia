#pragma once
#include "augs/misc/scope_guard.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"

void standard_solve(const logic_step step);

struct standard_solver {
	template <class Pre, class Post, class PostCleanup>
	void operator()(
		logic_step_input input,
		Pre pre_solve,
		Post post_solve,
		PostCleanup post_cleanup
	) {
		thread_local data_living_one_step queues;
		const logic_step step(input, queues);

		auto scope = augs::scope_guard([](){
			queues.clear();
		});

		pre_solve(step);
		standard_solve(step);
		post_solve(const_logic_step(step));
		step.perform_deletions();
		post_cleanup(const_logic_step(step));
	}
};


