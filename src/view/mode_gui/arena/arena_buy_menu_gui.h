#pragma once
#include "game/modes/mode_commands/item_purchase.h"
#include "augs/misc/enum/enum_array.h"
#include "game/assets/ids/asset_ids.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/detail/economy/money_type.h"
#include "game/modes/detail/item_purchase_structs.h"
#include "augs/graphics/rgba.h"
#include "view/mode_gui/arena/buy_menu_type.h"
#include "view/mode_gui/arena/arena_buy_menu_hotkeys.h"

class images_in_atlas_map;
struct app_ingame_intent_input;
struct buy_menu_gui_settings;

struct arena_buy_menu_gui {
	struct input {
		const const_entity_handle& subject;
		assets::image_id money_icon;
		money_type available_money;
		const images_in_atlas_map& images_in_atlas;
		const rgba money_indicator_color;
		const bool is_in_buy_zone;
		const item_purchases_vector& done_purchases;
		const bool give_extra_mags_on_first_purchase;
		const buy_menu_gui_settings& settings;
	};

	// GEN INTROSPECTOR struct arena_buy_menu_gui
	bool show = false;
	buy_menu_type current_menu = buy_menu_type::MAIN;
	arena_buy_menu_selected_weapons selected_weapons;
	arena_buy_menu_selected_weapons selected_replenishables;

	arena_buy_menu_requested_weapons requested_weapons;
	arena_buy_menu_requested_weapons requested_replenishables;
	// END GEN INTROSPECTOR

	/* Always initialize as hidden */

	bool control(app_ingame_intent_input);
	bool escape();
	void hide();
	std::optional<mode_commands::item_purchase> perform_imgui(input);
};
