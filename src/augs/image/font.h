#pragma once
#include <unordered_map>
#include "augs/math/vec2.h"
#include "augs/templates/exception_templates.h"
#include "augs/templates/container_templates.h"
#include "augs/misc/simple_pair.h"
#include "augs/filesystem/path.h"
#include "augs/image/image.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "augs/utf32_point.h"
#include "augs/misc/minmax.h"

#if BUILD_FREETYPE
struct FT_Glyph_Metrics_;
typedef FT_Glyph_Metrics_ FT_Glyph_Metrics;
#endif

namespace augs {
	using utf32_ranges = std::vector<augs::minmax<utf32_point>>;

	struct font_loading_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	struct font_glyph_metadata {
		// GEN INTROSPECTOR struct augs::font_glyph_metadata
		int adv = 0;
		int bear_x = 0;
		int bear_y = 0;
		unsigned index = 0xdeadbeef;

		std::vector<augs::simple_pair<unsigned, short>> kerning;
		// END GEN INTROSPECTOR

		font_glyph_metadata() = default;
#if BUILD_FREETYPE
		font_glyph_metadata(const FT_Glyph_Metrics&);
#endif
	};

	/* For future opts */
	struct font_settings {
		// GEN INTROSPECTOR struct augs::font_settings
		pad_bytes<4> pad;
		// END GEN INTROSPECTOR
	};

	struct font_metrics {
		// GEN INTROSPECTOR struct augs::font_metrics
		int ascender = 0;
		int descender = 0;
		// END GEN INTROSPECTOR

		unsigned get_height() const {
			return ascender - descender;
		}
	};

	struct stored_font_metadata {
		// GEN INTROSPECTOR struct augs::stored_font_metadata
		font_metrics metrics;
		font_settings settings;
		std::unordered_map<utf32_point, font_glyph_metadata> glyphs_by_code_point;
		// END GEN INTROSPECTOR
	};

	struct stored_baked_font {
		// GEN INTROSPECTOR struct augs::stored_baked_font
		stored_font_metadata meta;
		std::vector<augs::atlas_entry> glyphs_in_atlas;
		// END GEN INTROSPECTOR
	};

	struct baked_font {
		struct internal_glyph {
			font_glyph_metadata meta;
			augs::atlas_entry in_atlas;
		};

		font_metrics metrics;
		font_settings settings;

		std::unordered_map<utf32_point, internal_glyph> glyphs;

		/* 
			Enforce mindful lifetime management. 
			Copies could cause stack overflow.
		*/

		baked_font() = default;
		baked_font(const baked_font&) = delete;
		baked_font& operator=(const baked_font&) = delete;

		void unpack_from(const stored_baked_font& store) {
			metrics = store.meta.metrics;
			settings = store.meta.settings;

			for (const auto& g : store.meta.glyphs_by_code_point) {
				auto& out_g = glyphs[g.first];

				out_g.meta = g.second;
				out_g.in_atlas = store.glyphs_in_atlas[g.second.index];
			}
		}

		const internal_glyph* find_glyph(const utf32_point code_point) const {
			return mapped_or_nullptr(glyphs, code_point);
		}
	};

	struct font_loading_input {
		// GEN INTROSPECTOR struct augs::font_loading_input
		font_settings settings;
		path_type source_font_path;
		utf32_ranges unicode_ranges;
		float size_in_pixels = 0u;
		bool add_japanese_ranges = false;
		// END GEN INTROSPECTOR

		bool operator==(const font_loading_input& b) const {
			return 
				source_font_path == b.source_font_path 
				&& unicode_ranges == b.unicode_ranges 
				&& size_in_pixels == b.size_in_pixels
				&& add_japanese_ranges == b.add_japanese_ranges
			;
		}

		bool operator!=(const font_loading_input& b) const {
			return !operator==(b);
		}
	};

	struct font {
		stored_font_metadata meta;
		std::vector<augs::image> glyph_bitmaps;

		font(const font_loading_input&);
	};
}

namespace std {
	template <>
	struct hash<augs::font_loading_input> {
		size_t operator()(const augs::font_loading_input& in) const {
			return augs::hash_multiple(
				in.source_font_path.string(), 
#if TODO
				in.unicode_ranges.string(), 
#endif
				in.size_in_pixels
			);
		}
	};
}
