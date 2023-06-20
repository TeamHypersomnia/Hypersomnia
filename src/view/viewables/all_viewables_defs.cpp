#include "3rdparty/Box2D/Box2D.h"

#include "augs/templates/algorithm_templates.h"
#include "game/assets/all_logical_assets.h"
#include "view/viewables/all_viewables_defs.h"

const all_viewables_defs all_viewables_defs::empty {};

void all_viewables_defs::clear() {
	sounds.clear();
	particle_effects.clear();
	image_definitions.clear();
}

template <class I, class P>
std::optional<I> find_asset_id_by_path(
	const maybe_official_path<I>& searched_path,
	const P& definitions
) {
	using K = typename P::key_type;

	std::optional<K> result_id;

	for_each_id_and_object(definitions,
		[&result_id, &searched_path](const auto id, const auto& l) {
			if (searched_path == l.get_source_path()) {
				result_id = id;
			}
		}
	);

	return result_id;
}


void all_viewables_defs::update_relevant(all_logical_assets& logicals) const {
	for_each_id_and_object(
		image_definitions,
		[&](const auto id, const auto& l) {
			logicals.image_offsets.at(id.get_cache_index()) = l.meta.offsets;
		}
	);
}

template std::optional<assets::image_id> find_asset_id_by_path(const maybe_official_image_path& p, const image_definitions_map& definitions);
template std::optional<assets::sound_id> find_asset_id_by_path(const maybe_official_sound_path& p, const sound_definitions_map& definitions);
