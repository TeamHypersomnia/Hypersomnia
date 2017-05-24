#pragma once
// disables the warning due to type name length exceeded
#pragma warning(disable : 4503)

#include "augs/templates/type_list.h"

namespace components {
	struct tree_of_npo_node;
	struct special_physics;
	struct animation;
	struct behaviour_tree;
	struct position_copying;
	struct crosshair;
	struct missile;
	struct flags;
	struct gun;
	struct rotation_copying;
	struct movement;
	struct particles_existence;
	struct pathfinding;
	struct rigid_body;
	struct render;
	struct transform;
	struct sprite;
	struct polygon;
	struct tile_layer_instance;
	struct car;
	struct driver;
	struct fixtures;
	struct container;
	struct item;
	struct force_joint;
	struct item_slot_transfers;
	struct name;
	struct trace;
	struct melee;
	struct sentience;
	struct attitude;
	struct processing;
	struct guid;
	struct child;
	struct interpolation;
	struct light;
	struct wandering_pixels;
	struct sound_existence;
	struct explosive;
	struct catridge;
	struct shape_polygon;
	struct shape_circle;
	struct motor_joint;
	struct hand_fuse;
	struct sender;

	struct all_inferred_state;
}

template<template<typename...> class List, class... prepend>
struct put_all_components_into {
	typedef List<prepend...,
		components::tree_of_npo_node,
		components::special_physics,
		components::animation,
		components::behaviour_tree,
		components::position_copying,
		components::crosshair,
		components::missile,
		components::flags,
		components::gun,
		components::rotation_copying,
		components::movement,
		components::particles_existence,
		components::pathfinding,
		components::rigid_body,
		components::render,
		components::transform,
		components::sprite,
		components::polygon,
		components::tile_layer_instance,
		components::car,
		components::driver,
		components::fixtures,
		components::container,
		components::item,
		components::force_joint,
		components::item_slot_transfers,
		components::name,
		components::trace,
		components::melee,
		components::sentience,
		components::attitude,
		components::processing,
		components::guid,
		components::child,
		components::interpolation,
		components::light,
		components::wandering_pixels,
		components::sound_existence,
		components::explosive,
		components::catridge,
		components::shape_polygon,
		components::shape_circle,
		components::motor_joint,
		components::hand_fuse,
		components::sender,

		components::all_inferred_state

	> type;
};

template<template<typename...> class List, class... prepend>
using put_all_components_into_t = typename put_all_components_into<List, prepend...>::type;

template<class... Types>
struct type_count {
	static const unsigned value = sizeof...(Types);
};

class cosmos;

namespace augs {
	template<class, class...>
	class operations_on_all_components_mixin;
}

constexpr unsigned COMPONENTS_COUNT = put_all_components_into_t<type_count>::value;