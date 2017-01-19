#pragma once
namespace augs {
	class lua_state_raii;
}

void bind_game_and_augs(augs::lua_state_raii&);
