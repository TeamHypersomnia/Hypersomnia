#pragma once
#include "augs/log_direct.h"
#include "application/arena/arena_paths.h"

inline void choose_arena(
	sol::state& lua,
	online_arena_handle<false> handle,
	const server_solvable_vars& vars,
	cosmos_solvable_significant& clean_round_state
) {
	const auto& name = vars.current_arena;
	const auto emigrated_session = handle.on_mode([](const auto& typed_mode) { return typed_mode.emigrate(); });

	if (name.empty()) {
		LOG_DIRECT("Arena name empty, so making a default one.");

		handle.make_default(
			lua, 
			clean_round_state
		);
	}
	else {
		const auto paths = arena_paths(name);
		LOG_DIRECT("Solv file: " + paths.int_paths.solv_file.string());

		handle.load_from(
			paths,
			clean_round_state
		);
	}

	if (vars.override_default_ruleset.empty()) {
		handle.current_mode.choose(handle.rulesets.meta.server_default);
	}
	else {
		LOG_NVPS(vars.override_default_ruleset);
		ensure(false && "Not implemented!");
	}

	handle.on_mode_with_input(
		[&emigrated_session](auto& typed_mode, const auto& input) {
			typed_mode.migrate(input, emigrated_session);
		}
	);
}

