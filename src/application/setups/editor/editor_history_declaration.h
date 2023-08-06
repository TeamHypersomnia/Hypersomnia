#pragma once
#include "game/assets/ids/asset_ids.h"
#include "application/setups/editor/resources/all_editor_resource_types_declaration.h"
#include "application/setups/editor/nodes/all_editor_node_types_declaration.h"

template <class T>
struct create_resource_command;

template <class T>
struct edit_resource_command;

struct delete_resources_command;

struct rename_resource_command;

template <class T>
struct create_node_command;

template <class T>
struct edit_node_command;

struct delete_nodes_command;
struct clone_nodes_command;

struct create_layer_command;
struct delete_layers_command;
struct rename_layer_command;

struct reorder_nodes_command;
struct reorder_layers_command;

struct delete_layers_command;
struct create_multiple_nodes;
struct rename_node_command;
struct change_resource_command;

class move_nodes_command;
class resize_nodes_command;
class flip_nodes_command;

struct toggle_nodes_active_command;
struct toggle_layers_active_command;

struct edit_layer_command;

struct edit_project_settings_command;

struct inspect_command;
struct unpack_prefab_command;

struct replace_whole_project_command;

struct editor_history;

namespace augs {
	template <class Derived, class... CommandTypes>
	class history;
};

using editor_history_base = augs::history<
	editor_history,

	create_resource_command<editor_material_resource>,

	edit_resource_command<editor_sprite_resource>,
	edit_resource_command<editor_sound_resource>,
	edit_resource_command<editor_light_resource>,
	edit_resource_command<editor_particles_resource>,
	edit_resource_command<editor_wandering_pixels_resource>,
	edit_resource_command<editor_point_marker_resource>,
	edit_resource_command<editor_area_marker_resource>,
	edit_resource_command<editor_material_resource>,

	edit_resource_command<editor_firearm_resource>,
	edit_resource_command<editor_ammunition_resource>,
	edit_resource_command<editor_tool_resource>,
	edit_resource_command<editor_melee_resource>,
	edit_resource_command<editor_explosive_resource>,

	edit_resource_command<editor_prefab_resource>,
	edit_resource_command<editor_game_mode_resource>,

	rename_resource_command,
	delete_resources_command,

	edit_node_command<editor_sprite_node>,
	edit_node_command<editor_sound_node>,
	edit_node_command<editor_light_node>,
	edit_node_command<editor_particles_node>,
	edit_node_command<editor_wandering_pixels_node>,
	edit_node_command<editor_point_marker_node>,
	edit_node_command<editor_area_marker_node>,

	edit_node_command<editor_firearm_node>,
	edit_node_command<editor_ammunition_node>,
	edit_node_command<editor_tool_node>,
	edit_node_command<editor_melee_node>,
	edit_node_command<editor_explosive_node>,

	edit_node_command<editor_prefab_node>,

	rename_node_command,
	change_resource_command,

	create_layer_command,
	delete_layers_command,
	rename_layer_command,

	create_node_command<editor_sprite_node>,
	create_node_command<editor_sound_node>,
	create_node_command<editor_light_node>,
	create_node_command<editor_particles_node>,
	create_node_command<editor_wandering_pixels_node>,
	create_node_command<editor_point_marker_node>,
	create_node_command<editor_area_marker_node>,

	create_node_command<editor_firearm_node>,
	create_node_command<editor_ammunition_node>,
	create_node_command<editor_tool_node>,
	create_node_command<editor_melee_node>,
	create_node_command<editor_explosive_node>,

	create_node_command<editor_prefab_node>,

	reorder_nodes_command,
	reorder_layers_command,

	move_nodes_command,
	resize_nodes_command,
	flip_nodes_command,

	delete_nodes_command,
	clone_nodes_command,

	toggle_nodes_active_command,
	toggle_layers_active_command,

	edit_layer_command,
	edit_project_settings_command,

	inspect_command,
	unpack_prefab_command,

	replace_whole_project_command
>;
