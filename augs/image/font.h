#pragma once
#include <unordered_map>
#include "augs/math/vec2.h"
#include "augs/image/image.h"

#include "augs/misc/templated_readwrite.h"
#include "augs/texture_atlas/texture_atlas_entry.h"
#include "augs/misc/trivial_pair.h"

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

		std::vector<augs::trivial_pair<unsigned, int>> kerning;

		font_glyph_metadata() = default;
		font_glyph_metadata(const FT_Glyph_Metrics&);
	};

	struct font_metadata_from_file {
		int ascender = 0;
		int descender = 0;

		unsigned pt = 0;

		std::vector<font_glyph_metadata> glyphs;
		std::unordered_map<unsigned, unsigned> unicode_to_glyph_index;
	};

	struct baked_font {
		font_metadata_from_file meta_from_file;
		std::vector<augs::texture_atlas_entry> glyphs_in_atlas;
	};

	struct font_loading_input {
		std::string filename;
		std::wstring characters;
		
		unsigned pt = 0u;

		bool operator==(const font_loading_input& b) const {
			return filename == b.filename && characters == b.characters && pt == b.pt;
		}
	};

	struct font {
		font_metadata_from_file meta;
		std::vector<augs::image> glyph_bitmaps;

		font(
			const font_loading_input& = font_loading_input()
		);
	};

	template <class A>
	bool read_object(A& ar, baked_font& data) {
		return
			read_object(ar, data.meta_from_file)
			&& read_object(ar, data.glyphs_in_atlas)
			;
	}

	template <class A>
	void write_object(A& ar, const baked_font& data) {
		write_object(ar, data.meta_from_file);
		write_object(ar, data.glyphs_in_atlas);
	}

	template <class A>
	bool read_object(A& ar, font_metadata_from_file& data) {
		return
			read_object(ar, data.ascender)
			&& read_object(ar, data.descender)
			&& read_object(ar, data.pt)
			&& read_object(ar, data.glyphs)
			&& read_object(ar, data.unicode_to_glyph_index)
			;
	}

	template <class A>
	void write_object(A& ar, const font_metadata_from_file& data) {
		write_object(ar, data.ascender);
		write_object(ar, data.descender);
		write_object(ar, data.pt);
		write_object(ar, data.glyphs);
		write_object(ar, data.unicode_to_glyph_index);
	}

	template <class A>
	bool read_object(A& ar, font_glyph_metadata& data) {
		return
			read_object(ar, data.adv)
			&& read_object(ar, data.bear_x)
			&& read_object(ar, data.bear_y)
			&& read_object(ar, data.index)
			&& read_object(ar, data.unicode)
			&& read_object(ar, data.size)
			&& read_object(ar, data.kerning)
		;
	}

	template <class A>
	void write_object(A& ar, const font_glyph_metadata& data) {
		write_object(ar, data.adv);
		write_object(ar, data.bear_x);
		write_object(ar, data.bear_y);
		write_object(ar, data.index);
		write_object(ar, data.unicode);
		write_object(ar, data.size);
		write_object(ar, data.kerning);
	}
	
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

namespace std {
	template <>
	struct hash<augs::font_loading_input> {
		size_t operator()(const augs::font_loading_input& in) const {
			return augs::simple_two_hash(in.filename, in.characters) + in.pt;
		}
	};
}