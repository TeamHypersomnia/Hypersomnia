#include "controls.h"

namespace augs {
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

					checkbox_active = stylesheet(stylesheet::style(ltblue, assets::texture_id::BLANK, solid_stroke(0)));
					checkbox_active.hovered.border = solid_stroke(1, ltblue);
					checkbox_active.pushed.border = solid_stroke(2, ltblue);

					checkbox_inactive = stylesheet(checkbox_active);
					checkbox_inactive.released.color = white;

					rect_style = stylesheet(stylesheet::style(rgba(darkblue.r, darkblue.g, darkblue.b, 0), assets::texture_id::BLANK, solid_stroke(0)));
					textbox_style = stylesheet(stylesheet::style(darkblue, assets::texture_id::BLANK, solid_stroke(1, gray2)));
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
					: styles(styles), rect(r) {
					focusable = false;
				}

				cchecklabel::cchecklabel(const checklabel& c, const stylesheet& styles_active, const stylesheet& styles_inactive)
					: styles_active(styles_active), styles_inactive(styles_inactive), checklabel(c) {}

				ccheckbox::ccheckbox(const checkbox& c, const stylesheet& styles_active, const stylesheet& styles_inactive)
					: styles_active(styles_active), styles_inactive(styles_inactive), checkbox(c) {}


				void crect::consume_gui_event(event_info m) {
					rect::consume_gui_event(m);
					styles.update_appearance(m);
				}

				void cbutton::consume_gui_event(event_info m) {
					button::consume_gui_event(m);
					styles.update_appearance(m);
				}

				void ctext_button::consume_gui_event(event_info m) {
					text_button::consume_gui_event(m);
					styles.update_appearance(m);
				}

				void ccheckbox::consume_gui_event(event_info m) {
					checkbox::consume_gui_event(m);
					styles_active.update_appearance(m);
					styles_inactive.update_appearance(m);
				}

				void cchecklabel::consume_gui_event(event_info m) {
					checklabel::consume_gui_event(m);
					styles_active.update_appearance(m);
					styles_inactive.update_appearance(m);
				}

				void cslider::consume_gui_event(event_info m) {
					slider::consume_gui_event(m);
					styles.update_appearance(m);
				}

				void cscrollarea::consume_gui_event(event_info m) {
					scrollarea::consume_gui_event(m);
					styles.update_appearance(m);
				}

				void ctextbox::consume_gui_event(event_info m) {
					textbox::consume_gui_event(m);
					styles.update_appearance(m);
				}

				void cproperty_textbox::consume_gui_event(event_info m) {
					property_textbox::consume_gui_event(m);
					styles.update_appearance(m);
				}

				void crect::draw_triangles(draw_info in) {
					draw_rectangle_stylesheeted(in, styles);
					rect::draw_triangles(in);
				}

				void cbutton::draw_triangles(draw_info in) {
					draw_rectangle_stylesheeted(in, styles);
					button::draw_triangles(in);
				}

				void ctext_button::draw_triangles(draw_info in) {
					draw_rectangle_stylesheeted(in, styles);
					text_button::draw_triangles(in);
				}

				void ccheckbox::draw_triangles(draw_info in) {
					draw_rectangle_stylesheeted(in, get_state() ? styles_active : styles_inactive);
					checkbox::draw_triangles(in);
				}

				void cchecklabel::draw_triangles(draw_info in) {
					draw_rectangle_stylesheeted(in, get_state() ? styles_active : styles_inactive);
					checklabel::draw_triangles(in);
				}

				void cslider::draw_triangles(draw_info in) {
					draw_rectangle_stylesheeted(in, styles);
				}

				void cscrollarea::draw_triangles(draw_info in) {
					draw_rectangle_stylesheeted(in, styles);
					scrollarea::draw_triangles(in);
				}

				void ctextbox::draw_triangles(draw_info in) {
					draw_rectangle_stylesheeted(in, styles);
					textbox::draw_triangles(in);
				}

				void cproperty_textbox::draw_triangles(draw_info in) {
					draw_rectangle_stylesheeted(in, styles);
					property_textbox::draw_triangles(in);
				}
			}
		}
	}
}

