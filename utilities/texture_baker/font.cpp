#pragma once
#include "stdafx.h"
#include <freetype\ft2build.h> 
#include FT_FREETYPE_H

#include <set>

#include "../options.h"
#include "font.h"

#include "../augmentations.h"

namespace augs {
	namespace texture_baker {
		font_file::font_file() : pt(0) {}

		font_file::charset font_file::to_charset(const std::wstring& str) {
			charset ranges;
			ranges.reserve(str.size());

			std::set<wchar_t> s;
			unsigned size = str.size();
			for(unsigned i = 0; i < size; ++i) s.insert(str[i]);

			for(auto it = s.begin(), end = s.end(); it != end; ++it)
				ranges.push_back(std::pair<wchar_t, wchar_t>(*it, (*it)+1));

			return ranges;
		}

		void font_file::create(std::pair<wchar_t, wchar_t> range) {
			charset ranges;
			ranges.push_back(range);
			return create(ranges);
		}

		void font_file::create(const std::wstring& str) {
			return create(to_charset(str));
		}

		void font_file::create(const charset& ranges) {
			for(unsigned i = 0; i < ranges.size(); ++i) {
				for(unsigned j = ranges[i].first; j < ranges[i].second; ++j) { 
					glyphs.push_back(glyph());
					glyph& g = *glyphs.rbegin();
					g.unicode = j;

					unicode[j] = glyphs.size()-1;
				}
			}

			glyphs.shrink_to_fit();
		}

		bool font_file::open(const char* filename, unsigned pt, std::pair<wchar_t, wchar_t> range) {
			charset ranges;
			ranges.push_back(range);
			return open(filename, pt, ranges);
		}

		bool font_file::open(const char* filename, unsigned pt, const std::wstring& str) {
			return open(filename, pt, to_charset(str));
		}

		font::glyph::glyph() : info(nullptr) {}

		font_file::glyph::glyph()
			: adv(0), bear_x(0), bear_y(0) {}

		font_file::glyph::glyph(int adv, int bear_x, int bear_y, rects::wh<int> size)
			: adv(adv), bear_x(bear_x), bear_y(bear_y), size(size) {}

		font_file::glyph::glyph(const FT_Glyph_Metrics& m) 
			: adv(m.horiAdvance >> 6), bear_x(m.horiBearingX >> 6), bear_y(m.horiBearingY >> 6), size(m.width >> 6, m.height >> 6)  {
		}

		bool font_file::open(const char* filename, unsigned _pt, const charset& ranges) {
			pt = _pt;
			FT_Face face;
			int f = 1, error = FT_New_Face(*freetype_library.get(), filename, 0, &face);

			errsf(error != FT_Err_Unknown_File_Format, L"font format unsupported", f);
			errsf(!error, L"coulnd't open font file", f);
			errsf(!FT_Set_Char_Size(face, 0, pt << 6, 96, 96), L"couldn't set char size", f);
			errsf(!FT_Select_Charmap(face, FT_ENCODING_UNICODE), L"couldn't set encoding", f);

			if(!f) {
				std::string errstring("Couldn't load ");
				errstring += filename;
				throw std::runtime_error(errstring.c_str());
			}

			ascender = face->size->metrics.ascender >> 6;
			descender = face->size->metrics.descender >> 6;

			FT_UInt g_index;

			for(unsigned i = 0; i < ranges.size(); ++i) {
				for(unsigned j = ranges[i].first; j < ranges[i].second; ++j) { 
					g_index = FT_Get_Char_Index(face, j);

					if(g_index) {
						errsf(!FT_Load_Glyph(face, g_index, FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM), L"couldn't load glyph", f);
						errsf(!FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL), L"couldn't render glyph", f);

						glyphs.push_back(glyph(face->glyph->metrics));
						glyph& g = *glyphs.rbegin();

						g.index = g_index;
						g.unicode = j;

						if(face->glyph->bitmap.width)
							g.img.copy(face->glyph->bitmap.buffer, 1, face->glyph->bitmap.pitch, rects::wh<int>(face->glyph->bitmap.width, face->glyph->bitmap.rows));

						unicode[j] = glyphs.size()-1;
					}
				}
			}

