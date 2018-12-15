#pragma once
#include "application/arena/arena_paths.h"

inline void choose_arena(
	sol::state& lua,
	online_arena_handle<false> handle,
	const server_vars& vars,
	cosmos_solvable_significant& initial_signi
) {
	const auto& name = vars.current_arena;

	if (name.empty()) {
		handle.make_default(
			lua, 
			initial_signi
		);
	}
	else {
		handle.load_from(
			arena_paths(name),
			initial_signi
		);
	}

	if (vars.override_default_ruleset.empty()) {
		handle.current_mode.choose(handle.rulesets.meta.server_default);
	}
	else {
		ensure(false && "Not implemented!");
	}
}

