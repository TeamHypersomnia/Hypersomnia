#include <map>
#include "augs/image/font.h"

#include <ft2build.h> 
#include FT_FREETYPE_H

#include "augs/global_libraries.h"
#include "augs/ensure.h"

namespace augs {
	font_glyph_metadata::font_glyph_metadata(
		const FT_Glyph_Metrics& m
	) : 
		adv(m.horiAdvance >> 6), 
		bear_x(m.horiBearingX >> 6), 
		bear_y(m.horiBearingY >> 6), 
		size(m.width >> 6, m.height >> 6) 
	{
	}

	void font::from_file(const font_loading_input& in) {
		FT_Face face;
		
		const auto error = FT_New_Face(*global_libraries::freetype_library.get(), in.path.c_str(), 0, &face);

		LOG("Loading font %x", in.path);

		ensure(error != FT_Err_Unknown_File_Format && L"font format unsupported");
		ensure(!error && L"coulnd't open font file");
		ensure(!FT_Set_Char_Size(face, 0, in.pt << 6, 72, 72) && L"couldn't set char size");
		ensure(!FT_Select_Charmap(face, FT_ENCODING_UNICODE) && L"couldn't set encoding");

		meta.ascender = face->size->metrics.ascender >> 6;
		meta.descender = face->size->metrics.descender >> 6;

		FT_UInt g_index;

		const auto& chars = in.characters;

		meta.glyphs.reserve(chars.size());
		glyph_bitmaps.reserve(chars.size());

		for (const auto j : chars) {
			g_index = FT_Get_Char_Index(face, j);

			if (g_index) {
				ensure(!FT_Load_Glyph(face, g_index, FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_NO_AUTOHINT) && L"couldn't load glyph");
				ensure(!FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) && L"couldn't render glyph");

				meta.glyphs.push_back(font_glyph_metadata(face->glyph->metrics));
				glyph_bitmaps.push_back(augs::image());

				auto& g = *meta.glyphs.rbegin();

				g.index = g_index;
				g.unicode = j;

				if (face->glyph->bitmap.width) {
					auto& g_img = *glyph_bitmaps.rbegin();

					g_img.create_from(
						face->glyph->bitmap.buffer, 
						1, 
						face->glyph->bitmap.pitch, 
						vec2i(
							face->glyph->bitmap.width, 
							face->glyph->bitmap.rows
						)
					);
				}

				meta.unicode_to_glyph_index[j] = static_cast<unsigned>(meta.glyphs.size() - 1u);
			}
		}

		FT_Vector delta;
		if (FT_HAS_KERNING(face)) {
			for (unsigned i = 0; i < meta.glyphs.size(); ++i) {
				for (unsigned j = 0; j < meta.glyphs.size(); ++j) {
					FT_Get_Kerning(face, meta.glyphs[j].index, meta.glyphs[i].index, FT_KERNING_DEFAULT, &delta);
					
					if (delta.x) {
						meta.glyphs[i].kerning.push_back({ meta.glyphs[j].unicode, delta.x >> 6 });
					}
				}

				meta.glyphs[i].kerning.shrink_to_fit();
			}
		}

		FT_Done_Face(face);
	}
}
