#pragma once
#include "components_instantiation.h"

typedef typename put_all_components_into<augs::storage_for_components_and_aggregates>::type
storage_for_all_components_and_aggregates;

template class storage_for_all_components_and_aggregates::basic_aggregate_handle<false>;
template class storage_for_all_components_and_aggregates::basic_aggregate_handle<true>;

template class augs::storage_for_components_and_aggregates <
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
	components::item_slot_transfers,
	components::gui_element,
	components::trigger_collision_detector,
	components::name,
	components::trace,
	components::melee,
	components::sentience,
	components::attitude,
	components::relations
>;

template <bool is_const>
using basic_aggregate_handle = storage_for_all_components_and_aggregates::basic_aggregate_handle<is_const>;