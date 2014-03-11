#pragma once
#include "system.h"
#include "graphics/vertex.h"
#include "graphics/pixel.h"

#include "../../../game_framework/resources/render_info.h"

// got to revise gui systems in terms of rectangle update'ing
namespace augs {
	namespace graphics {
		namespace gui {
			namespace text {
				struct drafter;
				struct printer {
					/* these integers hold information at what index particular quads are pushed to the resulting vector
					may be useful for further processing
					still needs implementing
					*/
					struct index_info {
						int first_character, last_character, 
							selections_first, selections_last,
							highlight,
							caret;
					} quad_indices;

					pixel_32 selected_text_color;
					
					unsigned caret_width; 
					
					bool active, 
						align_caret_height, /* whether caret should be always of line height */
						highlight_current_line, 
						highlight_during_selection;
					
					printer();

					void draw_text(
						resources::renderable::draw_input out,
						const drafter&,
						const fstr& colors,
						/* if caret is 0, draw no caret */
						vec2<int> scroll,
						const rects::ltrb<int>* parent = 0) const;
		 		};
				
				/* 
				parent shifts position and clips the text
				wrapping_width = 0 means no wrapping
				 parent = 0 means no clipping/shifting
				returns text's bounding box (without clipping)
				*/

				vec2<int> get_text_bbox(const std::vector<formatted_char>& str, unsigned wrapping_width = 0);
				vec2<int> quick_print(resources::renderable::draw_input v,
										const std::vector<formatted_char>& str, 
										vec2<int> pos, 
										unsigned wrapping_width = 0);

				rects::wh<int> quick_print_format(resources::renderable::draw_input v,
										const std::wstring& wstr,
										style style,
										vec2<int> pos, 
										unsigned wrapping_width = 0,
										rects::ltrb<int>* parent = 0);
			}
		}
	}
}