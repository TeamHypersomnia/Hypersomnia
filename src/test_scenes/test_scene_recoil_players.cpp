#include "test_scenes/test_scenes_content.h"

#include "game/assets/recoil_player.h"
#include "game/assets/ids/asset_ids.h"
#include "game/assets/all_logical_assets.h"

#include "augs/misc/randomization.h"

static void populate_with_uniform_offsets(
	recoil_player& recoil_player,
   	const std::size_t quantity,
   	const rng_seed_type seed = 1236011
) {
	randomization rng{seed};

	for(std::size_t i = 0; i != quantity; ++i) {
		const auto nv = std::uniform_real_distribution<float>(-1.f, 1.f)(rng.generator);
		recoil_player.offsets.push_back(nv);
		
	}
}

void load_test_scene_recoil_players(recoil_players_pool& assets) {
	{
		auto& generic_recoil = assets[assets::recoil_player_id::GENERIC];
		populate_with_uniform_offsets(generic_recoil, 20);
	}
}
