#include "augs/filesystem/file.h"

#include "game/hardcoded_content/test_scenes/test_scenes_content.h"
#include "game/assets/all_assets.h"

#include "generated/introspectors.h"

void load_test_scene_sound_buffers(all_viewable_defs& manager) {
	using id_type = assets::sound_buffer_id;
	const auto directory = "content/official/sfx/";

	auto& sounds = manager.get_store_by(id_type());

	augs::for_each_enum_except_bounds<id_type>([&](const id_type id) {
		const auto stem = to_lowercase(augs::enum_to_string(id));
		using path = augs::path_type;

		const auto without_ext = path(directory) += stem;

		augs::sound_buffer_loading_input def;

		if (augs::file_exists(path(without_ext) += ".ogg")) {
			def.generate_mono = false;
			def.path_template = path(without_ext) += ".ogg";
		}
		else {
			if (augs::file_exists(path(without_ext) += ".wav")) {
				def.path_template = path(without_ext) += ".wav";
			}
			else if (augs::file_exists(path(without_ext) += "_1.wav")) {
				def.path_template = path(without_ext) += "_%x.wav";
			}

			def.generate_mono = true;
		}

		if (def.path_template.string().size() > 0) {
			sounds[id] = def;
		}
		else {
			throw test_scene_asset_loading_error(
				"Error loading %x: sound file was not found at %x.", 
				stem, 
				without_ext
			);
		}
	});
}