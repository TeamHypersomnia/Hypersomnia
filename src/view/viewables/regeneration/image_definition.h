#pragma once
#include <optional>

#include "augs/misc/enum/enum_array.h"
#include "augs/misc/enum/enum_map.h"
#include "augs/templates/value_with_flag.h"
#include "augs/filesystem/path.h"

#include "augs/drawing/flip.h"

#include "augs/texture_atlas/texture_atlas_entry.h"

#include "view/viewables/image_meta.h"

#include "view/asset_location_context.h"
#include "view/viewables/regeneration/neon_maps.h"
#include "view/maybe_official_path.h"

augs::path_type get_neon_map_path(augs::path_type from_source_image_path);
augs::path_type get_desaturation_path(augs::path_type from_source_image_path);

struct image_extra_loadables {
	// GEN INTROSPECTOR struct image_extra_loadables
	augs::value_with_flag<neon_map_input> generate_neon_map;
	bool generate_desaturation = false;
	// END GEN INTROSPECTOR
};

struct image_ldbl {
	// GEN INTROSPECTOR struct image_ldbl
	maybe_official_path source_image;
	image_extra_loadables extras;
	// END GEN INTROSPECTOR

	bool operator==(const image_ldbl& b) const;

	bool should_generate_desaturation() const {
		return extras.generate_desaturation;
	}
};

struct image_definition {
	// GEN INTROSPECTOR struct image_definition
	image_ldbl loadables;
	image_meta meta;
	// END GEN INTROSPECTOR

	void set_source_path(const maybe_official_path& p) {
		loadables.source_image = p;
	}

	const auto& get_source_path() const {
		return loadables.source_image;
	}
};

namespace sol {
	class state;
}

class image_definition_view {
	const augs::path_type resolved_source_image_path;
	const image_definition& def;

public:
	image_definition_view(
		const asset_location_context& project_dir,
		const image_definition& def
	);
	
	augs::path_type calc_custom_neon_map_path() const;
	augs::path_type calc_generated_neon_map_path() const;
	augs::path_type calc_desaturation_path() const;

	std::optional<augs::path_type> find_custom_neon_map_path() const;
	std::optional<augs::path_type> find_generated_neon_map_path() const;
	std::optional<augs::path_type> find_desaturation_path() const;

	void regenerate_all_needed(const bool force_regenerate) const;
	void delete_regenerated_files() const;

	augs::path_type get_source_image_path() const;
	vec2u read_source_image_size() const;
};
