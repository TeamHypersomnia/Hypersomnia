#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"
#include "game/assets/gl_texture_id.h"
#include "application/config_lua_table.h"

using namespace assets;

void load_requisite_atlases(assets_manager& manager, const config_lua_table& cfg) {
	manager.create(
		gl_texture_id::GAME_WORLD_ATLAS,
		cfg.save_regenerated_atlases_as_binary
	);
}