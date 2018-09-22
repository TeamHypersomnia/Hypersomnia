#pragma once
#include <unordered_set>
#include "view/mode_gui/arena/buy_menu_type.h"
#include "augs/misc/enum/enum_map.h"
#include "augs/window_framework/event.h"
#include "augs/misc/enum/enum_boolset.h"

using arena_buy_menu_hotkey_map = augs::enum_map<
	augs::event::keys::key, 
	buy_menu_type
>;

using arena_buy_menu_selected_weapons = std::unordered_set<int>;
using arena_buy_menu_requested_weapons = std::vector<int>;
