#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"
#include "game/assets/gl_texture_id.h"
#include "application/config_lua_table.h"

#include <imgui/imgui.h>

using namespace assets;

void load_requisite_atlases(assets_manager& manager) {
	manager.load_requisite(gl_texture_id::GAME_WORLD_ATLAS);

	auto& io = ImGui::GetIO();

	unsigned char* pixels = nullptr;
	int width = 0;
	int height = 0;
	
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	io.Fonts->TexID = reinterpret_cast<void*>(gl_texture_id::IMGUI_ATLAS);

	manager.get_store_by<gl_texture_id>().emplace(
		gl_texture_id::IMGUI_ATLAS, 
		augs::image(pixels, 4, 0, vec2i{ width, height })
	);
}