#pragma once
#include "game/cosmos/cosmos.h"
#include "augs/misc/lua/lua_utils.h"
#include "augs/readwrite/lua_file.h"
#include "augs/readwrite/to_bytes.h"

template <class T>
void dump_for_debugging(
	sol::state& lua,
	const std::string& preffix,
	const cosmos& cosm,
	const T& mode
) {
	auto as_text = [&](const auto& bytes) {
		std::string ss;

		for (const auto& b : bytes) {
			ss += std::to_string(int(b)) + "\n";
		}

		return ss;
	};

	auto make_path = [&](const auto& of) {
		return augs::path_type(preffix + of);
	};

	const auto& signi = cosm.get_solvable().significant;

	augs::save_as_text(make_path("signi_bytes.txt"), as_text(augs::to_bytes(signi)));
	augs::save_as_text(make_path("mode_bytes.txt"), as_text(augs::to_bytes(mode)));

	augs::save_as_lua_table(lua, signi, make_path("signi.lua"));
	augs::save_as_lua_table(lua, mode, make_path("mode.lua"));
}
