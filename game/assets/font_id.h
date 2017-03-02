#pragma once

namespace assets {
	enum class font_id {
		INVALID,

		GUI_FONT,
		COUNT
	};
}

namespace augs {
	struct font_metadata;
}

augs::font_metadata& operator*(const assets::font_id& id);
bool operator!(const assets::font_id& id);