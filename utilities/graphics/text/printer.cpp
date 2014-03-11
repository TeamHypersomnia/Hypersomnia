#pragma once
#include <algorithm>
#include "drafter.h"
#include "printer.h"

/* printer's draw needs revising in terms of scrolling */

namespace augs {
	namespace graphics {
		namespace gui {
			namespace text {
				printer::printer() :
					align_caret_height(true), caret_width(1),
					highlight_current_line(false),
					highlight_during_selection(true), 
					active(false) 
				{
					quad_indices.first_character = 
					quad_indices.last_character =  
					quad_indices.selections_first = 
					quad_indices.selections_last = 
					quad_indices.highlight = 
					quad_indices.caret = -1;
				}			

				void printer::draw_text(
					resources::renderable::draw_input my_input,
					const drafter& d,
					const fstr& colors,
					/* if caret is 0, draw no caret */
					vec2<int> pos,
					float size_multiplier,
					const rects::ltrb<int>* clipper) const
				{
					/* shortcuts */
					auto& lines			= d.lines;
					auto& sectors		= d.sectors;
					bool clip = clipper != nullptr;

					//if(clip) 
					//	pos = vec2<int>(*parent);
					//
					//vec2<int> global = scroll;

					/* we'll draw caret at the very end of procedure so we have to declare this variable here */
					rects::xywh<int> caret_rect(0, 0, 0, 0);
					
					/* validations */
					/* return if we want to clip but the clipper is not a valid rectangle */
					if(clip && !clipper->good()) return; 

					if(!lines.empty() && !sectors.empty()) {
						/* only these lines we want to process */
						std::pair<int, int> visible;
						
						if(clip)
							visible = d.get_line_visibility(*clipper - pos);
						else visible = std::make_pair(0, lines.size()-1);

						/* if this happens:
						- check if there is always an empty line
						- check if we return when clipper is not valid
						- check if scroll is always aligned */
						if(visible.first == -1) 
							return;

						/* for every visible line */
						for(unsigned l = visible.first;l <= unsigned(visible.second); ++l) {
							/* for every character in line */
							for(unsigned i = lines[l].begin; i < lines[l].end; ++i) {
								/* shortcut */
								auto& g = *d.cached[i];

								/* if it's not a whitespace */
								if(g.tex.get_rect().good()) {
									pixel_32 charcolor = style(colors[i]).color;
									
									auto my_input_copy = my_input;
									
									resources::sprite new_sprite;

									//new_sprite.size = vec2<float>(g.info->size.w, g.info->size.h)*size_multiplier;	
									
									new_sprite.set(&g.tex, charcolor);
									new_sprite.size *= size_multiplier;
									my_input_copy.transform.pos += vec2<float>(sectors[i] + g.info->bear_x, lines[l].top + lines[l].asc - g.info->bear_y) * size_multiplier + pos + new_sprite.size / 2;
								
									new_sprite.draw(my_input_copy);
									
									/* add the resulting character taking bearings into account */
								}
							}
						}
					} 
				}

				vec2<int> get_text_bbox(const std::vector<formatted_char>& str, unsigned wrapping_width) {
					drafter dr;
					printer pr;
					dr.wrap_width = wrapping_width;
					dr.draw(str);
					return vec2<int>(dr.get_bbox().w, dr.get_bbox().h);
				}

				vec2<int> quick_print(resources::renderable::draw_input v,
										const fstr& str, 
										vec2<int> pos, 
										float size_multiplier,
										unsigned wrapping_width) 
				{
					drafter dr;
					printer pr;
					dr.wrap_width = wrapping_width;
					dr.draw(str);
					pr.draw_text(v, dr, str, pos, size_multiplier, 0);
					return vec2<int>(dr.get_bbox().w, dr.get_bbox().h);
				}
				
				rects::wh<int> quick_print_format(resources::renderable::draw_input v,
										const std::wstring& wstr,
										gui::text::style style,
										vec2<int> pos, 
										float size_multiplier,
										unsigned wrapping_width,
										rects::ltrb<int>* clipper) 
				{
					fstr str = format(wstr.c_str(), style);
					drafter dr;
					printer pr;
					dr.wrap_width = wrapping_width;
					dr.draw(str);
					pr.draw_text(v, dr, str, pos, size_multiplier, clipper);
					return dr.get_bbox();
				}

				
			}
		}
	}
}