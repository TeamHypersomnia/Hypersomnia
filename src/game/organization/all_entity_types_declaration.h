#pragma once
#include "augs/templates/type_in_list_id.h"
#include "augs/templates/folded_finders.h"

struct plain_sprited_body;
struct controlled_character;
struct plain_missile;
struct shootable_weapon;
struct shootable_charge;
struct melee_weapon;
struct sprite_decoration;
struct complex_decoration;
struct wandering_pixels_decoration;
struct static_light;
struct hand_explosive;
struct finishing_trace;
struct container_item;
struct explosive_missile;
struct remnant_body;
struct sound_decoration;
struct particles_decoration;
struct point_marker;
struct box_marker;
struct explosion_body;
struct tool_item;

using all_entity_types = type_list<
	plain_sprited_body,
	controlled_character,
	plain_missile,
	shootable_weapon,
	shootable_charge,
	melee_weapon,
	sprite_decoration,
	complex_decoration,
	wandering_pixels_decoration,
	static_light,
	hand_explosive,
	finishing_trace,
	container_item,
	explosive_missile,
	remnant_body,
	sound_decoration,
	particles_decoration,
	point_marker,
	box_marker,
	explosion_body,
	tool_item
>;

using entity_type_id = type_in_list_id<all_entity_types>;

constexpr std::size_t ENTITY_TYPES_COUNT = num_types_in_list_v<all_entity_types>;

template <class T>
constexpr std::size_t ENTITY_TYPE_IDX = index_in_list_v<T, all_entity_types>;
