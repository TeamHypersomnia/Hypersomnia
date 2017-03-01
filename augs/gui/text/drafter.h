#pragma once
#include "augs/image/font.h"
#include "word_separator.h"
// ui relates on draft object (result) only
// if bugs viewing the caret, check the viewcaret where "car" variable was changed to caret_rect
// neither drafter nor printer have their fstr declared as member fields because it is better to pass it around than rewrite each time we want to change string
namespace augs {
	namespace gui {
		namespace text {
			struct drafter {
				struct line {
					unsigned hover(int x, const std::vector<int>& sectors) const;  /* return sector that is the closest x  */
					xywhi get_rect() const; /* actual line rect */

					int top, right, height() const, bottom() const, /* coordinates */
						asc, desc;
					unsigned begin, end;
					bool wrapped;
					line();
					void set(int y, int asc, int desc);
					void adjust(const augs::font&);
					bool empty() const;
				};

				std::vector<augs::font::glyph*> cached;
				std::vector<line> lines;
				std::vector<int> sectors;

				word_separator word_wrapper_separator;

				unsigned wrap_width;

				/* WARNING! Setting kerning flag to true may reuslt in performance hit if rendering huge amounts of text */
				bool kerning, password_mode;

				/* L'*' on init */
				wchar_t password_character;

				/* default glyph placed instead null characters */
				augs::font::glyph default_info;
				augs::font::glyph default_glyph;

				drafter();

				/* returns offset that clipper must be moved to show whole caret */
				vec2i view_caret(unsigned caret_pos, const ltrbi& clipper) const;
				unsigned get_line(unsigned caret_pos) const;

				/* note: vec2i is taken LOCALLY.*/
				unsigned map_to_line(const vec2i&) const;
				unsigned map_to_caret_pos(const vec2i&) const;

				/* returns text's bounding box */
				vec2i get_bbox() const;

				void draw(const fstr&);

				/*
				clipper is in local drafter's space: (0, 0) = left top corner
				if any in the pair is -1, there's no line visible */
				std::pair<int, int> get_line_visibility(const ltrbi& clipper) const;
			private:
				unsigned max_x;
				void find_ascdesc(const fstr& source, int i, int j, int&, int&) const;
				int get_kern(const fstr& source, unsigned code1, unsigned code2) const;
				const augs::font::glyph& get_cached(int i) const;
				augs::font& getf(const fstr& source, unsigned i) const;
			};
		}
	}
}