			FT_Vector delta;
			if(FT_HAS_KERNING(face)) {
				for(unsigned i = 0; i < glyphs.size(); ++i) {
					for(unsigned j = 0; j < glyphs.size(); ++j) {
						FT_Get_Kerning(face, glyphs[j].index, glyphs[i].index, FT_KERNING_DEFAULT, &delta);
						if(delta.x) 
							glyphs[i].kerning.push_back(std::pair<unsigned, int>(glyphs[j].unicode, delta.x >> 6));
					}
					glyphs[i].kerning.shrink_to_fit();
				}
			}

			glyphs.shrink_to_fit();
			FT_Done_Face(face);
			return f != 0;
		}

		font_file::glyph* font_file::get_glyphs() {
			return glyphs.data();
		}

		font_file::glyph* font_file::get_glyph(unsigned _unicode) {
			return glyphs.data() + unicode[_unicode];
		}

		unsigned font_file::get_count() const {
			return glyphs.size();	
		}

		unsigned font_file::get_pt() const {
			return pt;
		}

		unsigned font_file::get_height() const {
			return ascender - descender;
		}

		void font_file::free_images() {
			for(unsigned i = 0; i < glyphs.size(); ++i)
				glyphs[i].img.destroy();
		}

		void font_file::destroy() {
			glyphs.clear();
			glyphs.shrink_to_fit();
		}

		font::font() : glyphs(0), parent(0), bold(nullptr), italics(nullptr), regular(this) {}

		font::~font() { 
			destroy();
		}

		void font::build(font_file* _parent) {
			destroy();
			parent = _parent;
			glyphs = new glyph[parent->get_count()];
			for(unsigned i = 0; i < parent->get_count(); ++i) {
				glyphs[i].info = parent->glyphs.data() + i;
				glyphs[i].tex.set(&glyphs[i].info->img);
				glyphs[i].tex.luminosity_to_alpha(true);
			}
		}

		void font::add_to_atlas(atlas& atl) {
			for(unsigned i = 0; i < parent->get_count(); ++i) {
				if(glyphs[i].tex.get_rect().w) {
					atl.textures.push_back(&glyphs[i].tex);
				}
			}
		}

		void font::set_styles(font* b, font* i, font* bi) {
			b->regular	= i->regular = bi->regular = this;
			b->bold		= i->bold    = bi->bold    = b;
			b->italics	= i->italics = bi->italics = i;
			b->bi		= i->bi		 = bi->bi	   = bi;
			bold = b; italics = i; bi = bi;
		}

		bool font::can_be_bolded() {
			return (regular == this && bold) || (italics == this && bi) || this == bold || this == bi;
		}

		bool font::can_be_italicsed() {
			return (regular == this && italics) || (bold == this && bi) || this == italics || this == bi;
		}

		bool font::is_bolded() {
			return this == bold || this == bi;
		}

		bool font::is_italicsed() {
			return this == italics || this == bi;
		}

		unsigned font::get_height() {
			return get_parent()->get_height();
		}

		font_file* font::get_parent() {
			return parent;
		}

		font* font::get_bold(bool flag) {
			if(flag) {
				if(this == regular || this == bold) return bold ? bold : regular;
				if(this == italics || this == bi)   return bi   ? bi   : italics;
			}
			else {
				if(this == regular || this == bold) return regular;
				if(this == italics || this == bi)   return italics ? italics : bi;
			}
			return 0;
		}

		font* font::get_italics(bool flag) {
			if(flag) {
				if(this == regular || this == italics) return italics ? italics : regular;
				if(this == bold    || this == bi)      return bi      ? bi : bold;
			}
			else {
				if(this == regular || this == italics) return regular;
				if(this == bold    || this == bi)      return bold ? bold : bi;
			}
			return 0;
		}

		void font::destroy() {
			if(glyphs) {
				delete [] glyphs;
				glyphs = 0;
			}
		}

		font::glyph* font::get_glyph(unsigned unicode) {
			if(parent == nullptr) return nullptr;
			auto it = parent->unicode.find(unicode);
			if(it == parent->unicode.end()) return nullptr;
			else return glyphs + (*it).second;
			//return glyphs[parent->unicode[unicode]];
		}

		font_file* font::get_font() {
			return parent;
		}
	}
}