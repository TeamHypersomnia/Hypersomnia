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
			0.43297708034515381,
			0.9932628870010376,
			-0.45482802391052246,
			-0.072165310382843018,
			0.60215318202972412,
			0.33908355236053467,
			0.40217757225036621,
			-0.84138309955596924,
			-0.61532247066497803,
			-0.70864325761795044,
			-0.57748019695281982,
			0.23223221302032471,
			0.54007184505462646,
			0.047597408294677734,
			0.60266327857971191,
			0.69486057758331299,
			-0.42628997564315796,
			0.94623100757598877,
			-0.96650326251983643,
			0.47090291976928711
		};

		const auto test_id = test_scene_recoil_id::GENERIC;
		recoil.name = format_enum(test_id);

		const auto id = to_recoil_id(test_id);
		const auto new_allocation = all_definitions.allocate(std::move(recoil));

		(void)id;
		(void)new_allocation;

		ensure_eq(new_allocation.key, id);
	}
}
