#pragma once
#include "../system.h"
// got to revise gui systems in terms of rectangle update'ing
namespace db {
	namespace graphics {
		namespace gui {
			namespace text {
				struct caret_info;
				class ui;
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

					/* defines how the caret should blink and whether should blink at all */
					struct blinker {
						bool blink, caret_visible;
						int interval_ms;

						//static void regular_blink(blinker&, quad& caret);
						//void (*blink_func)(blinker&, quad&);

						window::timer timer;
						blinker();
						void update();
						void reset();
					};

					blinker blink;
					pixel_32 selected_text_color;
					
					unsigned caret_width; 
					
					bool active, 
						align_caret_height, /* whether caret should be always of line height */
						highlight_current_line, 
						highlight_during_selection;
					
					material caret_mat, 
						highlight_mat, 
						selection_bg_mat,
						selection_inactive_bg_mat; /* material for line highlighting */
					
					printer();

					void draw_text(std::vector<quad>& out, ui&, const rect& parent) const;
					
					void draw_text(
						std::vector<quad>& out, 
						const drafter&, 
						const fstr& colors, 
						/* if caret is 0, draw no caret */
						const caret_info* caret,
						const rect& parent
						) const;
					
					void draw_text(
						std::vector<quad>& out, 
						const drafter&, 
						const fstr& colors,
						/* if caret is 0, draw no caret */
						const caret_info* caret,
						point scroll,
						const rect_ltrb* parent = 0) const;
		 		};
				
				/* 
				parent shifts position and clips the text
				wrapping_width = 0 means no wrapping
				 parent = 0 means no clipping/shifting
				returns text's bounding box (without clipping)
				*/

				rect_wh quick_print(std::vector<quad>& v,
										const fstr& str, 
										point pos, 
										unsigned wrapping_width = 0,
										const rect_ltrb* parent = 0);

				rect_wh quick_print(std::vector<quad>& v,
										const std::wstring& wstr,
										style style,
										point pos, 
										unsigned wrapping_width = 0,
										const rect_ltrb* parent = 0);
			}
		}
	}
}