#include "augs/ensure_rel_util.h"
#include "augs/string/format_enum.h"
#include "test_scenes/test_scene_recoil_players.h"
#include "test_scenes/test_scenes_content.h"
#include "augs/misc/pool/pool_allocate.h"

#include "game/assets/recoil_player.h"
#include "game/assets/ids/asset_ids.h"
#include "game/assets/all_logical_assets.h"

void load_test_scene_recoil_players(recoil_players_pool& all_definitions) {
	using test_id_type = test_scene_recoil_id;

	all_definitions.reserve(enum_count(test_id_type()));

	{
		recoil_player recoil;
		recoil.offsets = {
			0.332977,
			0.432977,
			0.6,
			0.65,
			0.7,
			0.8,
			0.402178,
			0.202178,
			-0.341383,
			-0.841383,
			-0.915322,
			-0.708643,
			-0.57748,
			-0.232232,
			0.240072,
			0.42,
			0.602663,
			0.694861,
			-0.12629,
			-0.4,
			-0.666503,
			0.270903,
			0.6,
			0.9,
			0.2,
			-0.5,
			-1.0
		};

		const auto test_id = test_scene_recoil_id::GENERIC;
		recoil.name = format_enum(test_id);

		const auto id = to_recoil_id(test_id);
		const auto new_allocation = all_definitions.allocate(std::move(recoil));

		(void)id;
		(void)new_allocation;

		ensure_eq_id(new_allocation.key, id);
	}
}
