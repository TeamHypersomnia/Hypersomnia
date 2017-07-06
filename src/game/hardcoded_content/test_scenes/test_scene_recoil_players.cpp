#include "generated/setting_build_test_scenes.h"

#if BUILD_TEST_SCENES
#include <array>

#include "game/hardcoded_content/all_hardcoded_content.h"

#include "game/assets/recoil_player.h"
#include "game/assets/recoil_player_id.h"
#include "game/assets/assets_manager.h"


#if 0 // recoil generators
#include <type_traits>
#include <random>
#include <cmath>
#include <iostream>

#include "augs/misc/randomization.h"

namespace {
	using offsets_type = decltype(std::declval<recoil_player>().offsets);

	offsets_type generate_generic_recoil_offsets(const std::size_t quantity, const std::size_t seed = 1236011) {
		randomization rng{seed};
		std::normal_distribution<float> normal_distr{0.0f, 0.3f};
		offsets_type result;
		for(std::size_t i = 0; i != quantity; ++i) {
			float random_fraction = 0;
			do {
				random_fraction = normal_distr(rng.generator);
			} while(std::abs(random_fraction) > 1);

			result.push_back(-rng.randval(650, 850) * vec2{}.set_from_degrees(90 * random_fraction));
		}

		return result;
	}

	void dump_offsets(const offsets_type& offsets, std::ostream& out) {
		out << "const std::array<vec2, " << offsets.size() << "> offsets{{\n";
		for(auto const offset: offsets) {
			out << "\t{" << offset.x << "f, " << offset.y << "f},\n";
		}
		out << "}};\n";
	}
} // anonymous namespace
#endif // recoil generators

void load_test_scene_recoil_players(assets_manager& assets) {
	{
		auto& generic_recoil = assets[assets::recoil_player_id::GENERIC];

		//auto const& offsets = generate_generic_recoil_offsets(20);
		//dump_offsets(offsets, std::cout);
		//generic_recoil.offsets = offsets;

		const std::array<vec2, 20> offsets{{
				{-511.85f, 548.188f},
				{-647.486f, 84.6849f},
				{-671.111f, -72.3549f},
				{-806.213f, 184.256f},
				{-617.287f, 424.214f},
				{-708.705f, -175.035f},
				{-673.126f, -121.477f},
				{-739.059f, -154.329f},
				{-643.806f, 96.5152f},
				{-694.891f, 112.884f},
				{-680.94f, 309.556f},
				{-561.887f, -338.554f},
				{-787.314f, -65.0871f},
				{-810.315f, -33.317f},
				{-649.138f, 453.744f},
				{-751.828f, 207.736f},
				{-645.791f, -344.643f},
				{-701.453f, 88.3911f},
				{-752.176f, -360.259f},
				{-635.631f, 396.199f},
		}};
		generic_recoil.offsets.assign(offsets.begin(), offsets.end());
	}
}
#endif