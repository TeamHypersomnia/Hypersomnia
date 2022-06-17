#pragma once
#include <compare>

#include "augs/filesystem/path_declaration.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"

struct ad_hoc_atlas_subject {
	ad_hoc_entry_id id = 0;
	augs::path_type image_path;

	bool operator==(const ad_hoc_atlas_subject&) const = default;
};

using ad_hoc_atlas_subjects = std::vector<ad_hoc_atlas_subject>;
