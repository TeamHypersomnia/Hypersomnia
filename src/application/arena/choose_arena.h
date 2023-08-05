#pragma once
#include "augs/log_direct.h"
#include "application/arena/arena_paths.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/arena/arena_playtesting_context.h"
#include "application/arena/build_arena_from_editor_project.h"
#include "application/setups/editor/packaged_official_content_declaration.h"

#include "application/setups/editor/project/editor_project.h"
#include "application/network/network_common.h"

struct choose_arena_input {
	editor_project_readwrite::reading_settings settings;

	sol::state& lua;
	online_arena_handle<false> handle;
	const packaged_official_content& official;
	const arena_identifier& name;
	const game_mode_name_type& override_game_mode;
	cosmos_solvable_significant& clean_round_state;
	std::optional<arena_playtesting_context> playtesting_context;
	editor_project* keep_loaded_project;
	scene_entity_to_node_map* const entity_to_node;

	bool is_for_playtesting() const {
		return playtesting_context.has_value();
	}

	void make_default() {
		LOG_NOFORMAT("Couldn't find arena with a matching hash.\nCreating default scene until another one is chosen.");

		handle.make_default(lua, clean_round_state);
		handle.choose_mode(test_mode_ruleset());
	}
};

inline void load_arena_from_path(
	const choose_arena_input in,
	const augs::path_type& json_path,
	augs::secure_hash_type* const output_arena_hash
) {
	auto project = editor_project_readwrite::read_project_json(
		json_path,
		official_get_resources(in.official),
		official_get_resource_map(in.official),
		in.settings,
		output_arena_hash
	);

	const auto project_dir = json_path.parent_path();

	::build_arena_from_editor_project(
		in.handle,
		{
			project,
			in.override_game_mode,
			project_dir,
			in.official,
			in.entity_to_node,
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
	const std::string& json_document,
	scene_entity_to_node_map* const entity_to_node = nullptr
) {
	auto project = editor_project_readwrite::read_project_json(
		project_dir,
		json_document,
		official_get_resources(in.official),
		official_get_resource_map(in.official),
		in.settings,
		nullptr /* output_arena_hash */
	);

	::build_arena_from_editor_project(
		in.handle,
		{
			project,
			in.override_game_mode,
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
	augs::secure_hash_type loaded_arena_hash = augs::secure_hash_type();
	augs::path_type arena_folder_path;
};

inline server_choose_arena_result choose_arena_server(
	choose_arena_input in
) {
	const auto emigrated_session = in.handle.emigrate_mode_session();

	auto result = server_choose_arena_result();

	if (const auto path = ::server_choose_arena_file_by(in.name); !path.empty()) {
		LOG_NOFORMAT("Loading arena from: " + path.string());

		::load_arena_from_path(in, path, std::addressof(result.loaded_arena_hash));

		result.arena_folder_path = path.parent_path();
	}
	else {
		in.make_default();
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
		in.make_default();
		altered = true;
	}

	if (altered) {
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

