#pragma once
#include <unordered_map>
#include "augs/math/vec2.h"
#include "augs/templates/exception_templates.h"
#include "augs/misc/simple_pair.h"
#include "augs/filesystem/path.h"
#include "augs/image/image.h"
#include "augs/texture_atlas/texture_atlas_entry.h"

#if BUILD_FREETYPE
struct FT_Glyph_Metrics_;
typedef FT_Glyph_Metrics_ FT_Glyph_Metrics;
#endif

namespace augs {
	struct font_loading_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	struct font_glyph_metadata {
		// GEN INTROSPECTOR struct augs::font_glyph_metadata
		int adv = 0;
		int bear_x = 0;
		int bear_y = 0;
		unsigned index = 0xdeadbeef;

		std::vector<augs::simple_pair<wchar_t, short>> kerning;
		// END GEN INTROSPECTOR

		font_glyph_metadata() = default;
#if BUILD_FREETYPE
		font_glyph_metadata(const FT_Glyph_Metrics&);
#endif
	};

	/* For future opts */
	struct font_settings {
		// GEN INTROSPECTOR struct augs::font_settings
		// END GEN INTROSPECTOR
	};

	struct font_metrics {
		// GEN INTROSPECTOR struct augs::font_metrics
		int ascender = 0;
		int descender = 0;

		unsigned pt = 0;
		// END GEN INTROSPECTOR

		unsigned get_height() const {
			return ascender - descender;
		}
	};

	struct stored_font_metadata {
		// GEN INTROSPECTOR struct augs::stored_font_metadata
		font_metrics metrics;
		font_settings settings;
		std::unordered_map<wchar_t, font_glyph_metadata> glyphs_by_unicode;
		// END GEN INTROSPECTOR
	};

	struct stored_baked_font {
		// GEN INTROSPECTOR struct augs::stored_baked_font
		stored_font_metadata meta;
		std::vector<augs::texture_atlas_entry> glyphs_in_atlas;
		// END GEN INTROSPECTOR
	};

	struct baked_font {
		struct internal_glyph {
			font_glyph_metadata meta;
			augs::texture_atlas_entry in_atlas;
		};

		font_metrics metrics;
		font_settings settings;

		std::array<internal_glyph, std::size_t(std::numeric_limits<wchar_t>::max()) + 1> glyphs;

		/* 
			Enforce mindful lifetime management. 
			Copies could cause stack overflow.
		*/

		baked_font() = default;
		baked_font(const baked_font&) = delete;
		baked_font& operator=(const baked_font&) = delete;

		void unpack(const stored_baked_font& store) {
			metrics = store.meta.metrics;
			settings = store.meta.settings;

			for (const auto& g : store.meta.glyphs_by_unicode) {
				auto& out_g = glyphs[g.first];

				out_g.meta = g.second;
				out_g.in_atlas = store.glyphs_in_atlas[g.second.index];
			}
		}

		const internal_glyph* get_glyph(const wchar_t unicode_id) const {
			auto& g = glyphs[unicode_id];

			if (g.meta.index == 0xdeadbeef) {
				return nullptr;
			}

			return &g;
		}
	};

	struct font_loading_input {
		// GEN INTROSPECTOR struct augs::font_loading_input
		font_settings settings;
		path_type source_font_path;
		path_type charset_path;
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
				in.charset_path.string(), 
				in.pt
			);
		}
	};
}