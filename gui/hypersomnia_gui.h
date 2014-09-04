#pragma once
#include <functional>
#include "depthbase/gui/system.h"
#include "depthbase/gui/controls/stylesheeted/controls.h"
#include "misc/timer.h"
#include "math/vec2d.h"
#include "graphics/pixel.h"

#include "game_framework/resources/render_info.h"
namespace augs {
	namespace misc {
		/* vector wrapper that is used to faciliate binding to lua */
		template<class value>
		struct vector_wrapper;
	}
}

const int IMAGES = 1;
const int FONTS = 1;

namespace luabind {
	struct object;
}

using namespace augs::window;
using namespace augs::graphics::gui::controls::stylesheeted;
using namespace augs::graphics::gui::controls;

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
			if (m.msg == rect::event::character && m.owner.owner.events.utf16 == window::event::keys::ENTER) {
				return;
			}

			if (m.msg == rect::event::keydown) {
				if (m.owner.owner.events.key == window::event::keys::ENTER && !m.owner.owner.events.keys[window::event::keys::LSHIFT]) {
					if (command_callback)
						command_callback(augs::misc::wstr(editor.get_str()));

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

	augs::window::glwindow& gl;

	augs::graphics::gui::system sys;

	hypersomnia_gui(augs::window::glwindow& gl, texture_baker::texture* blank_texture);

	void update();

	static void bind(augs::lua_state_wrapper&);
};

struct gui_group {
	augs::graphics::gui::group main_window;
	void poll_events();
	void draw_call(resources::renderable::draw_input& in);
	void blur();

	gui_group(hypersomnia_gui& owner);
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
	gui_group* owner = nullptr;

	callback_rect(gui_group& owner);
	
	void focus();
	void setup(augs::rects::xywh<float>, bool focusable);
	void set_focus_callback(luabind::object), set_lpressed_callback(luabind::object), set_hover_callback(luabind::object), set_blur_callback(luabind::object);
};

struct callback_textbox {
	cslider sl = cslider(20);
	cslider slh = cslider(20);
	cscrollarea myscrtx = cscrollarea(scrollarea(rects::xywh<float>(0, 0, 10, 0), &textbox_object, &sl, scrollarea::orientation::VERTICAL));
	cscrollarea myscrhtx = cscrollarea(scrollarea(rects::xywh<float>(0, 0, 0, 10), &textbox_object, &slh, scrollarea::orientation::HORIZONTAL));

	augs::graphics::gui::text::style   active;
	augs::graphics::gui::text::style inactive;
	
	textbox_wrapper textbox_object;
	
	gui_group* owner = nullptr;
	callback_textbox() {}
	callback_textbox(gui_group& owner);
	
	void set_command_callback(luabind::object);
	
	void focus();
	void append_text(augs::graphics::gui::text::fstr&, bool set_default_font);
	void clear_text();
	bool is_clean();
	bool is_focused();

	void set_alpha_range(int pos1, int pos2, unsigned val);

	void set_caret(unsigned pos, bool select = false);
	unsigned get_length();
	void remove_line();
	void backspace();

	void view_caret();

	augs::vec2<> get_text_bbox();

	void draw(bool);
	
	void set_area(augs::rects::xywh<float>);
	void setup(augs::rects::xywh<float>, bool is_input_textbox, texture_baker::font*);
};
