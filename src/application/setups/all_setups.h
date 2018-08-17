#pragma once
#include <variant>
#include "application/setups/main_menu_setup.h"
#include "application/setups/test_scene_setup.h"
#include "application/setups/editor/editor_setup.h"

using setup_variant = std::variant<
	test_scene_setup,
	editor_setup
>;
