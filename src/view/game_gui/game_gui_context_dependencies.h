#pragma once
#include "view/viewables/all_viewables_declaration.h"
#include "view/necessary_resources.h"
#include "view/game_gui/elements/hotbar_settings.h"

namespace augs {
	struct baked_font;
}

struct game_gui_settings;
struct randomizing_system;

struct game_gui_context_dependencies {
	const image_definitions_map& image_definitions;
	const images_in_atlas_map& game_images;
	const necessary_images_in_atlas_map& necessary_images;
	const augs::baked_font& gui_font;
	randomizing_system& randomizing;
	const game_gui_settings& game_gui;
	const hotbar_settings hotbar;
};