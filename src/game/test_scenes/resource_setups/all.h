#pragma once
#include <tuple>
#include "application/content_generation/texture_atlases.h"
#include "game/assets/assets_manager_structs.h"

class config_lua_table;
class assets_manager;
class cosmos;
struct cosmos_global_state;

void load_all_requisite(const config_lua_table&);
void create_standard_opengl_resources(const config_lua_table&);

game_image_requests load_requisite_images();
game_font_requests load_requisite_fonts();

void set_standard_sound_buffers(assets_manager&);

void set_requisite_animations(assets_manager&);

void set_test_scene_particle_effects(assets_manager&);
void set_test_scene_physical_materials(assets_manager&);

void set_test_scene_sentience_properties(
	cosmos_global_state&
);

void set_test_scene_tile_layers(assets_manager&);