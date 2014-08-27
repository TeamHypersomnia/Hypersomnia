#pragma once
#include <functional>
#include "../../MicroPlus/code/depthbase/include/db.h"
#include "misc/timer.h"
#include "math/vec2d.h"
#include "graphics/pixel.h"

namespace augs {
	namespace misc {
		/* vector wrapper that is used to faciliate binding to lua */
		template<class value>
		struct vector_wrapper;
	}
}

using namespace db::graphics::gui::controls::stylesheeted;

const int IMAGES = 1;
const int FONTS = 1;

DB_USE_ALL_NAMESPACES;

namespace luabind {
	struct object;
}

struct textbox_wrapper : public ctextbox {
	cscrollarea* sl = nullptr;
	cscrollarea* slh = nullptr;

	std::function<void(std::wstring)> command_callback;
	bool is_input_textbox;

	void event_proc(event_info m) override {
		if (sl && slh) {
			if (m.msg == rect::event::focus) {
				sl->enabled = true;
				slh->enabled = true;
			}
			if (m.msg == rect::event::blur) {
				sl->enabled = false;
				slh->enabled = false;
			}
		}

		if (is_input_textbox) {
			if (m.msg == rect::event::character || m.msg == rect::event::keydown) {
				if (m.owner.owner.events.utf16 == db::event::keys::ENTER && !m.owner.owner.events.keys[db::event::keys::LSHIFT]) {
					if (command_callback)
						command_callback(wstr(editor.get_str()));

					editor.select_all();
					editor.backspace();

					return;
				}
			}

		}
		else {

		}

		ctextbox::event_proc(m);
	}

	textbox_wrapper(const ctextbox& t = ctextbox()) : ctextbox(t) {}
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
	rect focusable_bg;

	hypersomnia_gui(augs::window::glwindow& actual_window) : actual_window(actual_window) {}

	augs::vec2<> camera_size;
	void setup(augs::vec2<> camera_size);

	void poll_events();
	void draw_call();
	void blur();

	static void bind(augs::lua_state_wrapper&);
};


struct rect_wrapper : crect {
	std::function<void()> focus_callback, lpressed_callback, hover_callback, blur_callback;

	void event_proc(event_info m) override {
		auto msg = m.msg;
		switch (msg) {
		case rect::event::blur: if (blur_callback) blur_callback(); break;
		case rect::event::focus: if (focus_callback) focus_callback(); break;
		case rect::event::lpressed: if (lpressed_callback) lpressed_callback(); break;
		case rect::event::hover: if (hover_callback) hover_callback(); break;
		default: break;
		}

		crect::event_proc(m);
	}

	rect_wrapper(const crect& t = crect()) : crect(t) {}
};

struct callback_rect {
	rect_wrapper rect_obj;
	hypersomnia_gui* owner = nullptr;

	callback_rect(hypersomnia_gui& owner);
	
	void focus();
	void setup(augs::rects::xywh<float>, bool focusable);
	void set_focus_callback(luabind::object), set_lpressed_callback(luabind::object), set_hover_callback(luabind::object), set_blur_callback(luabind::object);
};

struct callback_textbox {
	cslider sl = cslider(20);
	cslider slh = cslider(20);
	cscrollarea myscrtx = cscrollarea(scrollarea(rect_xywh(0, 0, 10, 0), &textbox_object, &sl, scrollarea::orientation::VERTICAL));
	cscrollarea myscrhtx = cscrollarea(scrollarea(rect_xywh(0, 0, 0, 10), &textbox_object, &slh, scrollarea::orientation::HORIZONTAL));

	text::style   active;
	text::style inactive;
	
	textbox_wrapper textbox_object;
	
	hypersomnia_gui* owner = nullptr;
	callback_textbox() {}
	callback_textbox(hypersomnia_gui& owner);
	
	void set_command_callback(luabind::object);
	
	void append_text(augs::misc::vector_wrapper<wchar_t>&, augs::graphics::pixel_32);
	void clear_text();

	void setup(augs::rects::xywh<float>, bool is_input_textbox);
};