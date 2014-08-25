#pragma once
#include "hypersomnia_gui.h"

#include "../../MicroPlus/code/depthbase/include/db.h"

using namespace db::graphics::gui::controls::stylesheeted;

const int IMAGES = 1;
const int FONTS = 1;

DB_USE_ALL_NAMESPACES;

struct callback_textbox : public ctextbox {
	void event_proc(event_info m) override {

		if (m.msg == rect::event::character || m.msg == rect::event::keydown) {
			if (m.owner.owner.events.utf16 == db::event::keys::ENTER && !m.owner.owner.events.keys[db::event::keys::LSHIFT]) {
				//this->editor.select_all();
				//this->editor.backspace();
				//
				//return;
			}
		}

		ctextbox::event_proc(m);
	}

	callback_textbox(const ctextbox& t) : ctextbox(t) {}
};

namespace augs {
	struct lua_state_wrapper;
}

struct hypersomnia_gui {
	glwindow gl;
	augs::window::glwindow& actual_window;

	image images[IMAGES];
	texture textures[IMAGES];

	font_file fontf[FONTS];
	font      fonts[FONTS];
	
	io::input::atlas atl;

	gui::system sys = gui::system(gl.events);
	gui::group main_window = gui::group(sys);

	crect background = crect(rect_ltrb(0, 0, 800 - 20, 600 - 20));

	callback_textbox  chat_textbox = callback_textbox(ctextbox(textbox(rect_xywh(rect_ltrb(10, 17, 800 - 20, 600 - 20)), text::style(fonts + 0, white))));

	cslider   sl = cslider(20);
	cslider  slh = cslider(20);
	cscrollarea  myscrtx = cscrollarea(scrollarea(rect_xywh(0, 0, 10, 0), &chat_textbox, &sl, scrollarea::orientation::VERTICAL));
	cscrollarea myscrhtx = cscrollarea(scrollarea(rect_xywh(0, 0, 0, 10), &chat_textbox, &slh, scrollarea::orientation::HORIZONTAL));

	text::style   active = text::style(fonts + 1, ltblue);
	text::style inactive = text::style(fonts + 0, white);

	hypersomnia_gui(augs::window::glwindow& actual_window);

	void poll_events();
	void draw_call();

	static void bind(augs::lua_state_wrapper&);
};