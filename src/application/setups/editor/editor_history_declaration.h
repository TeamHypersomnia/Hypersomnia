#pragma once
#include "game/assets/ids/asset_ids.h"

namespace augs {
	template <class...>
	class history_with_marks;
};

struct delete_entities_command;
struct change_flavour_property_command;
struct fill_with_test_scene_command;
struct change_entity_property_command;
struct change_common_state_command;
struct move_entities_command;
struct paste_entities_command;
struct duplicate_entities_command;
struct change_grouping_command;
struct change_group_property_command;

template <class T>
struct create_asset_id_command;

template <class T>
struct forget_asset_id_command;

using editor_history_base = augs::history_with_marks<
	delete_entities_command,
	fill_with_test_scene_command,
	change_flavour_property_command,
	change_entity_property_command,
	change_common_state_command,
	move_entities_command,
	paste_entities_command,
	duplicate_entities_command,
	change_grouping_command,
	change_group_property_command,
	create_asset_id_command<assets::image_id>,
	forget_asset_id_command<assets::image_id>
	/* forget_image_path_command */
>;
