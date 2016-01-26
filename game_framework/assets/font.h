#pragma once
namespace augs {
	class font;
}

namespace assets {
	enum font_id {
		INVALID,
		GUI_FONT
	};
}

augs::font& operator*(const assets::font_id& id);
bool operator!(const assets::font_id& id);