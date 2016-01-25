#pragma once
#include "../../stylesheet.h"
#include "../checkbox.h"
#include "../scrollarea.h"
#include "../textbox.h"
#include "../button.h"

namespace augs {
	namespace graphics {
		namespace gui {
			namespace controls {
				namespace stylesheeted {
					extern stylesheet rect_style;
					extern stylesheet button_style;
					extern stylesheet text_button_style;
					extern stylesheet checklabel_inactive;
					extern stylesheet checklabel_active;
					extern stylesheet checkbox_active;
					extern stylesheet checkbox_inactive;
					extern stylesheet textbox_style;
					extern stylesheet property_textbox_style;
					extern stylesheet slider_style;
					extern stylesheet scrollarea_style;
					
					extern void ltblue_theme();

					struct crect : public rect {
						stylesheet styles;
						crect(const rects::xywh<float>& r = rects::xywh<float>(), const stylesheet& styles = rect_style);

						virtual void draw_proc(draw_info in) override;
						virtual void consume_gui_event(event_info) override;
					};
					
					struct cbutton : public button {
						stylesheet styles;
						cbutton(const button& r = button(), const stylesheet& styles = button_style);

						virtual void draw_proc(draw_info in) override;
						virtual void consume_gui_event(event_info) override;
					};

					struct ctext_button : public text_button {
						stylesheet styles;
						ctext_button(const text_button& r, const stylesheet& styles = text_button_style);

						virtual void draw_proc(draw_info in) override;
						virtual void consume_gui_event(event_info) override;
					};

					struct ccheckbox : public checkbox {
						stylesheet styles_inactive, styles_active;

						ccheckbox(const checkbox& = checkbox(), const stylesheet& styles_active = checkbox_active, const stylesheet& styles_inactive = checkbox_inactive);

						virtual void draw_proc(draw_info in) override;
						virtual void consume_gui_event(event_info) override;
					};

					struct cchecklabel : public controls::checklabel {
						stylesheet styles_inactive, styles_active;

						cchecklabel(const checklabel&, const stylesheet& styles_active = checklabel_active, const stylesheet& styles_inactive = checklabel_inactive);

						virtual void draw_proc(draw_info in) override;
						virtual void consume_gui_event(event_info) override;
					};
					
					struct ctextbox : public textbox {
						stylesheet styles;
						ctextbox(const textbox& r = textbox(), const stylesheet& = textbox_style);

						virtual void draw_proc(draw_info in) override;
						virtual void consume_gui_event(event_info) override;
					};

					struct cproperty_textbox : public property_textbox {
						stylesheet styles;
						cproperty_textbox(const property_textbox& r, const stylesheet& = property_textbox_style);

						virtual void draw_proc(draw_info in) override;
						virtual void consume_gui_event(event_info) override;
					};

					struct cslider : public scrollarea::slider {
						stylesheet styles;

						cslider(const scrollarea::slider& r, const stylesheet& = slider_style);

						virtual void draw_proc(draw_info in) override;
						virtual void consume_gui_event(event_info m) override;
					};

					struct cscrollarea : public scrollarea {
						stylesheet styles;

						cscrollarea(const scrollarea& r, const stylesheet& = scrollarea_style);

						virtual void draw_proc(draw_info in) override;
						virtual void consume_gui_event(event_info m) override;
					};
				}
			}
		}
	}
}