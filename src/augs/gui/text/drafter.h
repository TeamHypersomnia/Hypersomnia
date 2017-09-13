#pragma once
#include "augs/image/font.h"
#include "word_separator.h"
// ui relates on draft object (result) only
// if bugs viewing the caret, check the viewcaret where "car" variable was changed to caret_rect
// neither drafter nor printer have their formatted_string declared as member fields because it is better to pass it around than rewrite each time we want to change string
namespace augs {
	namespace gui {
		namespace text {
			struct drafter {
				struct line {
					int top = 0;
					int right = 0;
					int asc = 0; 
					int desc = 0;
					unsigned begin = 0;
					unsigned end = 0;
					bool wrapped = false;

					void set(int y, int asc, int desc);
					void adjust(const augs::font_metrics&);
					
					bool empty() const;
					int height() const;
					int bottom() const;
					xywhi get_rect() const; /* actual line rect */
					unsigned hover(int x, const std::vector<int>& sectors) const;  /* return sector that is the closest x */
				};

				std::vector<const augs::baked_font::internal_glyph*> cached;
				std::vector<line> lines;
				std::vector<int> sectors;

				word_separator word_wrapper_separator;

				unsigned wrap_width = 0;

				bool kerning = true;
				bool password_mode = false;

				wchar_t password_character = L'*';

				augs::font_glyph_metadata default_glyph;

				/* returns offset that clipper must be moved to show whole caret */
				vec2i view_caret(unsigned caret_pos, const ltrbi& clipper) const;
				unsigned get_line(unsigned caret_pos) const;

				/* note: vec2i is taken LOCALLY.*/
				unsigned map_to_line(const vec2i&) const;
				unsigned map_to_caret_pos(const vec2i&) const;

				/* returns text's bounding box */
				vec2i get_bbox() const;

				void draw(const formatted_string&);

				void clear();

				/*
				clipper is in local drafter's space: (0, 0) = left top corner
				if any in the pair is -1, there's no line visible */
				std::pair<int, int> get_line_visibility(const ltrbi& clipper) const;
			private:
				unsigned max_x;
				void find_ascdesc(const formatted_string& source, const int i, const int j, int&, int&) const;
				int get_kern(const formatted_string& source, const unsigned code1, const unsigned code2) const;
				const augs::font_glyph_metadata& get_cached(const int i) const;
				const augs::baked_font& getf(const formatted_string& source, const unsigned i) const;
			};
		}
	}
}