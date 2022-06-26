#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_mod_templates.h"

#include "application/setups/editor/resources/editor_prefab_resource.h"
#include "application/setups/editor/resources/editor_light_resource.h"
#include "application/setups/editor/resources/editor_sprite_resource.h"
#include "application/setups/editor/resources/editor_sound_resource.h"

using all_editor_resource_types = type_list<
	editor_sprite_resource,
	editor_sound_resource,
	editor_prefab_resource,
	editor_light_resource
>;
