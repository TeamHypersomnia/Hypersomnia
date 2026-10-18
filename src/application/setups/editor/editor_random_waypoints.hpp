#pragma once
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/commands/replace_whole_project_command.h"
#include "game/common_state/bot_waypoints_settings.h"
#include "game/modes/ai/random_waypoints_generator.h"
#include "game/modes/detail/create_random_waypoints.hpp"
#include "game/detail/navmesh_generation.hpp"

/*
	The editor's side of random bot waypoints (see random_waypoints_generator.h):
	counting the author's waypoints for the note under the Bot waypoints mode,
	and generating a preview into the random_waypoints layer - one undoable step, overwriting the previous preview.
	The preview layer never counts as the author's waypoints and is never used in game.
*/

/*
	Spawns and bombsites from the project's nodes - the editor's scene leaves out the hidden ones, the game doesn't.
*/

inline void gather_spawns_and_sites_from_project(
	const editor_project& project,
	const editor_resource_pools& officials,
	random_waypoints_input& input
) {
	input.resistance_spawns.clear();
	input.metropolis_spawns.clear();
	input.bombsites.clear();

	std::vector<vec2> ffa_spawns;

	for (const auto& node : project.nodes.template get_pool_for<editor_point_marker_node>()) {
		if (const auto resource = project.find_resource(officials, node.resource_id)) {
			const auto type = resource->editable.type;

			if (type == point_marker_type::TEAM_SPAWN) {
				(node.editable.faction == faction_type::METROPOLIS ? input.metropolis_spawns : input.resistance_spawns).push_back(node.editable.pos);
			}
			else if (type == point_marker_type::FFA_SPAWN) {
				ffa_spawns.push_back(node.editable.pos);
			}
		}
	}

	if (input.gun_game) {
		auto& all = input.resistance_spawns;
		all.insert(all.end(), input.metropolis_spawns.begin(), input.metropolis_spawns.end());
		all.insert(all.end(), ffa_spawns.begin(), ffa_spawns.end());
		input.metropolis_spawns.clear();
		return;
	}

	for (const auto& node : project.nodes.template get_pool_for<editor_area_marker_node>()) {
		if (const auto resource = project.find_resource(officials, node.resource_id)) {
			if (::is_bombsite(resource->editable.type)) {
				input.bombsites.emplace_back(ltrb::center_and_size(node.editable.pos, vec2(node.editable.size)), node.editable.letter);
			}
		}
	}
}

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

void editor_setup::generate_random_waypoints_preview(const uint32_t seed) {
	const auto& settings = project.settings;

	const bool gun_game = [&]() {
		if (const auto mode = project.find_resource(official.resources, settings.default_server_mode)) {
			return mode->type.template is<editor_gun_game_mode>();
		}

		return false;
	}();

	auto input = ::gather_random_waypoints_input(scene.world, gun_game);
	input.density = settings.bot_waypoints.density;
	input.seed = seed;

	::gather_spawns_and_sites_from_project(project, official.resources, input);

	/*
		The editor's scene has a navmesh only while it is shown - otherwise one is made just for this.
	*/

	cosmos_navmesh own_navmesh;

	if (input.navmesh->islands.empty()) {
		own_navmesh = ::generate_navmesh(scene.world, std::max(16u, settings.navmesh_cell_size));
		input.navmesh = &own_navmesh;
	}

	/*
		The scene shows the previous preview and the author's waypoints alike - the author's come from the project.
	*/

	input.existing.clear();

	if (settings.bot_waypoints.mode == bot_waypoints_mode::YOURS_AND_GENERATED) {
		::for_each_waypoint_node_with_layer(project, official.resources, [&](const editor_point_marker_node& node, const point_marker_type type, const editor_layer& layer) {
			if (layer.unique_name != random_waypoints_layer_name_v) {
				random_waypoint w;
				w.pos = node.editable.pos;
				w.type = type;
				w.faction = node.editable.faction;
				w.letter = node.editable.letter;
				input.existing.push_back(w);
			}
		});
	}

	const auto generated = ::generate_random_waypoints(input);

	auto after = std::make_unique<editor_project>(project);
	after->settings.bot_waypoints_preview_seed = seed;

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
