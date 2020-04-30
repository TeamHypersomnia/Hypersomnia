#pragma once
#include <unordered_map>
#include "view/viewables/image_in_atlas.h"

using ad_hoc_entry_id = uint32_t;

using ad_hoc_in_atlas_map = 
	std::unordered_map<ad_hoc_entry_id, augs::atlas_entry>
;
