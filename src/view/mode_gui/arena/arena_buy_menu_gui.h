#pragma once
#include "game/modes/mode_commands/item_purchase.h"
#include "augs/misc/enum/enum_array.h"
#include "game/assets/ids/asset_ids.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/detail/economy/money_type.h"

class images_in_atlas_map;
struct app_ingame_intent_input;

struct arena_buy_menu_gui {
	struct input {
		const const_entity_handle& character;
		assets::image_id money_icon;
		money_type available_money;
	};

	// GEN INTROSPECTOR struct arena_buy_menu_gui
	bool show = false;
	// END GEN INTROSPECTOR

	/* Always initialize as hidden */

	bool control(app_ingame_intent_input);
	std::optional<mode_commands::item_purchase> perform_imgui(input);
};
