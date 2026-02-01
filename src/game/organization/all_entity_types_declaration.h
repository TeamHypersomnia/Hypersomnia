#pragma once
#include <cstddef>
#include "augs/templates/type_in_list_id.h"
#include "augs/templates/folded_finders.h"

struct plain_sprited_body;
struct destructible_sprited_body;
struct controlled_character;
struct plain_missile;
struct shootable_weapon;
struct shootable_charge;
struct melee_weapon;
struct static_decoration;
struct dynamic_decoration;
struct wandering_pixels_decoration;
struct static_light;
struct hand_explosive;
struct finishing_trace;
struct container_item;
struct remnant_body;
struct sound_decoration;
struct particles_decoration;
struct point_marker;
struct area_marker;
struct explosion_body;
struct tool_item;
struct touch_collectible;
struct area_sensor;
struct decal_decoration;

using all_entity_types = type_list<
	plain_sprited_body,
	destructible_sprited_body,
	controlled_character,
	plain_missile,
	shootable_weapon,
	shootable_charge,
	melee_weapon,
	static_decoration,
	dynamic_decoration,
	wandering_pixels_decoration,
	static_light,
	hand_explosive,
	finishing_trace,
	container_item,
	remnant_body,
	sound_decoration,
	particles_decoration,
	point_marker,
	area_marker,
	explosion_body,
	tool_item,
	touch_collectible,
	area_sensor,
	decal_decoration
>;

#define FOR_ALL_ENTITY_TYPES(MACRO) \
	MACRO(plain_sprited_body) \
	MACRO(destructible_sprited_body) \
	MACRO(controlled_character) \
	MACRO(plain_missile) \
	MACRO(shootable_weapon) \
	MACRO(shootable_charge) \
	MACRO(melee_weapon) \
	MACRO(static_decoration) \
	MACRO(dynamic_decoration) \
	MACRO(wandering_pixels_decoration) \
	MACRO(static_light) \
	MACRO(hand_explosive) \
	MACRO(finishing_trace) \
	MACRO(container_item) \
	MACRO(remnant_body) \
	MACRO(sound_decoration) \
	MACRO(particles_decoration) \
	MACRO(point_marker) \
	MACRO(area_marker) \
	MACRO(explosion_body) \
	MACRO(tool_item) \
	MACRO(touch_collectible) \
	MACRO(area_sensor) \
	MACRO(decal_decoration)

using entity_type_id = type_in_list_id<all_entity_types>;

constexpr std::size_t ENTITY_TYPES_COUNT = num_types_in_list_v<all_entity_types>;

template <class T>
constexpr std::size_t ENTITY_TYPE_IDX = index_in_list_v<T, all_entity_types>;
