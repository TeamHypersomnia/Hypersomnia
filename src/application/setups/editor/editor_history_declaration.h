#pragma once
#include "game/assets/ids/asset_ids.h"
#include "application/setups/editor/resources/all_editor_resource_types.h"
#include "application/setups/editor/nodes/all_editor_node_types.h"

template <class T>
struct edit_resource_command;

template <class T>
struct edit_node_command;

namespace augs {
	template <class...>
	class history_with_saved_revision;
};

struct paste_nodes_command;
struct delete_nodes_command;
struct duplicate_nodes_command;

struct instantiate_resource_command;

struct create_layer_command;

struct reorder_nodes_command;
struct reorder_layers_command;

struct delete_layers_command;

using editor_history_base = augs::history_with_saved_revision<
	edit_resource_command<editor_sprite_resource>,
	edit_resource_command<editor_sound_resource>,
	edit_resource_command<editor_light_resource>,
	edit_node_command<editor_sprite_node>,
	edit_node_command<editor_sound_node>,
	edit_node_command<editor_light_node>
>;
