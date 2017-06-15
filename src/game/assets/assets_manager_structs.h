#pragma once
#include "application/content_generation/atlas_content_structs.h"

struct game_image_request;

using game_font_request = source_font_loading_input;
using game_font_baked = augs::baked_font;

using game_image_requests = std::unordered_map<assets::game_image_id, game_image_request>;
using game_font_requests = std::unordered_map<assets::font_id, game_font_request>;

struct atlases_regeneration_output;