#include <map>
#include "augs/image/font.h"

#if BUILD_FREETYPE
#include <ft2build.h> 
#include FT_FREETYPE_H
#endif

#include "augs/misc/imgui/imgui_utils.h"

#include "augs/global_libraries.h"
#include "augs/filesystem/file.h"
#include "augs/string/string_templates.h"
#include "augs/misc/scope_guard.h"
#include "augs/misc/bound.h"

namespace augs {
#if BUILD_FREETYPE
	font_glyph_metadata::font_glyph_metadata(
		const FT_Glyph_Metrics& m
	) :
		adv(m.horiAdvance >> 6),
		bear_x(m.horiBearingX >> 6),
		bear_y(m.horiBearingY >> 6)
	{
	}
#endif

	font::font(const font_loading_input& in) {
#if BUILD_FREETYPE
		auto throw_error = [&in](auto&&... args) {
			throw font_loading_error(
				typesafe_sprintf("Failed to load font file %x:\n", in.source_font_path.string())
				+ typesafe_sprintf(std::forward<decltype(args)>(args)...)
			);
		};

		FT_Face face;

		const auto error = FT_New_Face(*global_libraries::freetype_library.get(), in.source_font_path.string().c_str(), 0, &face);

		auto scope = scope_guard([&face]() {
			FT_Done_Face(face);
		});

		LOG("Loading font %x", in.source_font_path);

		if (error == FT_Err_Unknown_File_Format) {
			throw_error("Font format unsupported");
		}
		else if (error) {
			throw_error("FT_New_Face returned %x", error);
		}

		{
			/* Based on an ImGui snippet */
			FT_Size_RequestRec req;
			req.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
			req.width = 0;
			req.height = (uint32_t)in.size_in_pixels * 64;
			req.horiResolution = 0;
			req.vertResolution = 0;

			if (const auto result = FT_Request_Size(face, &req)) {
				throw_error("FT_Request_Size returned %x", result);
			}
		}

		if (const auto result = FT_Select_Charmap(face, FT_ENCODING_UNICODE)) {
			throw_error("FT_Select_Charmap returned %x", result);
		}

		meta.metrics.ascender = face->size->metrics.ascender >> 6;
		meta.metrics.descender = face->size->metrics.descender >> 6;

		FT_UInt g_index;

		struct loaded_index {
			FT_UInt char_index;
			FT_UInt code_point;
		};

		thread_local std::vector<loaded_index> ft_indices;
		ft_indices.clear();

		try {
			auto ranges = in.unicode_ranges;

			if (_should(in.add_japanese_ranges)) {
				augs::imgui::concat_ranges(ranges, ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
			}

			if (_should(in.add_cyrillic_ranges)) {
				augs::imgui::concat_ranges(ranges, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
			}

			std::size_t total = 0;

			for (const auto& range : ranges) {
				total += range.second - range.first;
			}

			glyph_bitmaps.reserve(total);

			for (const auto& range : ranges) {
				for (auto j = range.first; j <= range.second; ++j) {
					g_index = FT_Get_Char_Index(face, j);

					if (g_index) {
						ft_indices.push_back({ g_index, j });

						if (const auto result = FT_Load_Glyph(face, g_index, FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_NO_AUTOHINT)) {
							throw_error("FT_Load_Glyph returned %x", result);
						}

						if (const auto result = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
							throw_error("FT_Render_Glyph returned %x", result);
						}

						auto& g = meta.glyphs_by_code_point[j];
						g = face->glyph->metrics;
						g.index = static_cast<unsigned>(glyph_bitmaps.size());

						glyph_bitmaps.push_back(augs::image());

						if (face->glyph->bitmap.width) {
							auto& g_img = *glyph_bitmaps.rbegin();

							g_img = augs::image(
								face->glyph->bitmap.buffer, 
								vec2u(
									face->glyph->bitmap.width, 
									face->glyph->bitmap.rows
								),
								1, 
								face->glyph->bitmap.pitch
							);
						}
					}
				}	
			}

			FT_Vector delta;

			if (FT_HAS_KERNING(face)) {
				for (unsigned i = 0; i < ft_indices.size(); ++i) {
					auto& subject = meta.glyphs_by_code_point[ft_indices[i].code_point];

					for (unsigned j = 0; j < ft_indices.size(); ++j) {
						FT_Get_Kerning(face, ft_indices[j].char_index, ft_indices[i].char_index, FT_KERNING_DEFAULT, &delta);
						
						if (delta.x) {
							subject.kerning.push_back({ ft_indices[j].code_point, static_cast<short>(delta.x >> 6) });
						}
					}

					subject.kerning.shrink_to_fit();
				}
			}
		}
		catch (const augs::file_open_error& err) {

		}
#else
		(void)in;
#endif
	}
}
