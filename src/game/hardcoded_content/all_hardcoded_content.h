#pragma once
#include <tuple>
#include "application/content_generation/texture_atlases.h"
#include "game/assets/game_image_structs.h"

class config_lua_table;
class assets_manager;
class cosmos;
struct cosmos_global_state;

void load_all_requisite(const config_lua_table&);
void load_requisite_animations(assets_manager&);
void load_requisite_atlases(assets_manager&, const config_lua_table&);
void load_requisite_shaders(assets_manager&);
void load_requisite_sound_buffers(assets_manager&);

// test scene content

void load_test_scene_animations(assets_manager&);
void load_test_scene_particle_effects(assets_manager&);
void load_test_scene_physical_materials(assets_manager&);
void load_test_scene_sentience_properties(cosmos_global_state&);
void load_test_scene_sound_buffers(assets_manager&);
void load_test_scene_tile_layers(assets_manager&);
void load_test_scene_recoil_players(assets_manager&);
