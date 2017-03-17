#include <algorithm>
#include "ui.h"
#include "drafter.h"
#include "printer.h"
#include "augs/graphics/drawers.h"

/* printer's draw needs revising in terms of scrolling */

namespace augs {
	namespace gui {
		namespace text {
			printer::printer() :
				caret_mat(material(assets::game_image_id::BLANK, rgba(255, 255, 255, 255))),
				align_caret_height(true), caret_width(1),
				highlight_current_line(false),
				highlight_during_selection(true),
				active(false),
				selection_bg_mat(rgba(128, 255, 255, 120)),
				selection_inactive_bg_mat(rgba(128, 255, 255, 40)),
				highlight_mat(rgba(15, 15, 15, 255))
			{
			}

			printer::blinker::blinker() : caret_visible(true), blink(true), interval_ms(250)//, blink_func(regular_blink) 
			{
				reset();
			}

			//	void printer::blinker::regular_blink(blinker& b, quad& caret) {
			//		for(int i = 0; i < 4; ++i)
			//			caret.p[i].col.a = b.caret_visible ? 255 : 0;
			//	}

			void printer::blinker::update() {
				if (timer.get<std::chrono::milliseconds>() > interval_ms) {
					caret_visible = !caret_visible;
					timer.reset();
				}
				//if(blink_func) blink_func(*this, caret);
			}

			void printer::blinker::reset() {
				timer.reset();
				caret_visible = true;
			}

