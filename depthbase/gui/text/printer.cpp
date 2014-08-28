#pragma once
#include <algorithm>
#include "ui.h"
#include "drafter.h"
#include "printer.h"
#include "../rect.h"
#include "../../../window/window.h"

#undef min
#undef max

/* printer's draw needs revising in terms of scrolling */

namespace db {
	using namespace math;
	namespace graphics {
		using namespace io::input;
		namespace gui {
			namespace text {
				printer::printer() :
					caret_mat(material(null_texture, pixel_32(255, 255, 255, 255))),
					align_caret_height(true), caret_width(1),
					highlight_current_line(false),
					highlight_during_selection(true), 
					active(false), 
					selection_bg_mat(pixel_32(128, 255, 255, 120)),
					selection_inactive_bg_mat(pixel_32(128, 255, 255, 40)),
					highlight_mat(pixel_32(15, 15, 15, 255))
				{
					quad_indices.first_character = 
					quad_indices.last_character =  
					quad_indices.selections_first = 
					quad_indices.selections_last = 
					quad_indices.highlight = 
					quad_indices.caret = -1;
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
					if(timer.get_miliseconds() > interval_ms) {
						caret_visible = !caret_visible;
						timer.microseconds();
					}
					//if(blink_func) blink_func(*this, caret);
				}

				void printer::blinker::reset() {
					timer.microseconds();
					caret_visible = true;
				}
				
				void printer::draw_text(std::vector<quad>& out, ui& u, const rect& parent) const {
					draw_text(out, u.get_draft(), u.get_str(), &u.caret, parent);
				}

