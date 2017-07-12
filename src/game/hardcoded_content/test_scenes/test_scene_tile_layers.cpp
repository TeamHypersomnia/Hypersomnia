#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"

void load_test_scene_tile_layers(assets_manager& manager) {
	{
		auto nth_tile = [](const unsigned i) {
			return i;
			//return assets::game_image_id(unsigned(assets::game_image_id::CATHEDRAL_TILE_1) + i - 1);
		};

		auto& metropolis_floor = manager[assets::tile_layer_id::METROPOLIS_FLOOR];

		for (int i = int(assets::game_image_id::CATHEDRAL_TILE_1); i <= int(assets::game_image_id::CATHEDRAL_TILE_49); ++i) {
			components::sprite tt;
			tt.set(assets::game_image_id(i), manager[assets::game_image_id(i)].get_size());
			//if (
			//	assets::game_image_id(i) == assets::game_image_id(int(assets::game_image_id::CATHEDRAL_TILE_1) + 3)
			//	//|| assets::game_image_id(i) == assets::game_image_id(int(assets::game_image_id::CATHEDRAL_TILE_1) + 4)
			//	) {
			//	tt.effect = components::sprite::special_effect::COLOR_WAVE;
			//}		
			if (
				assets::game_image_id(i) == assets::game_image_id(int(assets::game_image_id::CATHEDRAL_TILE_1) + 0)
				) {
				tt.max_specular_blinks = 2;
			}
			metropolis_floor.register_tile_type(
				manager, 
				tt
			);
		}

		//metropolis_floor.get_tile_type(nth_tile(1)).max_specular_blinks = 2;

		tile_layer::tile_rectangular_filling gold_floor_filling;
		gold_floor_filling.fill = nth_tile(49);

		gold_floor_filling.left_border = nth_tile(2);
		gold_floor_filling.top_border = nth_tile(13);
		gold_floor_filling.bottom_border = nth_tile(24);
		gold_floor_filling.right_border = nth_tile(8);

		gold_floor_filling.lt_corner = nth_tile(6);
		gold_floor_filling.rt_corner = nth_tile(7);
		gold_floor_filling.rb_corner = nth_tile(12);
		gold_floor_filling.lb_corner = nth_tile(11);

		metropolis_floor.set_tiles_alternating(xywhu(1 + 0, 0, 10, 30), nth_tile(4), nth_tile(5));
		metropolis_floor.set_tiles_alternating(xywhu(1 + 30, 0, 10, 30), nth_tile(4), nth_tile(5));

		metropolis_floor.set_tiles(xywhu(1 + 10 + 0, 20, 5, 21), gold_floor_filling);
		metropolis_floor.set_tiles(xywhu(1 + 10 + 5, 20, 10, 21), nth_tile(1));
		metropolis_floor.set_tiles(xywhu(1 + 10 + 15, 20, 5, 21), gold_floor_filling);
		metropolis_floor.set_tiles(xywhu(1 + 10 + 0, 0, 20, 20), nth_tile(1));

		metropolis_floor.set_tiles(xywhu(0, 0, 1, 30), nth_tile(3));
		metropolis_floor.set_tiles(xywhu(10, 30, 1, 11), nth_tile(3));
		metropolis_floor.set_tiles(xywhu(0, 30, 10, 1), nth_tile(3));

		metropolis_floor.set_tiles(xywhu(31 + 10, 0, 1, 30), nth_tile(3));
		metropolis_floor.set_tiles(xywhu(31 + 10 - 10, 30, 1, 11), nth_tile(3));
		metropolis_floor.set_tiles(xywhu(31 + 1, 30, 10, 1), nth_tile(3));
	}
}