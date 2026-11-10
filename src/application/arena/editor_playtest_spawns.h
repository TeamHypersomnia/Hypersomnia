#pragma once
#include <variant>
#include <vector>
#include "game/components/marker_component.h"
#include "game/modes/arena_mode.h"
#include "game/modes/test_mode.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/editor_rebuild_game_mode.hpp"

inline bool is_playtest_marker_node(const editor_point_marker_node& node, const point_marker_type type) {
	if (type == point_marker_type::TEAM_SPAWN) {
		return node.editable.playtest_spawn;
	}

	return true;
}

template <class F>
void for_each_playtest_marker_node(
	const editor_project& project,
	const editor_resource_pools& officials,
	const point_marker_type type,
	F&& callback
) {
	for (const auto layer_id : project.layers.order) {
		const auto layer = project.find_layer(layer_id);

		if (layer == nullptr) {
			continue;
		}

		for (const auto node_id : layer->hierarchy.nodes) {
			project.on_node(node_id, [&](const auto& node, const auto) {
				using N = remove_cref<decltype(node)>;

				if constexpr(std::is_same_v<N, editor_point_marker_node>) {
					if (const auto resource = project.find_resource(officials, node.resource_id)) {
						if (resource->editable.type == type && ::is_playtest_marker_node(node, type)) {
							callback(node);
						}
					}
				}
			});
		}
	}
}

inline bool has_playtest_spawns(const editor_project& project, const editor_resource_pools& officials) {
	bool found = false;

	::for_each_playtest_marker_node(project, officials, point_marker_type::TEAM_SPAWN, [&](const auto&) {
		found = true;
	});

	return found;
}

inline bool has_bot_test_navigation_ends(const editor_project& project, const editor_resource_pools& officials) {
	bool found = false;

	::for_each_playtest_marker_node(project, officials, point_marker_type::BOT_TEST_NAVIGATION_END, [&](const auto&) {
		found = true;
	});

	return found;
}

template <class F>
std::vector<playtest_spawn> make_playtest_spawns(
	const editor_project& project,
	const editor_resource_pools& officials,
	const point_marker_type type,
	F find_resource
) {
	auto playtest_spawns = std::vector<playtest_spawn>();

	::for_each_playtest_marker_node(project, officials, type, [&](const editor_point_marker_node& node) {
		if (!node.scene_entity_id.is_set()) {
			return;
		}

		auto faction_of_spawn = components::marker();
		faction_of_spawn.faction = node.editable.faction;

		auto spawn_def = playtest_spawn();
		spawn_def.spawn = node.scene_entity_id;
		spawn_def.custom_equipment = node.editable.playtest_custom_equipment;
		spawn_def.respawns_in_quick_test = node.editable.respawns_in_quick_test;
		spawn_def.bot_nickname = node.get_display_name();

		if (spawn_def.custom_equipment) {
			spawn_def.equipment = ::to_game_requested_equipment(
				node.editable.playtest_equipment,
				find_resource,
				faction_of_spawn.get_faction()
			);
		}

		playtest_spawns.push_back(spawn_def);
	});

	return playtest_spawns;
}

inline std::vector<bot_test_navigation_end> make_bot_test_navigation_ends(
	const editor_project& project,
	const editor_resource_pools& officials
) {
	auto ends = std::vector<bot_test_navigation_end>();

	::for_each_playtest_marker_node(project, officials, point_marker_type::BOT_TEST_NAVIGATION_END, [&](const editor_point_marker_node& node) {
		if (!node.scene_entity_id.is_set()) {
			return;
		}

		auto end_def = bot_test_navigation_end();
		end_def.end = node.scene_entity_id;
		end_def.sprint = node.editable.test_navigation_sprint;
		end_def.hides_weapons_on_arrival = node.editable.test_navigation_hides_weapons;

		ends.push_back(end_def);
	});

	return ends;
}

template <class A, class F>
void set_playtest_spawns_in_rules(
	A arena_handle,
	const editor_project& project,
	const editor_resource_pools& officials,
	F find_resource
) {
	if (const auto rules = std::get_if<arena_mode_ruleset>(std::addressof(arena_handle.ruleset))) {
		rules->playtest_spawns = ::make_playtest_spawns(project, officials, point_marker_type::TEAM_SPAWN, find_resource);
	}

	if (const auto rules = std::get_if<test_mode_ruleset>(std::addressof(arena_handle.ruleset))) {
		rules->quick_test_spawns = ::make_playtest_spawns(project, officials, point_marker_type::QUICK_TEST_SPAWN, find_resource);
		rules->bot_test_navigation_ends = ::make_bot_test_navigation_ends(project, officials);
	}
}
