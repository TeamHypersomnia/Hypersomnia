#pragma once
#include "game/assets/ids/asset_ids.h"
#include "augs/templates/folded_finders.h"

namespace augs {
	template <class...>
	class history_with_marks;
};

struct fill_with_test_scene_command;

class move_entities_command;
class flip_entities_command;
class resize_entities_command;

struct paste_entities_command;
struct delete_entities_command;
struct duplicate_entities_command;

struct create_flavour_command;
struct duplicate_flavour_command;
struct delete_flavour_command;
struct change_flavour_property_command;
struct change_initial_component_property_command;

struct instantiate_flavour_command;

struct change_entity_property_command;
struct change_common_state_command;

struct change_grouping_command;
struct change_group_property_command;

struct change_current_mode_property_command;
struct change_mode_vars_property_command;
struct change_mode_player_property_command;

template <class>
struct create_pathed_asset_id_command;

template <class>
struct create_unpathed_asset_id_command;

template <class>
struct forget_asset_id_command;

template <class>
struct change_asset_property_command;

template <class>
struct duplicate_asset_command;

using editor_history_base = augs::history_with_marks<
	fill_with_test_scene_command,

	move_entities_command,
	flip_entities_command,
	resize_entities_command,

	paste_entities_command,
	delete_entities_command,
	duplicate_entities_command,

	create_flavour_command,
	duplicate_flavour_command,
	delete_flavour_command,
	change_flavour_property_command,
	change_initial_component_property_command,
	instantiate_flavour_command,

	change_entity_property_command,
	change_common_state_command,

	change_grouping_command,
	change_group_property_command,

	change_mode_vars_property_command,

	create_pathed_asset_id_command<assets::image_id>,
	forget_asset_id_command<assets::image_id>,
	change_asset_property_command<assets::image_id>,

	create_pathed_asset_id_command<assets::sound_id>,
	forget_asset_id_command<assets::sound_id>,
	change_asset_property_command<assets::sound_id>,

	create_unpathed_asset_id_command<assets::plain_animation_id>,
	forget_asset_id_command<assets::plain_animation_id>,
	duplicate_asset_command<assets::plain_animation_id>,
	change_asset_property_command<assets::plain_animation_id>,

	create_unpathed_asset_id_command<assets::particle_effect_id>,
	forget_asset_id_command<assets::particle_effect_id>,
	duplicate_asset_command<assets::particle_effect_id>,
	change_asset_property_command<assets::particle_effect_id>,

	/* Playtest-specific */

	change_current_mode_property_command,
	change_mode_player_property_command
>;

template <class T>
constexpr bool is_playtest_specific_v = is_one_of_v<T,
	change_current_mode_property_command,
	change_mode_player_property_command
>;
