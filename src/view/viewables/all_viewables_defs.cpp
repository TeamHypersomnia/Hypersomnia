#include <Box2D/Box2D.h>

#include "augs/templates/algorithm_templates.h"
#include "game/assets/all_logical_assets.h"
#include "view/viewables/all_viewables_defs.h"

#include "augs/templates/introspect.h"

const all_viewables_defs all_viewables_defs::empty {};

void all_viewables_defs::clear() {
	augs::introspect([](auto, auto& c){ c.clear(); }, *this);
}

std::optional<assets::image_id> find_asset_id_by_path(
	const maybe_official_path& p,
	const image_definitions_map& loadables
) {
	std::optional<assets::image_id> result_id;

	loadables.for_each_object_and_id(
		[&result_id, &p](const auto& l, const auto id) {
			if (p == l.get_source_path()) {
				result_id = assets::image_id(id);
			}
		}
	);

	return result_id;
}
