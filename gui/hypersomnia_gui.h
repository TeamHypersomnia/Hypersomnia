#pragma once
#include "../../MicroPlus/code/depthbase/include/db.h"
#include "misc/timer.h"
#include "math/vec2d.h"
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

	callback_textbox(const ctextbox& t = ctextbox()) : ctextbox(t) {

	}
};

namespace augs {
	struct lua_state_wrapper;

	namespace window {
		struct glwindow;
	}
}

struct hypersomnia_gui {
	augs::misc::timer delta_timer;

	glwindow gl;
	augs::window::glwindow& actual_window;

	image images[IMAGES];
	texture textures[IMAGES];

	font_file fontf[FONTS];
	font      fonts[FONTS];
	
	io::input::atlas atl;
	
	gui::system sys = gui::system(gl.events);
	gui::group main_window = gui::group(sys);

	hypersomnia_gui(augs::window::glwindow& actual_window) : actual_window(actual_window) {}

	void setup();

	void poll_events();
	void draw_call();

	static void bind(augs::lua_state_wrapper&);
};


struct command_textbox {
	cslider sl = cslider(20);
	cslider slh = cslider(20);
	cscrollarea myscrtx = cscrollarea(scrollarea(rect_xywh(0, 0, 10, 0), &textbox_object, &sl, scrollarea::orientation::VERTICAL));
	cscrollarea myscrhtx = cscrollarea(scrollarea(rect_xywh(0, 0, 0, 10), &textbox_object, &slh, scrollarea::orientation::HORIZONTAL));

	text::style   active;
	text::style inactive;
	
	callback_textbox  textbox_object;

	bool was_added = false;

	hypersomnia_gui* owner = nullptr;
	command_textbox() {}
	command_textbox(hypersomnia_gui& owner);
	void setup(augs::rects::xywh<float>);
};