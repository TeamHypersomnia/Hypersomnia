#pragma once
#include "game/assets/ids/asset_ids.h"

namespace augs {
	template <class id_type>
	struct sprite;
}

namespace invariants {
	using sprite = augs::sprite<assets::image_id>;
}

namespace components {
	struct sprite;
}