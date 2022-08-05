#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_mod_templates.h"

struct editor_sprite_resource;
struct editor_sound_resource;
struct editor_prefab_resource;
struct editor_light_resource;
struct editor_particles_resource;

struct editor_material_resource;

using all_editor_resource_types = type_list<
	editor_sprite_resource,
	editor_sound_resource,
	//editor_prefab_resource,
	editor_light_resource,
	editor_particles_resource,

	editor_material_resource
>;
