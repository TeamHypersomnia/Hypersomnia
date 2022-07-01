#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_mod_templates.h"

struct editor_light_node;
struct editor_sprite_node;
struct editor_sound_node;
struct editor_prefab_node;

using all_editor_node_types = type_list<
	editor_light_node,
	editor_sprite_node,
	editor_sound_node,
	editor_prefab_node
>;
