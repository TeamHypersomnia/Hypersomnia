#pragma once
#include "font.h"
#include <set>

#include <freetype\ft2build.h> 
#include FT_FREETYPE_H

#include "../augmentations.h"
#include "augs/error/augs_error.h"

namespace augs {
	font::charset font::to_charset(const std::wstring& str) {
		charset ranges;
		ranges.reserve(str.size());

		std::set<wchar_t> s;
		unsigned size = str.size();
		for (unsigned i = 0; i < size; ++i) s.insert(str[i]);

		for (auto it = s.begin(), end = s.end(); it != end; ++it)
			ranges.push_back(std::pair<wchar_t, wchar_t>(*it, (*it) + 1));

		return ranges;
	}

	bool font::open(const char* filename, unsigned pt, std::pair<wchar_t, wchar_t> range) {
		charset ranges;
		ranges.push_back(range);
		return open(filename, pt, ranges);
	}

	bool font::open(const char* filename, unsigned pt, const std::wstring& str) {
		return open(filename, pt, to_charset(str));
	}

	font::glyph::glyph(const FT_Glyph_Metrics& m)
		: adv(m.horiAdvance >> 6), bear_x(m.horiBearingX >> 6), bear_y(m.horiBearingY >> 6), size(m.width >> 6, m.height >> 6) {
	}

	bool font::open(const char* filename, unsigned _pt, const charset& ranges) {
		pt = _pt;
		FT_Face face;
		int f = 1, error = FT_New_Face(*freetype_library.get(), filename, 0, &face);

		errsf(error != FT_Err_Unknown_File_Format, L"font format unsupported", f);
		errsf(!error, L"coulnd't open font file", f);
		errsf(!FT_Set_Char_Size(face, 0, pt << 6, 72, 72), L"couldn't set char size", f);
		errsf(!FT_Select_Charmap(face, FT_ENCODING_UNICODE), L"couldn't set encoding", f);

		if (!f) {
			std::string errstring("Couldn't load ");
			errstring += filename;
			throw std::runtime_error(errstring.c_str());
		}

		ascender = face->size->metrics.ascender >> 6;
		descender = face->size->metrics.descender >> 6;

		FT_UInt g_index;

		size_t reservation = 0;

		for (unsigned i = 0; i < ranges.size(); ++i)
			reservation += ranges[i].second - ranges[i].first;

		glyphs.reserve(reservation);

		for (unsigned i = 0; i < ranges.size(); ++i) {
			for (unsigned j = ranges[i].first; j < ranges[i].second; ++j) {
				g_index = FT_Get_Char_Index(face, j);

				if (g_index) {
					errsf(!FT_Load_Glyph(face, g_index, FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM), L"couldn't load glyph", f);
					errsf(!FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL), L"couldn't render glyph", f);

					glyphs.push_back(glyph(face->glyph->metrics));
					glyph& g = *glyphs.rbegin();

					g.index = g_index;
					g.unicode = j;

					if (face->glyph->bitmap.width) {
						g.img.copy(face->glyph->bitmap.buffer, 1, face->glyph->bitmap.pitch, rects::wh<int>(face->glyph->bitmap.width, face->glyph->bitmap.rows));
					}

					unicode[j] = glyphs.size() - 1;
				}
			}
		}

		FT_Vector delta;
		if (FT_HAS_KERNING(face)) {
			for (unsigned i = 0; i < glyphs.size(); ++i) {
				for (unsigned j = 0; j < glyphs.size(); ++j) {
					FT_Get_Kerning(face, glyphs[j].index, glyphs[i].index, FT_KERNING_DEFAULT, &delta);
					if (delta.x)
						glyphs[i].kerning.push_back(std::pair<unsigned, int>(glyphs[j].unicode, delta.x >> 6));
				}
				glyphs[i].kerning.shrink_to_fit();
			}
		}

		glyphs.shrink_to_fit();
		FT_Done_Face(face);
		return f != 0;
	}

	font::glyph* font::get_glyphs() {
		return glyphs.data();
	}

	unsigned font::get_pt() const {
		return pt;
	}

	unsigned font::get_height() const {
		return ascender - descender;
	}

	void font::free_images() {
		for (unsigned i = 0; i < glyphs.size(); ++i)
			glyphs[i].img.destroy();
	}

	void font::add_to_atlas(atlas& atl) {
		for (auto& g : glyphs) {
			if (g.img.get_size().w) {
				g.tex.set(&g.img);
				g.tex.luminosity_to_alpha(true);
				atl.textures.push_back(&g.tex);
			}
		}
	}

	//void font::set_styles(assets::font_id regular, assets::font_id b, assets::font_id i, assets::font_id bi) {
	//	(*b)->regular = (*i)->regular = (*bi)->regular = regular;
	//	(*b)->bold = (*i)->bold = (*bi)->bold = b;
	//	(*b)->italics = (*i)->italics = (*bi)->italics = i;
	//	(*b)->bi = (*i)->bi = (*bi)->bi = bi;
	//
	//	(*regular)->regular = regular;
	//	(*regular)->bold = b;
	//	(*regular)->italics = i; 
	//	(*regular)->bi = bi;
	//}
	//
	//bool font::can_be_bolded(assets::font_id regular) {
	//	return (regular == this && bold) || (italics == this && bi) || this == bold || this == bi;
	//}
	//
	//bool font::can_be_italicsed(assets::font_id regular) {
	//	return (regular == this && italics) || (bold == this && bi) || this == italics || this == bi;
	//}
	//
	//bool font::is_bolded(assets::font_id regular) {
	//	return this == bold || this == bi;
	//}
	//
	//bool font::is_italicsed(assets::font_id regular) {
	//	return this == italics || this == bi;
	//}
	//
	//font* font::get_bold(bool flag) {
	//	if (flag) {
	//		if (this == regular || this == bold) return bold ? bold : regular;
	//		if (this == italics || this == bi)   return bi ? bi : italics;
	//	}
	//	else {
	//		if (this == regular || this == bold) return regular;
	//		if (this == italics || this == bi)   return italics ? italics : bi;
	//	}
	//	return 0;
	//}
	//
	//font* font::get_italics(bool flag) {
	//	if (flag) {
	//		if (this == regular || this == italics) return italics ? italics : regular;
	//		if (this == bold || this == bi)      return bi ? bi : bold;
	//	}
	//	else {
	//		if (this == regular || this == italics) return regular;
	//		if (this == bold || this == bi)      return bold ? bold : bi;
	//	}
	//	return 0;
	//}
	//
	font::glyph* font::get_glyph(unsigned unicode_id) {
		auto it = unicode.find(unicode_id);
		if (it == unicode.end()) return nullptr;
		else return &glyphs[(*it).second];
		//return glyphs[parent->unicode[unicode]];
	}
}