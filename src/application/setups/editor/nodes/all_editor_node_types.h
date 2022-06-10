#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_mod_templates.h"

#include "application/setups/editor/nodes/editor_light.h"
#include "application/setups/editor/nodes/editor_sprite.h"
#include "application/setups/editor/nodes/editor_prefab_instance.h"

using all_editor_node_types = type_list<
	editor_light,
	editor_sprite,
	editor_prefab_instance
>;
