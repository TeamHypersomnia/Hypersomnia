#pragma once
#include "application/content_generation/atlas_content_structs.h"

struct game_image_request;

typedef source_font_loading_input game_font_request;

typedef augs::baked_font game_font_baked;

typedef std::unordered_map<assets::game_image_id, game_image_request> game_image_requests;
typedef std::unordered_map<assets::font_id, game_font_request> game_font_requests;

struct atlases_regeneration_output;