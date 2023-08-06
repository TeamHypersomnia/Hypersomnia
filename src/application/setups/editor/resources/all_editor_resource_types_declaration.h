#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_mod_templates.h"
#include "augs/templates/list_utils.h"

struct editor_sprite_resource;
struct editor_sound_resource;
struct editor_light_resource;
struct editor_particles_resource;
struct editor_wandering_pixels_resource;
struct editor_point_marker_resource;
struct editor_area_marker_resource;

struct editor_firearm_resource;
struct editor_ammunition_resource;
struct editor_melee_resource;
struct editor_explosive_resource;
struct editor_tool_resource;

struct editor_material_resource;

struct editor_prefab_resource;
struct editor_game_mode_resource;

using external_editor_resource_types = type_list<
	editor_sprite_resource,
	editor_sound_resource
>;

using internal_editor_resource_types = type_list<
	editor_light_resource,
	editor_particles_resource,
	editor_wandering_pixels_resource,
	editor_point_marker_resource,
	editor_area_marker_resource,

	editor_firearm_resource,
	editor_ammunition_resource,
	editor_melee_resource,
	editor_explosive_resource,
	editor_tool_resource,

	editor_material_resource,

	editor_prefab_resource,
	editor_game_mode_resource
>;

using all_editor_resource_types = concatenate_lists_t<
	external_editor_resource_types,
	internal_editor_resource_types
>;
