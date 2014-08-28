#pragma once
#include "controls.h"

namespace augs {
	namespace graphics {
		namespace gui {
			namespace controls {
				namespace stylesheeted {
					stylesheet rect_style;
					stylesheet button_style;
					stylesheet text_button_style;
					stylesheet checklabel_inactive;
					stylesheet checklabel_active;
					stylesheet checkbox_active;
					stylesheet checkbox_inactive;
					stylesheet textbox_style;
					stylesheet property_textbox_style;
					stylesheet slider_style;
					stylesheet scrollarea_style;

					void ltblue_theme() {
						using namespace colors;

						slider_style = stylesheet();
						slider_style.released.color = gray3;
						slider_style.hovered.color = gray4;
						slider_style.pushed.color = white;

						scrollarea_style = stylesheet();
						scrollarea_style.released.color = gray2;
						
						checklabel_inactive = stylesheet();
						checklabel_inactive.released.border = solid_stroke(1, gray1);
						checklabel_inactive.hovered.border = solid_stroke(1, ltblue);
						checklabel_inactive.pushed.border = solid_stroke(2, ltblue);
					
						button_style = checklabel_inactive;
						button_style.focused.border = solid_stroke(1, gray3, solid_stroke::INSIDE);
						text_button_style = button_style;

						checklabel_active = stylesheet();
						checklabel_active.released.border = solid_stroke(1, ltblue);
						checklabel_active.hovered.border = solid_stroke(2, ltblue);
						checklabel_active.pushed.border = solid_stroke(3, ltblue);

						// checklabel_active.focused.border = solid_stroke(1, gray3);
						// checklabel_inactive.focused.border = solid_stroke(1, gray3);

						checkbox_active = stylesheet(stylesheet::style(ltblue, gui::null_texture, solid_stroke(0)));
						checkbox_active.hovered.border = solid_stroke(1, ltblue);
						checkbox_active.pushed.border = solid_stroke (2, ltblue);
						
						checkbox_inactive = stylesheet(checkbox_active);
						checkbox_inactive.released.color = white;

						rect_style = stylesheet(stylesheet::style(pixel_32(darkblue.r, darkblue.g, darkblue.b, 0), gui::null_texture, solid_stroke(0)));
						textbox_style = stylesheet(stylesheet::style(darkblue, gui::null_texture, solid_stroke(1, gray2)));
						textbox_style.focused.border = solid_stroke(1, ltblue);

						property_textbox_style = stylesheet();
						property_textbox_style = stylesheet(textbox_style);
					}
					
					cslider::cslider(const scrollarea::slider& r, const stylesheet& styles) 
						: styles(styles), slider(r) {}

					cbutton::cbutton(const button& r, const stylesheet& styles) 
						: styles(styles), button(r) {}
						
					ctext_button::ctext_button(const text_button& r, const stylesheet& styles)
						: styles(styles), text_button(r) {
					}

					cscrollarea::cscrollarea(const scrollarea& r, const stylesheet& styles) 
						: styles(styles), scrollarea(r) {}
					
					ctextbox::ctextbox(const textbox& r, const stylesheet& styles)
						: styles(styles), textbox(r) {}

					cproperty_textbox::cproperty_textbox(const property_textbox& r, const stylesheet& styles)
						: styles(styles), property_textbox(r) {}

					crect::crect(const rects::xywh<float>& r, const stylesheet& styles) 
						: styles(styles), rect(r)  { focusable = false; }

					cchecklabel::cchecklabel(const checklabel& c, const stylesheet& styles_active, const stylesheet& styles_inactive) 
						: styles_active(styles_active), styles_inactive(styles_inactive), checklabel(c) {}

					ccheckbox::ccheckbox(const checkbox& c, const stylesheet& styles_active, const stylesheet& styles_inactive) 
						: styles_active(styles_active), styles_inactive(styles_inactive), checkbox(c) {}

					
					void crect::event_proc(event_info m) {
						rect::event_proc(m);
						styles.update_appearance(m);
					}
					
					void cbutton::event_proc(event_info m) {
						button::event_proc(m);
						styles.update_appearance(m);
					}

					void ctext_button::event_proc(event_info m) {
						text_button::event_proc(m);
						styles.update_appearance(m);
					}

					void ccheckbox::event_proc(event_info m) {
						checkbox::event_proc(m);
						styles_active  .update_appearance(m);
						styles_inactive.update_appearance(m);
					}

					void cchecklabel::event_proc(event_info m) {
						checklabel::event_proc(m);
						styles_active  .update_appearance(m);
						styles_inactive.update_appearance(m);
					}

					void cslider::event_proc(event_info m) {
						slider::event_proc(m);
						styles.update_appearance(m);
					}

					void cscrollarea::event_proc(event_info m) {
						scrollarea::event_proc(m);
						styles.update_appearance(m);
					}
					
					void ctextbox::event_proc(event_info m) {
						textbox::event_proc(m);
						styles.update_appearance(m);
					}
					
					void cproperty_textbox::event_proc(event_info m) {
						property_textbox::event_proc(m);
						styles.update_appearance(m);
					}
					
					void crect::draw_proc(draw_info in) {
						draw_rect(in, styles);
						draw_children(in);
					}
					
					void cbutton::draw_proc(draw_info in) {
						draw_rect(in, styles);
						draw_children(in);
					}

					void ctext_button::draw_proc(draw_info in) {
						draw_rect(in, styles);
						draw_children(in);
					}

					void ccheckbox::draw_proc(draw_info in) {
						draw_rect(in, get_state() ? styles_active : styles_inactive);
						draw_children(in);
					}

					void cchecklabel::draw_proc(draw_info in) {
						draw_rect(in, get_state() ? styles_active : styles_inactive);
						draw_children(in);
					}
					
					void cslider::draw_proc(draw_info in) {
						draw_rect(in, styles);
					}

					void cscrollarea::draw_proc(draw_info in) {
						draw_rect(in, styles);
						draw_slider(in);
					}
					
					void ctextbox::draw_proc(draw_info in) {					
						draw_rect(in, styles);
						draw_text_ui(in);
					}

					void cproperty_textbox::draw_proc(draw_info in) {					
						draw_rect(in, styles);
						draw_text_ui(in);
					}
				}
			}
		}
	}
}

