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
			0.18943500000000002,
			0.30000000000000004,
			0.303267,
			0.303267,
			0.348348,
			0.348348,
			0.360108,
			0.360108,
			0.4063545,
			0.5120745,
			0.5120745,
			0.6000000000000001,
			0.603267,
			0.603267,
			0.63,
			0.63,
			0.6494655,
			0.75,
			0.7994655,
			0.86622,
			0.86622,
			0.8999999999999999,
			0.8999999999999999,
			0.9039944999999999,
			0.9039944999999999,
			0.9750000000000001,
			0.9997544999999999,
			1.0422915,
			1.0422915,
			1.0499999999999998,
			1.0499999999999998,
			1.0629645,
			1.0629645,
			1.2000000000000002,
			1.2000000000000002,
			1.222983,
			1.222983,
			1.2620745,
			1.2620745
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
