#pragma once
// disables the warning due to type name length exceeded
#pragma warning(disable : 4503)

namespace components {
	struct dynamic_tree_node;
	struct special_physics;
	struct animation;
	struct animation_response;
	struct behaviour_tree;
	struct position_copying;
	struct crosshair;
	struct damage;
	struct flags;
	struct gun;
	struct rotation_copying;
	struct movement;
	struct particle_effect_response;
	struct particles_existence;
	struct pathfinding;
	struct physics;
	struct render;
	struct transform;
	struct sprite;
	struct polygon;
	struct tile_layer_instance;
	struct car;
	struct driver;
	struct trigger;
	struct trigger_query_detector;
	struct fixtures;
	struct container;
	struct item;
	struct force_joint;
	struct item_slot_transfers;
	struct gui_element;
	struct trigger_collision_detector;
	struct name;
	struct trace;
	struct melee;
	struct sentience;
	struct attitude;
	struct processing;
	struct guid;
	struct child;
	struct sub_entities;
	struct physical_relations;
	struct interpolation;
	struct light;
	struct wandering_pixels;
	struct substance;
}

template<template<typename...> class List, class... prepend>
struct put_all_components_into {
	typedef List<prepend...,
		components::dynamic_tree_node,
		components::special_physics,
		components::animation,
		components::animation_response,
		components::behaviour_tree,
		components::position_copying,
		components::crosshair,
		components::damage,
		components::flags,
		components::gun,
		components::rotation_copying,
		components::movement,
		components::particle_effect_response,
		components::particles_existence,
		//components::pathfinding,
		components::physics,
		components::render,
		components::transform,
		components::sprite,
		components::polygon,
		components::tile_layer_instance,
		components::car,
		components::driver,
		components::trigger,
		components::trigger_query_detector,
		components::fixtures,
		components::container,
		components::item,
		components::force_joint,
		components::item_slot_transfers,
		components::gui_element,
		components::trigger_collision_detector,
		components::name,
		components::trace,
		components::melee,
		components::sentience,
		components::attitude,
		components::processing,
		components::guid,
		components::child,
		components::sub_entities,
		components::physical_relations,
		components::interpolation,
		components::light,
		components::wandering_pixels,

		components::substance

	> type;
};

namespace std {
	template<class...>
	class tuple;
}

typedef std::tuple<
	components::pathfinding
> disabled_components;

template<class... Types>
struct type_count {
	static const unsigned value = sizeof...(Types);
};

class cosmos;

namespace augs {
	template<class, class...>
	class operations_on_all_components_mixin;
}

constexpr unsigned COMPONENTS_COUNT = put_all_components_into<type_count>::type::value;