#include <algorithm>
#include "augs/gui/rect.h"
#include "drafter.h"

#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"
#undef min
#undef max
namespace augs {
	namespace gui {
		namespace text {
			const baked_font& drafter::getf(const gui::text::formatted_utf32_string& source, const unsigned i) const {
				//return (i < source.length() && source[i].format.font) ? source[i].format.font : target_caret->default_style.f;
				return *source[i].format.font;
			}

			int drafter::get_kern(const gui::text::formatted_utf32_string& source, const unsigned i, const unsigned l) const {
				if (kerning && i > lines[l].begin) {
					auto* f1 = &getf(source, i);
					auto* f2 = &getf(source, i - 1);

					if (f1 == f2) {
						auto& vk = get_cached(i).kerning;
						
						for (unsigned k = 0; k < vk.size(); ++k) {
							if (vk[k].first == source[i - 1].utf_unit) {
								return vk[k].second;
							}
						}
					}

				}
				return 0;
			}

			const font_glyph_metadata& drafter::get_cached(const int i) const {
				return (cached.at(i) == nullptr) ? default_glyph : cached[i]->meta;
			}

			void drafter::find_ascdesc(const gui::text::formatted_utf32_string& in, const int l, const int r, int& asc, int& desc) const {
				if (l == r) {
					if (l > 0) {
						asc = getf(in, l - 1).metrics.ascender;
						desc = getf(in, l - 1).metrics.descender;
					}
					else {
						asc = getf(in, l).metrics.ascender;
						desc = getf(in, l).metrics.descender;
					}
				}
				else {
					asc = getf(in, l).metrics.ascender, desc = getf(in, l).metrics.descender;

					for (int j = l; j < r; ++j) {
						asc = std::max(asc, getf(in, j).metrics.ascender);
						desc = std::min(desc, getf(in, j).metrics.descender);
					}
				}
			}

			int drafter::line::height() const {
				return asc - desc;
			}

			int drafter::line::bottom() const {
				return top + height();
			}

			xywhi drafter::line::get_rect() const {
				return ltrbi(0, top, right, bottom());
			}

			void drafter::line::set(const int _y, const int _asc, const int _desc) {
				top = _y;
				asc = _asc;
				desc = _desc;
			}

			void drafter::line::adjust(const font_metrics& f) {
				asc = std::max(asc, f.ascender);
				desc = std::min(desc, f.descender);
			}

			bool drafter::line::empty() const {
				return begin == end;
			}

			unsigned drafter::line::hover(const int x, const std::vector<int>& sectors) const {
				if (end - begin <= 1)
					return begin; /* obvious if we have no (or one) sectors */

				auto iter = static_cast<unsigned>(lower_bound(sectors.begin() + begin, sectors.begin() + end, x) - sectors.begin());

				/* if iter is equal to begin there's no "previous" sector we can compare to */
				if (iter == begin /*|| iter == sectors.size()-1*/) return iter;

				/* each line has a newline on its end,
				and caret will land there if mouse is clicked to the right of the line, but printer will interpret it as the line below
				thus we want to decrement iter so the caret appears in the same line

				of course except for the last line where there's no newline at the and, and that's what this condition is for
				*/
				if (iter == end && iter != sectors.size() - 1) --iter;
				//if(iter !== end) --iter;

				/* snap result to the closest sector */
				if (!wrapped && sectors[iter] - x > x - sectors[iter - 1])
					--iter;
				else if (wrapped && (iter < end ? sectors[iter] : right) - x > x - sectors[iter - 1])
					--iter;

				return iter;
			}

			unsigned drafter::get_line(const unsigned i) const {
				if (lines.empty()) return 0;
				line l;
				l.end = i;
				const auto res = static_cast<unsigned>(upper_bound(lines.begin(), lines.end(), l, [](const line& x, const line& y) {return x.end < y.end; }) - lines.begin());

				if (res == lines.size()) {
					return res - 1;
				}
				
				return res;
			}

			vec2i drafter::view_caret(const unsigned caret_pos, const ltrbi& clipper) const {
				vec2i offset(0, 0);

				if (!clipper.good() || !clipper.hover(ltrbi(vec2i(0, 0), get_bbox())))
					return offset;

				/* we are now sure that both rectangles intersect */

				/* shortcut */
				const auto& l = lines[get_line(caret_pos)];

				/* if requested line's top is above or it won't fit at all, snap clipper to it's top */
				if (clipper.h() < l.height() || l.top < clipper.t)
					offset.y = l.top - clipper.t;
				/* otherwise if its bottom is under clipper's bottom, snap to line's bottom */
				else if (l.bottom() > clipper.b)
					offset.y = l.bottom() - clipper.b;

				const int car = sectors[caret_pos];
				if (car <= clipper.l)
					offset.x = car - clipper.l;
				else if (car >= clipper.r)
					offset.x = car - clipper.r + 1;

				return offset;
			}

			unsigned drafter::map_to_line(const vec2i& p) const {
				if (lines.empty() || sectors.empty() || p.y < 0) return 0;
				line l;
				l.top = p.y;
				l.desc = l.asc = 0;
				auto res = static_cast<unsigned>(lower_bound(lines.begin(), lines.end(), l, [](const line& x, const line& y) {return x.bottom() < y.bottom(); }) - lines.begin());
				if (res == lines.size()) --res;
				return res;
			}

