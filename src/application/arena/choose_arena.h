#pragma once
#include "augs/log_direct.h"
#include "application/arena/arena_paths.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/arena/arena_playtesting_context.h"
#include "application/arena/build_arena_from_editor_project.h"
#include "application/setups/editor/packaged_official_content_declaration.h"

#include "application/setups/editor/project/editor_project.h"

struct choose_arena_input {
	sol::state& lua;
	online_arena_handle<false> handle;
	const packaged_official_content& official;
	const arena_identifier& name;
	const ruleset_name_type& override_default_ruleset;
	cosmos_solvable_significant& clean_round_state;
	std::optional<arena_playtesting_context> playtesting_context;
	editor_project* keep_loaded_project;
	
	bool is_for_playtesting() const {
		return playtesting_context.has_value();
	}

	void make_default() {
		LOG_NOFORMAT("Couldn't load requested arena, so making a default one.");

		handle.make_default(lua, clean_round_state);
	}
};

inline void load_arena_from_path(
	const choose_arena_input in,
	const augs::path_type& json_path,
	augs::secure_hash_type* const output_arena_hash
) {
	const bool strict = true;
	scene_entity_to_node_map* const entity_to_node = nullptr;

	auto project = editor_project_readwrite::read_project_json(
		json_path,
		official_get_resources(in.official),
		official_get_resource_map(in.official),
		strict,
		output_arena_hash
	);

	const auto project_dir = json_path.parent_path();

	::build_arena_from_editor_project(
		in.handle,
		{
			project,
			project_dir,
			in.official,
			entity_to_node,
			std::addressof(in.clean_round_state),
			in.is_for_playtesting(),
			false /* editor_preview */
		}
	);

	if (in.keep_loaded_project) {
		*in.keep_loaded_project = std::move(project);
	}
}

inline void load_arena_from_string(
	const choose_arena_input in,
	const augs::path_type& project_dir,
	const std::string& json_document
) {
	const bool strict = true;
	scene_entity_to_node_map* const entity_to_node = nullptr;

	auto project = editor_project_readwrite::read_project_json(
		project_dir,
		json_document,
		official_get_resources(in.official),
		official_get_resource_map(in.official),
		strict,
		nullptr /* output_arena_hash */
	);

	::build_arena_from_editor_project(
		in.handle,
		{
			project,
			project_dir,
			in.official,
			entity_to_node,
			std::addressof(in.clean_round_state),
			in.is_for_playtesting(),
			false /* editor_preview */
		}
	);

	if (in.keep_loaded_project) {
		*in.keep_loaded_project = std::move(project);
	}
}

struct server_choose_arena_result {
	augs::secure_hash_type required_hash = augs::secure_hash_type();
	augs::path_type arena_folder_path;
};

inline server_choose_arena_result choose_arena_server(
	choose_arena_input in
) {
	const auto emigrated_session = in.handle.emigrate_mode_session();

	auto result = server_choose_arena_result();

	if (const auto path = ::server_choose_arena_file_by(in.name); !path.empty()) {
		LOG_NOFORMAT("Loading arena from: " + path.string());

		::load_arena_from_path(in, path, std::addressof(result.required_hash));

		result.arena_folder_path = path.parent_path();
	}
	else {
		in.make_default();
	}

	if (in.override_default_ruleset.size() > 0) {
		// handle.current_mode.choose(in.override_default_ruleset);
	}
	else {
		const auto launched_mode = in.is_for_playtesting() ?
			in.handle.rulesets.meta.playtest_default :
			in.handle.rulesets.meta.server_default
		;

		in.handle.current_mode.choose(launched_mode);
	}

	in.handle.migrate_mode_session(emigrated_session);

	in.handle.on_mode(
		[&]<typename T>(T& typed_mode) {
			if constexpr(std::is_same_v<test_mode, T>) {
				typed_mode.playtesting_context = in.playtesting_context;
			}
		}
	);

	return result;
}

inline client_find_arena_result choose_arena_client(
	choose_arena_input in,
	const augs::secure_hash_type& required_hash
) {
	const auto emigrated_session = in.handle.emigrate_mode_session();

	client_find_arena_result result;
	bool altered = false;

	result = ::client_find_arena(in.name, required_hash);

	if (const auto maybe_arena_folder = result.arena_folder_path) {
		LOG_NOFORMAT("Arena with a matching hash found in: " + maybe_arena_folder->string());

		::load_arena_from_string(in, *maybe_arena_folder, result.json_document);
		altered = true;
	}
	else {
		LOG_NOFORMAT("Couldn't find arena with a matching hash. Creating default.");

		in.make_default();
		altered = true;
	}

	if (altered) {
		if (in.override_default_ruleset.size() > 0) {
			// in.handle.current_mode.choose(in.override_default_ruleset);
		}
		else {
			const auto launched_mode = in.is_for_playtesting() ?
				in.handle.rulesets.meta.playtest_default :
				in.handle.rulesets.meta.server_default
			;

			in.handle.current_mode.choose(launched_mode);
		}

		in.handle.migrate_mode_session(emigrated_session);

		in.handle.on_mode(
			[&]<typename T>(T& typed_mode) {
				if constexpr(std::is_same_v<test_mode, T>) {
					typed_mode.playtesting_context = in.playtesting_context;
				}
			}
		);
	}

	return result;
}