			void printer::draw_text(
				std::vector<augs::vertex_triangle>& out,
				const drafter& d,
				const formatted_string& colors,
				const caret_info* const caret,
				const vec2i pos,
				const ltrbi clipper
			) const {
				/* shortcuts */
				auto& lines = d.lines;
				auto& sectors = d.sectors;
				auto& v = out;
				const bool clip = clipper.good();

				//if(clip) 
				//	pos = vec2i(*parent);
				//
				//vec2i global = scroll;

				/* we'll draw caret at the very end of procedure so we have to declare this variable here */
				xywhi caret_rect(0, 0, 0, 0);

				/* here we highlight the line caret is currently on */
				if (caret && active && highlight_current_line) {
					drafter::line highlighted = lines.size() ? lines[d.get_line(caret->pos)] : drafter::line();
					gui::draw_clipped_rect(highlight_mat, xywhi(0, highlighted.top, clip ? d.get_bbox().x + clipper.w() : d.get_bbox().x,

						/* snap to default style's height */
						highlighted.empty() ? (*(caret->default_style.f)).meta_from_file.get_height()
						: highlighted.height()) + pos, clipper, v);
				}

				if (!lines.empty() && !sectors.empty()) {
					/* only these lines we want to process */
					std::pair<int, int> visible;

					if (clip)
						visible = d.get_line_visibility(clipper - pos);
					else visible = std::make_pair(0, lines.size() - 1);

					/* if this happens:
					- check if there is always an empty line
					- check if we return when clipper is not valid
					- check if scroll is always aligned */
					if (visible.first == -1)
						return;

					/* we'll need these variables later so we declare them here */
					unsigned select_left = 0;
					unsigned select_right = 0;
					unsigned caret_line = 0;

					/* if we specified a caret to draw
					(which is not always the case when printing static captions for example) */
					if (caret) {
						caret_line = d.get_line(caret->pos);

						/* let's calculate some values only once */
						select_left = caret->get_left_selection();
						select_right = caret->get_right_selection();

						if (caret->selection_offset) {
							unsigned select_left_line = d.get_line(select_left);
							unsigned select_right_line = d.get_line(select_right);
							unsigned first_visible_selection = std::max(select_left_line, unsigned(visible.first));
							unsigned last_visible_selection = std::min(select_right_line, unsigned(visible.second));

							/* manage selections */
							for (unsigned i = first_visible_selection; i <= last_visible_selection; ++i) {
								/* init selection rect on line rectangle;
								its values won't change if selecting between first and the last line
								*/
								ltrbi sel_rect = d.lines[i].get_rect();

								/* if it's the first line to process and we can see it, we have to trim its x coordinate */
								if (i == first_visible_selection && select_left_line >= first_visible_selection)
									sel_rect.l = d.sectors[select_left];

								/* similiarly with the last one
								note that with only one line selecting, it is still correct to apply these two conditions
								*/
								if (i == last_visible_selection && select_right_line <= last_visible_selection)
									sel_rect.r = d.sectors[select_right];

								gui::draw_clipped_rect(active ? selection_bg_mat : selection_inactive_bg_mat, sel_rect + pos, clipper, v);
							}
						}
					}

					/* for every visible line */
					for (unsigned l = visible.first; l <= unsigned(visible.second); ++l) {
						/* for every character in line */
						for (unsigned i = lines[l].begin; i < lines[l].end; ++i) {
							/* shortcut */
							auto& g = *d.cached[i];

							/* if it's not a whitespace */
							if (d.cached_atlas_entries[i]->original_size_pixels.x > 0) {
								rgba charcolor = style(colors[i]).color;

								/* if a character is between selection bounds, we change its color to the one specified in selected_text_color
								if there's no caret, this is never true
								*/
								if (caret && i > select_left && i < select_right)
									charcolor = selected_text_color;

								/* add the resulting character taking bearings into account */
								augs::draw_clipped_rect(v, xywhi(sectors[i] + g.bear_x, lines[l].top + lines[l].asc - g.bear_y, g.size.x, g.size.y) + pos, 
									*d.cached_atlas_entries[i], charcolor, clipper);
							}
						}
					}

					if (caret && active) {
						/* if we can retrieve some sane values */
						if (!lines[caret_line].empty()) {
							if (align_caret_height)
								caret_rect = xywhi(sectors[caret->pos], lines[caret_line].top, caret_width, lines[caret_line].height());
							else {
								int pos = std::max(1u, caret->pos);
								auto& glyph_font = *colors[pos - 1].font_used;
								caret_rect = xywhi(sectors[caret->pos], lines[caret_line].top + lines[caret_line].asc - glyph_font.meta_from_file.ascender,
									caret_width, glyph_font.meta_from_file.get_height());
							}
						}
						/* otherwise set caret's height to default style's height to avoid strange situations */
						else
							caret_rect = xywhi(0, d.lines[caret_line].top, caret_width, (*caret->default_style.f).meta_from_file.get_height());

					}
				}
				/* there is nothing to draw, but we are still active so we want to draw caret anyway */
				else if (active && caret)
					caret_rect = xywhi(0, 0, caret_width, (*caret->default_style.f).meta_from_file.get_height());

				//					this->quad_indices.caret = v.size();
				if (blink.caret_visible) gui::draw_clipped_rect(caret_mat, caret_rect + pos, clipper, v);
			}

			vec2i get_text_bbox(const formatted_string& str, const unsigned wrapping_width) {
				drafter dr;
				dr.wrap_width = wrapping_width;
				dr.draw(str);
				return dr.get_bbox();
			}

			vec2 quick_print(
				std::vector<augs::vertex_triangle>& v,
				const formatted_string& str,
				const vec2i pos,
				const unsigned wrapping_width,
				const ltrbi clipper
			) {
				drafter dr;
				printer pr;
				dr.wrap_width = wrapping_width;
				dr.draw(str);
				pr.draw_text(v, dr, str, 0, pos, clipper);
				return dr.get_bbox();
			}

			vec2 quick_print_format(
				std::vector<augs::vertex_triangle>& v,
				const std::wstring& wstr,
				const gui::text::style style,
				const vec2i pos,
				const unsigned wrapping_width,
				const ltrbi clipper
			) {
				formatted_string str = format(wstr.c_str(), style);
				drafter dr;
				printer pr;
				dr.wrap_width = wrapping_width;
				dr.draw(str);
				pr.draw_text(v, dr, str, 0, pos, clipper);
				return dr.get_bbox();
			}
		}
	}
}