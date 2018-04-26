#pragma once
#include "augs/audio/sound_buffer_structs.h"
#include "view/maybe_official_path.h"
#include "view/viewables/asset_definition_view.h"

struct sound_loadables {
	// GEN INTROSPECTOR struct sound_loadables
	maybe_official_sound_path source_sound;
	augs::sound_buffer_loading_settings settings;
	// END GEN INTROSPECTOR

	bool operator==(const sound_loadables& b) const {
		return 
			source_sound == b.source_sound 
			&& settings == b.settings
		;
	}

	bool operator!=(const sound_loadables& b) const {
		return !operator==(b);
	}
};

struct sound_definition {
	// GEN INTROSPECTOR struct sound_definition
	sound_loadables loadables;
	// END GEN INTROSPECTOR

	void set_source_path(const maybe_official_sound_path& p) {
		loadables.source_sound = p;
	}

	const auto& get_source_path() const {
		return loadables.source_sound;
	}
};

struct sound_definition_view : asset_definition_view<sound_definition> {
	using base = asset_definition_view<sound_definition>;
	using base::base;

	augs::path_type get_source_sound_path() const {
		return resolved_source_path;
	}

	auto make_sound_loading_input() const {
		return augs::sound_buffer_loading_input {
			resolved_source_path,
			def.loadables.settings
		};
	}
};