			unsigned drafter::map_to_caret_pos(const vec2i& p) const {
				return lines[map_to_line(p)].hover(p.x, sectors);
			}
			
			void drafter::clear() {
				cached.clear();
				lines.clear();
				sectors.clear();

				/* add a new, empty, initial line
				there is ALWAYS at least one, even with empty string
				*/
				lines.push_back(line());

				max_x = 0;
			}

			void drafter::draw(const formatted_string& source_utf8) {
				cached_str = source_utf8;
				const auto& source = cached_str;

				clear();

				/* we have nothing to draw */
				if (source.empty()) {
					return;
				}

				/* reserve enough space to avoid reallocation */
				cached.reserve(source.size());

				/* update glyph data so each glyph object corresponds to a string character */
				for (unsigned i = 0; i < source.size(); ++i) {
					const auto& ff = getf(source, i);

					if (const auto g = ff.find_glyph(password_mode ? password_character : source[i].utf_unit)) {
						cached.push_back(g);
					}
					else if (const auto g = ff.find_glyph(' ')) {
						/* if we allowed a null glyph in string, it must be newline */
						cached.push_back(g);
					}
				}

				vec2i pen(0, 0);

				/* FIRST PASS: ONLY GENERATE LINES DEPENDING ON NEWLINE CHARACTERS AND WRAPPING WIDTH */
				/* for every character */
				for (unsigned i = 0, l = 0; i < cached.size(); ++i) {
					/* shortcut */
					const auto& g = get_cached(i);

					/* advance pen taking kerning into consideration */
					pen.x += get_kern(source, i, l) + g.adv;
					/* at this vec2i "pen.x" means "where would caret be AFTER placing this character" */
					bool wrap = (wrap_width > 0 && pen.x + g.bear_x >= int(wrap_width));

					/* if we have just encountered a newline character or there is need to wrap, we have to break the current line and
					create another */
					if (augs::is_character_newline(source[i].utf_unit) || wrap) {
						/* take care of the current line */
						lines[l].wrapped = wrap;
						/* this will be moved left if we're wrapping */
						lines[l].right = pen.x;
						/* end is exclusive, so we add 1 */
						lines[l].end = i + 1;


						/* push new line object */
						lines.push_back(line());
						/* set it, begin is inclusive */
						lines[l + 1].begin = lines[l].end;

						/* reset pen */
						pen.x = 0;

						if (wrap) {
							/* find word breaking character or return begin if there's no such in the current line */
							int left = word_wrapper_separator.get_left_word(source, i + 1, lines[l].begin);
							/* if current line is not entirely a single word, we can move the last word to the next line */
							if (lines[l].end - left > lines[l].begin) {
								/* update lines' bounds with length of last word */
								lines[l].end -= left;
								lines[l + 1].begin -= left;

								/* update pen so it's in front of the moved word */
								for (unsigned k = lines[l + 1].begin; k < lines[l + 1].begin + left; ++k) {
									int advance = get_cached(k).adv + get_kern(source, k, l + 1);
									pen.x += advance;
									/* also update current line's right coordinate as we're taking characters away */
									lines[l].right -= advance;
								}
							}
							/* otherwise we move only last character and update pen;
								note there's no kerning because it's always first character in line
							*/
							else {
								int advance = g.adv;
								lines[l].right -= advance;
								--lines[l].end;
								--lines[l + 1].begin;
								pen.x += advance;
							}
						}

						/* expand text bounding box's right coordinate */
						max_x = std::max(max_x, unsigned(lines[l].right));

						/* proceed to the newly created line */
						++l;
					}
				}

				/* break the last line as it will never hit condition of wrapping nor newlining
					same things happen here
				*/
				(*lines.rbegin()).end = static_cast<unsigned>(cached.size());
				(*lines.rbegin()).right = pen.x;
				max_x = std::max(max_x, unsigned((*lines.rbegin()).right));

				/* SECOND PASS: GENERATE SECTORS AND FILL LINE INFO DEPENDING ON CHARACTERS HEIGHT */

				/* just to make sure */
				pen = vec2i();
				for (unsigned l = 0; l < lines.size(); ++l) {
					lines[l].top = pen.y;

					/* roll pen back */
					pen.x = 0;
					for (unsigned i = lines[l].begin; i < lines[l].end && i < cached.size(); ++i) {
						/* update line's height so it is maximum of all characters' heights */
						lines[l].adjust(getf(source, i).metrics);

						pen.x += get_kern(source, i, l);
						sectors.push_back(pen.x);

						pen.x += get_cached(i).adv;
					}
					pen.y += lines[l].height();
				}

				sectors.push_back(pen.x);
			}

			vec2i drafter::get_bbox() const {
				if (sectors.empty() || lines.empty()) {
					return{ 0, 0 };
				}

				/* plus 1 for caret, so the view caret is not canceled by clamp_scroll_to_right_down_corner */
				return { static_cast<int>(max_x) + 1, lines[lines.size() - 1].bottom() };
			}

			simple_pair<int, int> drafter::get_line_visibility(const ltrbi& clipper) const {
				if (!clipper.good() || !clipper.hover(ltrbi(vec2i(0, 0), get_bbox())))
					return { -1, -1 };

				/* we are now sure that both rectangles intersect */
				return { static_cast<int>(map_to_line(vec2i(0, clipper.t))), static_cast<int>(map_to_line(vec2i(0, clipper.b))) };
			}
		}
	}
}