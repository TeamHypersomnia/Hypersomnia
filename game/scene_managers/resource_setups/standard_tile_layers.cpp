#include "all.h"
#include "game/resources/manager.h"

namespace resource_setups {
	void load_standard_tile_layers() {
		{
			auto nth_tile = [](unsigned i) {
				return assets::texture_id(unsigned(assets::texture_id::METROPOLIS_TILE_FIRST) + i - 1);
			};

			auto& metropolis_floor = resource_manager.create(assets::tile_layer_id::METROPOLIS_FLOOR);

			metropolis_floor.set_tiles(xywhu(0, 0, 20, 20), nth_tile(1));
			metropolis_floor.set_tiles(xywhu(20, 0, 20, 20), nth_tile(3));

		}
	}
}