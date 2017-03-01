#pragma once
#include <unordered_map>
#include <memory>
#include "augs/texture_atlas/texture_atlas.h"

#include "game/assets/font_id.h"

#include "augs/texture_atlas/texture_with_image.h"

struct FT_Glyph_Metrics_;
typedef FT_Glyph_Metrics_ FT_Glyph_Metrics;

namespace augs {
	class font {
	public:
		struct glyph {
			texture_with_image sprite;

			int adv = 0, bear_x = 0, bear_y = 0;
			unsigned int index = 0, unicode = 0;

			vec2i size = vec2i(0, 0);

			std::vector<std::pair<unsigned, int> > kerning;

			glyph() = default;
			glyph(const FT_Glyph_Metrics&);
		};

		std::vector<glyph> glyphs;
		std::unordered_map<unsigned, unsigned> unicode;

		typedef std::vector<std::pair<wchar_t, wchar_t> > charset;

		void open(const char* filename, unsigned pt, std::pair<wchar_t, wchar_t> range);
		void open(const char* filename, unsigned pt, const charset& ranges);
		void open(const char* filename, unsigned _pt, const std::wstring& characters);

		glyph* get_glyphs(), *get_glyph(unsigned unicode);
		unsigned get_pt() const, get_height() const;

		charset to_charset(const std::wstring&);
		int ascender = 0, descender = 0;
		unsigned pt = 0;

		void free_images();

		void add_to_atlas(texture_atlas&);

		// TODO: Remove placeholders

		bool can_be_bolded() { return false; }
		bool can_be_italicsed() { return false; }
		bool is_bolded() { return false; }
		bool is_italicsed() { return false;  }

		assets::font_id get_bold(bool flag) { return assets::font_id::INVALID; }
		assets::font_id get_italics(bool flag) { return assets::font_id::INVALID; }

		//assets::font_id get_bold(bool flag);
		//assets::font_id get_italics(bool flag);
		//
		//assets::font_id bold = assets::font_id::INVALID;
		//assets::font_id italics = assets::font_id::INVALID;
		//assets::font_id bi = assets::font_id::INVALID;
		//assets::font_id regular = assets::font_id::INVALID;
		//
		//
		//static void set_styles(assets::font_id regular, assets::font_id bold, assets::font_id italics, assets::font_id bi);
		//static bool can_be_bolded(assets::font_id);
		//static bool can_be_italicsed(assets::font_id);
		//static bool is_bolded(assets::font_id);
		//static bool is_italicsed(assets::font_id);
	};
}