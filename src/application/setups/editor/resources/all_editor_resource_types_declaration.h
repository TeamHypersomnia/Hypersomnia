#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_mod_templates.h"

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

struct editor_material_resource;

struct editor_prefab_resource;

using all_editor_resource_types = type_list<
	editor_sprite_resource,
	editor_sound_resource,
	editor_light_resource,
	editor_particles_resource,
	editor_wandering_pixels_resource,
	editor_point_marker_resource,
	editor_area_marker_resource,

	editor_firearm_resource,
	editor_ammunition_resource,
	editor_melee_resource,
	editor_explosive_resource,

	editor_material_resource,

	editor_prefab_resource
>;
