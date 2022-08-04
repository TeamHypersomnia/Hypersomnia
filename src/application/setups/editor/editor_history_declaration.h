#pragma once
#include "game/assets/ids/asset_ids.h"
#include "application/setups/editor/resources/all_editor_resource_types_declaration.h"
#include "application/setups/editor/nodes/all_editor_node_types_declaration.h"

template <class T>
struct edit_resource_command;

template <class T>
struct edit_node_command;

template <class T>
struct create_node_command;

template <class T>
struct delete_node_command;

namespace augs {
	template <class...>
	class history_with_saved_revision;
};

struct paste_nodes_command;
struct delete_nodes_command;
struct duplicate_nodes_command;

struct create_layer_command;
struct delete_layers_command;

struct reorder_nodes_command;
struct reorder_layers_command;

struct delete_layers_command;
struct create_multiple_nodes;
struct rename_node_command;

class move_nodes_command;
class resize_nodes_command;
class flip_nodes_command;

struct toggle_nodes_visibility_command;
struct toggle_layers_visibility_command;

struct inspect_command;

using editor_history_base = augs::history_with_saved_revision<
	edit_resource_command<editor_sprite_resource>,
	edit_resource_command<editor_sound_resource>,
	edit_resource_command<editor_light_resource>,

	edit_node_command<editor_sprite_node>,
	edit_node_command<editor_sound_node>,
	edit_node_command<editor_light_node>,

	rename_node_command,

	create_layer_command,
	delete_layers_command,

	create_node_command<editor_sprite_node>,
	create_node_command<editor_sound_node>,
	create_node_command<editor_light_node>,

	reorder_nodes_command,
	reorder_layers_command,

	move_nodes_command,
	resize_nodes_command,
	flip_nodes_command,

	delete_nodes_command,
	duplicate_nodes_command,

	toggle_nodes_visibility_command,
	toggle_layers_visibility_command,

	inspect_command
>;
