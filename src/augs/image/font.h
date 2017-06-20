#pragma once
#include <unordered_map>
#include "augs/math/vec2.h"
#include "augs/image/image.h"

#include "augs/misc/trivially_copyable_pair.h"

#include "augs/texture_atlas/texture_atlas_entry.h"

#include "game/assets/font_id.h"

struct FT_Glyph_Metrics_;
typedef FT_Glyph_Metrics_ FT_Glyph_Metrics;

namespace augs {
	struct font_glyph_metadata {
		// GEN INTROSPECTOR struct augs::font_glyph_metadata
		int adv = 0;
		int bear_x = 0;
		int bear_y = 0;
		unsigned int index = 0;
		unsigned int unicode = 0;

		vec2i size = vec2i(0, 0);

		std::vector<augs::trivially_copyable_pair<unsigned, int>> kerning;
		// END GEN INTROSPECTOR

		font_glyph_metadata() = default;
		font_glyph_metadata(const FT_Glyph_Metrics&);
	};

	struct font_metadata_from_file {
		// GEN INTROSPECTOR struct augs::font_metadata_from_file
		int ascender = 0;
		int descender = 0;

		unsigned pt = 0;

		std::vector<font_glyph_metadata> glyphs;
		std::unordered_map<unsigned, unsigned> unicode_to_glyph_index;
		// END GEN INTROSPECTOR

		unsigned get_height() const {
			return ascender - descender;
		}

		const font_glyph_metadata* get_glyph(const unsigned unicode_id) const {
			auto it = unicode_to_glyph_index.find(unicode_id);
			if (it == unicode_to_glyph_index.end()) return nullptr;
			else return &glyphs[(*it).second];
		}

		unsigned get_glyph_index(const unsigned unicode) const {
			const auto it = unicode_to_glyph_index.find(unicode);

			if (it == unicode_to_glyph_index.end()) {
				return 0xdeadbeef;
			}

			return (*it).second;
		}
	};

	struct baked_font {
		// GEN INTROSPECTOR struct augs::baked_font
		font_metadata_from_file meta_from_file;
		std::vector<augs::texture_atlas_entry> glyphs_in_atlas;
		// END GEN INTROSPECTOR

		bool can_be_bolded() const { return false; }
		bool can_be_italicsed() const { return false; }
		bool is_bolded() const { return false; }
		bool is_italicsed() const { return false; }

		assets::font_id get_bold(const bool flag) const { return assets::font_id::INVALID; }
		assets::font_id get_italics(const bool flag) const { return assets::font_id::INVALID; }
	};

	struct font_loading_input {
		// GEN INTROSPECTOR struct augs::font_loading_input
		std::string source_font_path;
		std::string charset_path;
		
		unsigned pt = 0u;
		// END GEN INTROSPECTOR

		bool operator==(const font_loading_input& b) const {
			return 
				source_font_path == b.source_font_path 
				&& charset_path == b.charset_path 
				&& pt == b.pt
			;
		}
	};

	struct font {
		font_metadata_from_file meta;
		std::vector<augs::image> glyph_bitmaps;

		void from_file(const font_loading_input&);
	};
}

namespace std {
	template <>
	struct hash<augs::font_loading_input> {
		size_t operator()(const augs::font_loading_input& in) const {
			return augs::simple_two_hash(in.source_font_path, in.charset_path) + in.pt;
		}
	};
}