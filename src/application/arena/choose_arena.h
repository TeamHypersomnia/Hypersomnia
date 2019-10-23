#pragma once
#include "augs/log_direct.h"
#include "application/arena/arena_paths.h"

inline void choose_arena(
	sol::state& lua,
	online_arena_handle<false> handle,
	const server_solvable_vars& vars,
	cosmos_solvable_significant& initial_signi
) {
	const auto& name = vars.current_arena;

	if (name.empty()) {
		LOG_DIRECT("Arena name empty, so making a default one.");

		handle.make_default(
			lua, 
			initial_signi
		);
	}
	else {
		const auto paths = arena_paths(name);
		LOG_DIRECT("Solv file: " + paths.int_paths.solv_file.string());

		handle.load_from(
			paths,
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

