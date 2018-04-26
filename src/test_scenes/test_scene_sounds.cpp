#include "augs/misc/enum/enum_map.h"
#include "augs/templates/enum_introspect.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/filesystem/file.h"
#include "augs/audio/sound_buffer.h"

#include "game/assets/ids/asset_ids.h"

#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_sounds.h"

void load_test_scene_sounds(sound_buffer_inputs_map& all_definitions) {
	using id_type = assets::sound_id;
	using test_id_type = test_scene_sound_id;

	const auto directory = "content/official/sfx/";

	all_definitions.reserve(enum_count(test_id_type()));

	augs::for_each_enum_except_bounds([&](const test_scene_sound_id enum_id) {
		const auto id = to_sound_id(enum_id);

		if (found_in(all_definitions, id)) {
			return;
		}

		const auto stem = to_lowercase(augs::enum_to_string(enum_id));
		using path = augs::path_type;

		const auto without_ext = path(directory) += stem;

		augs::sound_buffer_loading_input definition;

		auto try_with = [&](const auto& p) {
			if (augs::exists(path(p) += ".ogg")) {
				definition.generate_mono = false;
				definition.source_sound = path(p) += ".ogg";
			}
			else if (augs::exists(path(p) += ".wav")) {
				definition.source_sound = path(p) += ".wav";
				definition.generate_mono = true;
			}
		};

		try_with(without_ext);
		try_with(path(without_ext) += "_1");

		if (definition.source_sound.string().size() > 0) {
			const auto new_allocation = all_definitions.allocate(std::move(definition));
			ensure_eq(new_allocation.key, id);
		}
		else {
			throw test_scene_asset_loading_error(
				"Failed to load %x: sound file was not found at %x.", 
				stem, 
				without_ext
			);
		}
	});
}