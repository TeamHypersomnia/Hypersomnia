#include "game/hardcoded_content/all_hardcoded_content.h"

#include "game/assets/recoil_player.h"
#include "game/assets/recoil_player_id.h"
#include "game/assets/assets_manager.h"

#include <type_traits>
#include <random>
#include <cmath>
#include <iostream>

#include "augs/misc/randomization.h"

namespace {
	void populate_with_uniform_offsets(recoil_player& recoil_player, const std::size_t quantity, const std::size_t seed = 1236011) {
		randomization rng{seed};
		for(std::size_t i = 0; i != quantity; ++i) {
			recoil_player.offsets.push_back(rng.randval(recoil_player.fallback_random_magnitude));
		}
	}

	void dump_offsets(const recoil_player& recoil_player, std::ostream& out) {
		out << "const std::array<float, " << recoil_player.offsets.size() << "> offsets{{\n";
		for(auto const offset: recoil_player.offsets) {
			out << "\t" << offset << "f,\n";
		}
		out << "}};\n";
	}
} // anonymous namespace

void load_test_scene_recoil_players(assets_manager& assets) {
	{
		auto& generic_recoil = assets[assets::recoil_player_id::GENERIC];

		generic_recoil.fallback_random_magnitude = 5;
		populate_with_uniform_offsets(generic_recoil, 20);
//		dump_offsets(generic_recoil, std::cout);
	}
}
