#include "augs/string/format_enum.h"
#include "test_scenes/test_scene_recoil_players.h"
#include "test_scenes/test_scenes_content.h"
#include "augs/misc/pool/pool_allocate.h"

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

void load_test_scene_recoil_players(recoil_players_pool& all_definitions) {
	using test_id_type = test_scene_recoil_id;

	all_definitions.reserve(enum_count(test_id_type()));

	{
		recoil_player recoil;
		populate_with_uniform_offsets(recoil, 20);

		const auto test_id = test_scene_recoil_id::GENERIC;
		recoil.name = format_enum(test_id);

		const auto id = to_recoil_id(test_id);
		const auto new_allocation = all_definitions.allocate(std::move(recoil));

		ensure_eq(new_allocation.key, id);
	}
}
