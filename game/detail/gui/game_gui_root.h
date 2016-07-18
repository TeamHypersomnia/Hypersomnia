#pragma once
#include "game/step.h"
#include "special_drag_and_drop_target.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/rect_world.h"
#include "drag_and_drop.h"
#include "aabb_highlighter.h"

class fixed_step;

struct game_gui_root : public augs::gui::rect {
	augs::gui::rect parent_of_inventory_controls;

	game_gui_root();
};

class gui_system;
struct game_gui_world : public augs::gui::rect_world {
};