#pragma once
#include "entity_system/storage_for_components_and_aggregates.h"

namespace components {
	struct animation;
	struct animation_response;
	struct behaviour_tree;
	struct camera;
	struct position_copying;
	struct crosshair;
	struct damage;
	struct gun;
	struct input_receiver;
	struct rotation_copying;
	struct movement;
	struct particle_effect_response;
	struct particle_group;
	struct pathfinding;
	struct physics;
	struct render;
	struct steering;
	struct transform;
	struct visibility;
	struct sprite;
	struct polygon;
	struct tile_layer;
	struct car;
	struct driver;
	struct trigger;
	struct trigger_query_detector;
	struct fixtures;
	struct container;
	struct item;
	struct force_joint;
	struct physics_definition;
	struct item_slot_transfers;
	struct gui_element;
	struct trigger_collision_detector;
	struct name;
	struct trace;
	struct melee;
	struct sentience;
	struct attitude;
}

typedef augs::storage_for_components_and_aggregates <
	components::animation,
	components::animation_response,
	components::behaviour_tree,
	components::camera,
	components::position_copying,
	components::crosshair,
	components::damage,
	components::gun,
	components::input_receiver,
	components::rotation_copying,
	components::movement,
	components::particle_effect_response,
	components::particle_group,
	components::pathfinding,
	components::physics,
	components::render,
	components::steering,
	components::transform,
	components::visibility,
	components::sprite,
	components::polygon,
	components::tile_layer,
	components::car,
	components::driver,
	components::trigger,
	components::trigger_query_detector,
	components::fixtures,
	components::container,
	components::item,
	components::force_joint,
	components::physics_definition,
	components::item_slot_transfers,
	components::gui_element,
	components::trigger_collision_detector,
	components::name,
	components::trace,
	components::melee,
	components::sentience,
	components::attitude
> world;