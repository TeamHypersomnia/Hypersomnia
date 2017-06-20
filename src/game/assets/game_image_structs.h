#pragma once
#include "application/content_generation/atlas_content_structs.h"

struct game_image_definition;

using game_font_definition = source_font_loading_input;
using game_font_baked = augs::baked_font;

using game_image_definitions = std::unordered_map<std::string, game_image_definition>;
using game_font_definitions = std::unordered_map<std::string, game_font_definition>;
