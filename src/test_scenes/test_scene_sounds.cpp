#include "augs/ensure_rel_util.h"
#include "augs/misc/enum/enum_map.h"
#include "augs/misc/pool/pool_allocate.h"
#include "augs/templates/enum_introspect.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/filesystem/file.h"
#include "augs/audio/sound_buffer.h"

#include "game/assets/ids/asset_ids.h"

#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_sounds.h"

#include "view/viewables/sound_definition.h"

#include "game/enums/battle_event.h"
#include "game/enums/faction_type.h"
#include "augs/filesystem/find_path.h"
#include "augs/log.h"

void load_test_scene_sounds(sound_definitions_map& all_definitions) {
	using test_id_type = test_scene_sound_id;

	all_definitions.reserve(enum_count(test_id_type()));

	augs::for_each_enum_except_bounds([&](const test_scene_sound_id enum_id) {
		const auto id = to_sound_id(enum_id);

		if (found_in(all_definitions, id)) {
			return;
		}

		const auto stem = augs::path_type("sfx") / to_lowercase(augs::enum_to_string(enum_id));
		using path = augs::path_type;

		sound_definition definition;


		auto try_with = [&](const auto& tried_stem) {
			const auto with_existent_ext = augs::first_existing_extension(OFFICIAL_CONTENT_PATH / path(tried_stem), augs::SOUND_EXTENSIONS);

			if (!with_existent_ext.empty()) {
				maybe_official_sound_path m;
				m.is_official = true;
				m.path = path(tried_stem);
				m.path += with_existent_ext.extension();
				definition.set_source_path(m);
			}
		};

		try_with(stem);
		try_with(path(stem) += "_1");

		if (!definition.get_source_path().path.empty()) {
			const auto new_allocation = all_definitions.allocate(std::move(definition));
			(void)new_allocation;
			(void)id;
			ensure_eq_id(new_allocation.key, id);
		}
		else {
			throw test_scene_asset_loading_error(
				"Failed to load %x: sound file was not found at %x.", 
				stem, 
				OFFICIAL_CONTENT_PATH / stem
			);
		}
	});
}