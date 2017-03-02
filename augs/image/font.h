#pragma once
#include <unordered_map>
#include "augs/math/vec2.h"
#include "augs/image/image.h"

#include "augs/misc/templated_readwrite.h"

struct FT_Glyph_Metrics_;
typedef FT_Glyph_Metrics_ FT_Glyph_Metrics;

namespace augs {
	struct font_glyph_metadata {
		int adv = 0;
		int bear_x = 0;
		int bear_y = 0;
		unsigned int index = 0;
		unsigned int unicode = 0;

		vec2i size = vec2i(0, 0);

		std::vector<std::pair<unsigned, int>> kerning;

		font_glyph_metadata() = default;
		font_glyph_metadata(const FT_Glyph_Metrics&);
	};

	struct font_metadata {
		int ascender = 0;
		int descender = 0;

		unsigned pt = 0;

		std::vector<font_glyph_metadata> glyphs;
		std::unordered_map<unsigned, unsigned> unicode_to_glyph_index;
	};

	struct font_loading_input {
		std::string filename;
		std::wstring characters;
		
		unsigned pt = 0u;
	};

	struct font {
		font_metadata meta;
		std::vector<augs::image> glyph_bitmaps;

		font(
			const font_loading_input& = font_loading_input()
		);
	};

	template <class A>
	bool read_object(A& ar, font_loading_input& data) {
		return
			read_object(ar, data.filename)
			&& read_object(ar, data.characters)
			&& read_object(ar, data.pt)
			;
	}

	template <class A>
	void write_object(A& ar, const font_loading_input& data) {
		write_object(ar, data.filename);
		write_object(ar, data.characters);
		write_object(ar, data.pt);
	}
}