#pragma once
#include "view/viewables/image_in_atlas.h"
#include "augs/network/network_types.h"

using avatars_in_atlas_map = 
	std::array<augs::atlas_entry, max_incoming_connections_v>
;
