#include <map>
#include "augs/image/font.h"

#include <ft2build.h> 
#include FT_FREETYPE_H

#include "augs/global_libraries.h"
#include "augs/filesystem/file.h"
#include "augs/misc/scope_guard.h"

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

	font::font(const font_loading_input& in) {
		auto throw_error = [&in](auto&&... args) {
			throw font_loading_error(
				typesafe_sprintf("Failed to load font file %x:\n", in.source_font_path.string())
				+ typesafe_sprintf(std::forward<decltype(args)>(args)...)
			);
		};

		FT_Face face;

		const auto error = FT_New_Face(*global_libraries::freetype_library.get(), in.source_font_path.string().c_str(), 0, &face);

		auto scope = make_scope_guard([&face]() {
			FT_Done_Face(face);
		});

		LOG("Loading font %x", in.source_font_path);

		if (error == FT_Err_Unknown_File_Format) {
			throw_error("Font format unsupported");
		}
		else if (error) {
			throw_error("FT_New_Face returned %x", error);
		}

		if (const auto result = FT_Set_Char_Size(face, 0, in.pt << 6, 72, 72)) {
			throw_error("FT_Set_Char_Size returned %x", result);
		}

		if (const auto result = FT_Select_Charmap(face, FT_ENCODING_UNICODE)) {
			throw_error("FT_Select_Charmap returned %x", result);
		}

		meta.ascender = face->size->metrics.ascender >> 6;
		meta.descender = face->size->metrics.descender >> 6;

		FT_UInt g_index;

		try {
			const auto& chars = augs::get_file_contents(in.charset_path, wchar_t());

			meta.glyphs.reserve(chars.size());
			glyph_bitmaps.reserve(chars.size());

			for (const auto j : chars) {
				g_index = FT_Get_Char_Index(face, j);

				if (g_index) {
					if (const auto result = FT_Load_Glyph(face, g_index, FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_NO_AUTOHINT)) {
						throw_error("FT_Load_Glyph returned %x", result);
					}

					if (const auto result = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
						throw_error("FT_Render_Glyph returned %x", result);
					}

					meta.glyphs.push_back(font_glyph_metadata(face->glyph->metrics));
					glyph_bitmaps.push_back(augs::image());

					auto& g = *meta.glyphs.rbegin();

					g.index = g_index;
					g.unicode = j;

					if (face->glyph->bitmap.width) {
						auto& g_img = *glyph_bitmaps.rbegin();

						g_img = augs::image(
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
		}
		catch (const augs::ifstream_error& err) {
			throw_error("Couldn't load charset file: %x\n%x", in.charset_path, err.what());
		}
	}
}
