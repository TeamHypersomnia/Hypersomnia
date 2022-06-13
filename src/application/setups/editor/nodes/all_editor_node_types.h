#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_mod_templates.h"

#include "application/setups/editor/nodes/editor_light_node.h"
#include "application/setups/editor/nodes/editor_sprite_node.h"
#include "application/setups/editor/nodes/editor_prefab_node.h"

using all_editor_node_types = type_list<
	editor_light_node,
	editor_sprite_node,
	editor_prefab_node
>;