				void printer::draw_text(std::vector<quad>& out, 
						const drafter& d, 
						const fstr& colors,
						const caret_info* caret,
						const rect& subject
						) const 
				{
					/* note that parent's scroll is already taken into account by absolute_xy */
					draw_text(out, d, colors, caret, subject.get_absolute_xy() - subject.scroll, subject.clip ? &subject.get_clipping_rect() : &subject.get_parent()->get_clipping_rect());
				}
				void printer::draw_text(std::vector<quad>& out, 
						const drafter& d, 
						const fstr& colors,
						const caret_info* caret,
						point pos,
						const rect_ltrb* clipper
						) const 
				{
					/* shortcuts */
					auto& lines			= d.lines;
					auto& sectors		= d.sectors;
					auto& v = out;
					bool clip = clipper != nullptr;

					//if(clip) 
					//	pos = point(*parent);
					//
					//point global = scroll;

					/* we'll draw caret at the very end of procedure so we have to declare this variable here */
					rect_xywh caret_rect(0, 0, 0, 0);
					
					/* validations */
					/* return if we want to clip but the clipper is not a valid rectangle */
					if(clip && !clipper->good()) return; 

					/* here we highlight the line caret is currently on */
					if(caret && active && highlight_current_line) {
						drafter::line highlighted = lines.size() ? lines[d.get_line(caret->pos)] : drafter::line();
						gui::add_quad(highlight_mat, rect_xywh(0, highlighted.top, clipper ? d.get_bbox().w + clipper->w() : d.get_bbox().w,

							/* snap to default style's height */
							highlighted.empty() ? caret->default_style.f->parent->get_height() 
							: highlighted.height()) + pos, clipper, v);
					}

					if(!lines.empty() && !sectors.empty()) {
						/* only these lines we want to process */
						pair<int, int> visible;
						
						if(clip)
							visible = d.get_line_visibility(*clipper - pos);
						else visible = make_pair(0, lines.size()-1);

						/* if this happens:
						- check if there is always an empty line
						- check if we return when clipper is not valid
						- check if scroll is always aligned */
						if(visible.first == -1) 
							return;

						/* we'll need these variables later so we declare them here */
						unsigned select_left  =	0;
						unsigned select_right =	0;
						unsigned caret_line = 0;

						/* if we specified a caret to draw 
						(which is not always the case when printing static captions for example) */
						if(caret) {
							caret_line = d.get_line(caret->pos);

							/* let's calculate some values only once */
							select_left = caret->get_left_selection();
							select_right = caret->get_right_selection();

							if(caret->selection_offset) {
								unsigned select_left_line =  d.get_line(select_left);
								unsigned select_right_line = d.get_line(select_right);
								unsigned first_visible_selection = max(select_left_line, unsigned(visible.first));
								unsigned last_visible_selection	 = min(select_right_line, unsigned(visible.second));

								/* manage selections */
								for(unsigned i = first_visible_selection; i <= last_visible_selection; ++i) {
									/* init selection rect on line rectangle;
									its values won't change if selecting between first and the last line
									*/
									rect_ltrb sel_rect = d.lines[i].get_rect();

									/* if it's the first line to process and we can see it, we have to trim its x coordinate */
									if(i == first_visible_selection && select_left_line >= first_visible_selection)
										sel_rect.l = d.sectors[select_left];

									/* similiarly with the last one
									note that with only one line selecting, it is still correct to apply these two conditions
									*/
									if(i == last_visible_selection && select_right_line <= last_visible_selection)
										sel_rect.r = d.sectors[select_right];

									gui::add_quad(active ? selection_bg_mat : selection_inactive_bg_mat, sel_rect+pos, clipper, v); 
								}
							}
						}

						/* for every visible line */
						for(unsigned l = visible.first;l <= unsigned(visible.second); ++l) {
							/* for every character in line */
							for(unsigned i = lines[l].begin; i < lines[l].end; ++i) {
								/* shortcut */
								auto& g = *d.cached[i];

								/* if it's not a whitespace */
								if(g.tex.get_rect().good()) {
									pixel_32 charcolor = style(colors[i]).color;
									
									/* if a character is between selection bounds, we change its color to the one specified in selected_text_color 
									if there's no caret, this is never true
									*/
									if(caret && i > select_left && i < select_right)
										charcolor = selected_text_color;
									
									/* add the resulting character taking bearings into account */
									gui::add_quad(material(&g.tex, charcolor), 
									rect_xywh (sectors[i] + g.info->bear_x, lines[l].top + lines[l].asc - g.info->bear_y, g.info->size.w, g.info->size.h) + pos, clipper, 
									v);
								}
							}
						}

						if(caret && active) {
							/* if we can retrieve some sane values */
							if(!lines[caret_line].empty()) {
								if(align_caret_height)
									caret_rect = rect_xywh(sectors[caret->pos], lines[caret_line].top, caret_width, lines[caret_line].height());
								else {
									int pos = max(1u, caret->pos);
									auto& glyph_font = *colors[pos-1].font_used->parent;
									caret_rect = rect_xywh(sectors[caret->pos], lines[caret_line].top + lines[caret_line].asc - glyph_font.ascender, 
										caret_width, glyph_font.get_height());
								}
							}
							/* otherwise set caret's height to default style's height to avoid strange situations */
							else
								caret_rect = rect_xywh(0, d.lines[caret_line].top, caret_width, caret->default_style.f->parent->get_height());

						}
					} 
					/* there is nothing to draw, but we are still active so we want to draw caret anyway */
					else if(active && caret)
						caret_rect = rect_xywh(0, 0, caret_width, caret->default_style.f->parent->get_height());
					
//					this->quad_indices.caret = v.size();
					if(blink.caret_visible) gui::add_quad(caret_mat, caret_rect + pos, clipper, v); 
				}

				rect_wh quick_print(std::vector<quad>& v,
										const fstr& str, 
										point pos, 
										unsigned wrapping_width,
										const rect_ltrb* clipper) 
				{
					drafter dr;
					printer pr;
					dr.wrap_width = wrapping_width;
					dr.draw(str);
					pr.draw_text(v, dr, str, 0, pos, clipper);
					return dr.get_bbox();
				}
				
				rect_wh quick_print(std::vector<quad>& v,
										const std::wstring& wstr,
										gui::text::style style,
										point pos, 
										unsigned wrapping_width,
										const rect_ltrb* clipper) 
				{
					fstr str = format(wstr.c_str(), style);
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
}