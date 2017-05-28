#pragma once
#include <tuple>
#include "application/content_generation/texture_atlases.h"
#include "game/assets/assets_manager_structs.h"

class config_lua_table;
class assets_manager;
class cosmos;
struct cosmos_global_state;

void load_standard_everything(const config_lua_table&);
void create_standard_opengl_resources(const config_lua_table&);

game_image_requests load_standard_images();
game_font_requests load_standard_fonts();

void set_standard_behaviour_trees(cosmos&);

void set_standard_sound_buffers(assets_manager&);
void set_standard_particle_effects(assets_manager&);
void set_standard_physical_materials(assets_manager&);

void set_standard_sentience_properties(
	cosmos_global_state&
);

void set_standard_animations(assets_manager&);
void set_standard_tile_layers(assets_manager&);