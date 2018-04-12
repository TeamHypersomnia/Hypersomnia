#include "augs/misc/enum/enum_map.h"
#include "augs/templates/enum_introspect.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/filesystem/file.h"
#include "augs/audio/sound_buffer.h"

#include "game/assets/ids/sound_buffer_id.h"

#include "test_scenes/test_scenes_content.h"

void load_test_scene_sound_buffers(sound_buffer_inputs_map& sounds) {
	using id_type = assets::sound_buffer_id;
	const auto directory = "content/official/sfx/";

	augs::for_each_enum_except_bounds([&](const id_type id) {
		const auto stem = to_lowercase(augs::enum_to_string(id));
		using path = augs::path_type;

		const auto without_ext = path(directory) += stem;

		augs::sound_buffer_loading_input def;

		if (augs::exists(path(without_ext) += ".ogg")) {
			def.generate_mono = false;
			def.path_template = path(without_ext) += ".ogg";
		}
		else {
			if (augs::exists(path(without_ext) += ".wav")) {
				def.path_template = path(without_ext) += ".wav";
			}
			else if (augs::exists(path(without_ext) += "_1.wav")) {
				def.path_template = path(without_ext) += "_%x.wav";
			}

			def.generate_mono = true;
		}

		if (def.path_template.string().size() > 0) {
			sounds[id] = def;
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