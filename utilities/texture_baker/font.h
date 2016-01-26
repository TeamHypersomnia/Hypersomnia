#pragma once
#include <unordered_map>
#include <memory>
#include "texture_baker.h"

struct FT_Glyph_Metrics_;
typedef FT_Glyph_Metrics_ FT_Glyph_Metrics;

namespace augs {
	struct font {
		struct glyph {
			image img;
			texture tex;

			int adv = 0, bear_x = 0, bear_y = 0;
			unsigned int index = 0, unicode = 0;

			rects::wh<int> size = rects::wh<int>(0, 0);

			std::vector<std::pair<unsigned, int> > kerning;

			glyph() = default;
			glyph(const FT_Glyph_Metrics&);
		};

		std::vector<glyph> glyphs;
		std::unordered_map<unsigned, unsigned> unicode;

		typedef std::vector<std::pair<wchar_t, wchar_t> > charset;

		bool open(const char* filename, unsigned pt, std::pair<wchar_t, wchar_t> range);
		bool open(const char* filename, unsigned pt, const charset& ranges);
		bool open(const char* filename, unsigned _pt, const std::wstring& characters);

		glyph* get_glyphs(), *get_glyph(unsigned unicode);
		unsigned get_pt() const, get_height() const;

		charset to_charset(const std::wstring&);
		int ascender = 0, descender = 0;
		unsigned pt = 0;

		void free_images();

		font* get_bold(bool flag);
		font* get_italics(bool flag);

		font *bold = nullptr, *italics = nullptr, *bi = nullptr, *regular = nullptr;

		void add_to_atlas(atlas&);

		void set_styles(font* bold, font* italics, font* bi);

		bool can_be_bolded();
		bool can_be_italicsed();
		bool is_bolded();
		bool is_italicsed();
	};
}