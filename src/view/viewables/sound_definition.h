#pragma once
#include "augs/audio/sound_buffer_structs.h"
#include "view/maybe_official_path.h"
#include "view/viewables/asset_definition_view.h"

struct sound_meta {
	// GEN INTROSPECTOR struct sound_meta
	augs::sound_buffer_loading_settings loading_settings;
	// END GEN INTROSPECTOR
};

struct sound_definition {
	// GEN INTROSPECTOR struct sound_definition
	maybe_official_sound_path source_sound;
	sound_meta meta;
	// END GEN INTROSPECTOR

	bool loadables_differ(const sound_definition& b) const {
		return source_sound != b.source_sound || meta.loading_settings != b.meta.loading_settings;
	}

	void set_source_path(const maybe_official_sound_path& p) {
		source_sound = p;
	}

	const auto& get_source_path() const {
		return source_sound;
	}
};

struct sound_definition_view : asset_definition_view<const sound_definition> {
	using base = asset_definition_view<const sound_definition>;
	using base::base;

	augs::path_type get_source_sound_path() const {
		return resolved_source_path;
	}

	auto make_sound_loading_input() const {
		return augs::sound_buffer_loading_input {
			resolved_source_path,
			get_def().meta.loading_settings
		};
	}
};
