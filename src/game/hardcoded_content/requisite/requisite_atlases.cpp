#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"
#include "game/assets/gl_texture_id.h"
#include "application/config_lua_table.h"

#include <imgui/imgui.h>

using namespace assets;

void load_requisite_atlases(assets_manager& manager, const config_lua_table& cfg) {
	manager.create(
		gl_texture_id::GAME_WORLD_ATLAS,
		cfg.save_regenerated_atlases_as_binary
	);

	auto& io = ImGui::GetIO();
	augs::image imgui_atlas;

	{
		unsigned char* pixels = nullptr;
		int width = 0;
		int height = 0;
		
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		io.Fonts->TexID = reinterpret_cast<void*>(gl_texture_id::IMGUI_ATLAS);
		imgui_atlas.create_from(pixels, 4, 0, vec2i{ width, height });
	}

	manager[gl_texture_id::IMGUI_ATLAS].create(imgui_atlas);
}