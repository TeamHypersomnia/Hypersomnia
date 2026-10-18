#pragma once
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/commands/replace_whole_project_command.h"
#include "game/common_state/bot_waypoints_settings.h"
#include "game/modes/ai/random_waypoints_generator.h"
#include "game/modes/detail/create_random_waypoints.hpp"

/*
	The editor's side of random bot waypoints (see random_waypoints_generator.h):
	counting the author's waypoints for the note under the Bot waypoints mode,
	and generating a preview into the random_waypoints layer - one undoable step, overwriting the previous preview.
	The preview layer never counts as the author's waypoints and is never used in game.
*/

template <class F>
void for_each_waypoint_node_with_layer(const editor_project& project, const editor_resource_pools& officials, F&& callback) {
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
						if (::is_waypoint(resource->editable.type)) {
							callback(node, resource->editable.type, *layer);
						}
					}
				}
			});
		}
	}
}

std::size_t editor_setup::count_author_waypoints() const {
	std::size_t count = 0;

	::for_each_waypoint_node_with_layer(project, official.resources, [&](const auto&, const auto, const editor_layer& layer) {
		if (layer.unique_name != random_waypoints_layer_name_v) {
			++count;
		}
	});

	return count;
}

std::vector<random_waypoint> editor_setup::calc_random_waypoints_preview(const uint32_t seed) const {
	/*
		The arena built like for a playtest (see start_playtesting), into a cosmos of its own -
		so the preview has exactly the waypoints a round with this seed gets in the playtest:
		the same nodes and layers, the same navmesh, the same game mode.
	*/

	intercosm built_scene;
	all_rulesets_variant built_ruleset;
	all_modes_variant built_mode_state;
	cosmos_solvable_significant built_clean_round_state;
	synced_dynamic_vars built_dynamic_vars;

	const auto built_handle = editor_arena_handle<false> {
		built_mode_state,
		built_scene,
		built_scene.world,
		built_ruleset,
		built_clean_round_state,
		built_dynamic_vars
	};

	const bool for_playtesting = true;
	const bool editor_preview = false;
	const bool showing_navmesh = true;

	/*
		Building writes the built entity ids into the nodes - into a copy, so the editor's scene keeps its own.
	*/

	const auto project_copy = project;

	::build_arena_from_editor_project<editor_arena_handle<false>>(
		built_handle,
		{
			project_copy,
			game_mode_name_type(""),
			paths.project_folder,
			official,
			nullptr,
			nullptr,
			for_playtesting,
			editor_preview,
			showing_navmesh
		}
	);

	const auto* const arena_rules = std::get_if<arena_mode_ruleset>(std::addressof(built_ruleset));
	const bool gun_game = arena_rules != nullptr && arena_rules->is_gun_game();

	auto input = ::gather_random_waypoints_input(built_scene.world, gun_game);
	input.seed = seed;

	/*
		Like create_random_waypoints - except that Automatic previews generated ones even when the map has the author's.
	*/

	if (project.settings.bot_waypoints.mode != bot_waypoints_mode::YOURS_AND_GENERATED) {
		input.existing.clear();
	}

	return ::generate_random_waypoints(input);
}

void editor_setup::generate_random_waypoints_preview(const uint32_t seed) {
	const auto generated = calc_random_waypoints_preview(seed);

	view.bot_waypoints_preview_seed = seed;

	auto after = std::make_unique<editor_project>(project);

	/*
		A preview layer kept in the scratchpad stays there under a free name - it was moved there to be kept.
	*/

	for (const auto layer_id : after->layers.scratchpad) {
		if (auto* const layer = after->find_layer(layer_id)) {
			if (layer->unique_name == random_waypoints_layer_name_v) {
				layer->unique_name = get_free_layer_name_for(std::string(random_waypoints_layer_name_v) + " (kept)");
			}
		}
	}

	/*
		Overwrite: the previous preview layer goes away with all its nodes.
	*/

	{
		auto& layers = after->layers;
		const auto previous = std::find_if(layers.order.begin(), layers.order.end(), [&](const auto id) {
			const auto layer = after->find_layer(id);
			return layer != nullptr && layer->unique_name == random_waypoints_layer_name_v;
		});

		if (previous != layers.order.end()) {
			const auto previous_id = *previous;

			for (const auto node_id : after->find_layer(previous_id)->hierarchy.nodes) {
				after->nodes.on_pool(node_id.type_id, [&](auto& pool) {
					pool.free(node_id.raw);
				});
			}

			layers.pool.free(previous_id);
			layers.order.erase(previous);
		}
	}

	editor_layer new_layer;
	new_layer.unique_name = random_waypoints_layer_name_v;

	const auto& official_markers = official.resource_map.point_markers;
	auto& marker_pool = after->nodes.template get_pool_for<editor_point_marker_node>();
	std::size_t index = 0;

	for (const auto& w : generated) {
		const auto resource = official_markers.find(w.type);

		if (resource == official_markers.end()) {
			continue;
		}

		editor_point_marker_node node;
		node.resource_id = resource->second;
		node.unique_name = typesafe_sprintf("Random %x %x", w.type == point_marker_type::BOT_WAYPOINT_PUSH ? "push" : "patrol", ++index);
		node.chronological_order = after->nodes.next_chronological_order++;

		auto& e = node.editable;
		e.pos = w.pos;
		e.rotation = w.rotation;
		e.faction = w.faction;
		e.letter = w.letter;
		e.camp = w.camp;
		e.camp_look_far = w.camp_look_far;
		e.camp_secs_min = w.camp_secs_min;
		e.camp_secs_max = w.camp_secs_max;

		const auto allocation = marker_pool.allocate(std::move(node));
		new_layer.hierarchy.nodes.push_back(editor_typed_node_id<editor_point_marker_node>::from_raw(allocation.key).operator editor_node_id());
	}

	{
		auto& layers = after->layers;
		const auto allocation = layers.pool.allocate(std::move(new_layer));
		layers.order.insert(layers.order.begin(), allocation.key);
	}

	replace_whole_project_command cmd;
	cmd.before = std::make_unique<editor_project>(project);
	cmd.after = std::move(after);
	cmd.built_description = typesafe_sprintf("Generated %x random waypoints (seed %x)", generated.size(), seed);

	post_new_command(std::move(cmd));
}
