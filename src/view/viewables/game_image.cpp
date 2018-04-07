#include "view/viewables/game_image.h"
#include "view/viewables/regeneration/game_image_loadables.h"

game_image_cache::game_image_cache(
	const game_image_loadables& loadables,
	const game_image_meta& meta
) { 
	original_image_size = loadables.read_source_image_size();
	partitioned_shape.make_box(vec2(original_image_size));
}

loaded_game_image_caches_map::loaded_game_image_caches_map(
	const game_image_loadables_map& loadables,
	const game_image_metas_map& metas
) { 
	for (const auto& l : loadables) {
		game_image_cache ch(l.second, metas.at(l.first));

		emplace(l.first, ch);
	}
}
