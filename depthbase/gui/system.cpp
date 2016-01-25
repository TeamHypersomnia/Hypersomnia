#pragma once
#include "system.h"
#include "rect.h"
#include "window_framework/window.h"
#include <GL/OpenGL.h>
#undef max
namespace augs {
	namespace misc {
		std::wstring wstr(const graphics::gui::text::fstr& f) {
			size_t l = f.size();
			std::wstring ww;
			ww.reserve(l);
			for(size_t i = 0; i < l; ++i)
				ww += f[i].c;

			return ww;
		}
	}

	namespace graphics {
		namespace gui {
			augs::texture* null_texture = 0;

			namespace text {
				void format(const wchar_t* _str, style s, fstr& out) {
					out.clear();
					formatted_char ch;
					int len = wcslen(_str);
					for(int i = 0; i < len; ++i) {
						ch.set(_str[i], s.f, s.color);
						out.append(1, ch);
					}
				}

				fstr format(const wchar_t* _str, style s) {
					fstr out;	

					formatted_char ch;
					ch.font_used = s.f;
					int len = wcslen(_str);

					//out.reserve(len);
					for(int i = 0; i < len; ++i) {
						ch.set(_str[i], s.f, s.color);
						out.append(1, ch);
					}

					return out;
				}

				void format(const std::wstring& _str, style s, fstr& out) {
					out.clear();
					formatted_char ch;
					int len = _str.length();
					for(int i = 0; i < len; ++i) {
						ch.set(_str[i], s.f, s.color);
						out.append(1, ch);
					}
				}

				fstr format(const std::wstring& _str, style s) {
					fstr out;	

					formatted_char ch;
					ch.font_used = s.f;
					int len = _str.length();

					//out.reserve(len);
					for(int i = 0; i < len; ++i) {
						ch.set(_str[i], s.f, s.color);
						out.append(1, ch);
					}

					return out;
				}
			}


			namespace text {
				void formatted_char::set(wchar_t ch, font* f, const rgba& p) {
					font_used = f;
					c = ch;
					memcpy(&r, &p, sizeof(rgba));
				}

				void formatted_char::set(font* f, const rgba& p) {
					font_used = f;
					memcpy(&r, &p, sizeof(rgba));
				}

				style::style(font* f, rgba c) : f(f), color(c) {}

				style::style(const formatted_char& c) : f(c.font_used), color(rgba(c.r, c.g, c.b, c.a)) {}

				style::operator formatted_char() {
					formatted_char c;
					c.set(f, color);
					return c;
				}
			}

			system::system(augs::window::event::state& events) : events(events), own_copy(false), own_clip(false), fetch_clipboard(true) {
			}

			bool system::is_clipboard_own() {
				return own_clip;
			}
				
			group::group(system& owner) : owner(owner), rect_in_focus(&root) {
				root.clip = false;
				root.focusable = false;
				root.scrollable = false;
			}
			
			void group::set_focus(rect* f) {
				if(f == rect_in_focus) return;
				update_rectangles();

				if(rect_in_focus)
					rect_in_focus->event_proc(rect::event_info(*this, rect::event::blur));
				
				rect_in_focus = f;
				if(f) {
					f->event_proc(rect::event_info(*this, rect::event::focus));
					//f->scroll_to_view();
				}
			}
			
			rect* group::get_rect_in_focus() const {
				return rect_in_focus;
			}

			void group::draw_gl_fixed() {
				glVertexPointer(2, GL_FLOAT, sizeof(augs::vertex_triangle), quad_array.data());
				glTexCoordPointer(2, GL_FLOAT, sizeof(augs::vertex_triangle), (char*) (quad_array.data()) + sizeof(int) * 2);
				glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(augs::vertex_triangle), (char*) (quad_array.data()) + sizeof(int) * 2 + sizeof(float) * 2);
				glDrawArrays(GL_QUADS, 0, quad_array.size() * 4);
			}
			
			void group::default_update() {
				update_rectangles();
				call_updaters();
				update_rectangles();
				update_array();
			}

			void group::update_array() {
				quad_array.clear();
				rect::draw_info in(*this, quad_array);

				root.draw_children(in);
				
				if(middlescroll.subject) {
					rects::ltrb<float> scroller = rects::wh<float>(middlescroll.size);
					scroller.center(middlescroll.pos);
					rect::draw_clipped_rectangle(middlescroll.mat, scroller, &root, quad_array); 
				}
			}
			
			void group::update_rectangles() {
				root.update_rectangles();
			}

			void group::call_updaters() {
				root.update_proc(*this);

				if(middlescroll.subject) {
					vec2i tempp = middlescroll.subject->scroll; 
					middlescroll.subject->scroll += (owner.events.mouse.pos - middlescroll.pos) * float(middlescroll.speed_mult*fps.frame_speed());
				}
			}

			void paste_clipboard(text::fstr& out, text::formatted_char f) {
				std::wstring w;
				window::paste_clipboard(w);
				size_t len = w.length();
				out.clear();
				out.reserve(len);
				for(size_t i = 0; i < len; ++i) {
					f.c = w[i];
					out += f;
				}
			}

			void system::copy_clipboard(text::fstr& s) {
				clipboard = s;
				own_copy = true;
				own_clip = true;
				window::copy_clipboard(misc::wstr(s));
			}

			void system::change_clipboard() {
				if(!own_copy && fetch_clipboard) {
					text::formatted_char ch;
					ch.set(0, 0, rgba(0, 0, 0, 255));
					paste_clipboard(clipboard, ch);
					own_clip = false;
				}

				own_copy = false;
			}

			void group::poll_events() {
				augs::window::event::state& gl = owner.events;
				using namespace augs::window;
				rect::poll_info in(*this, gl.msg);
				bool pass = true;

				if(middlescroll.subject) {
					if (gl.msg == event::mdown || gl.msg == event::mdoubleclick) {
						pass = false;
						middlescroll.subject = 0;
					}
					return;
				}

				rect::event_info e(*this, rect::event::unknown);

				if(gl.msg == event::lup) {
					if(rect_held_by_lmb) {
						if(rect_held_by_lmb->get_clipped_rect().hover(gl.mouse.pos)) {
							rect_held_by_lmb->event_proc(e = rect::event::lup);
							rect_held_by_lmb->event_proc(e = rect::event::lclick);
							pass = false;
						}
						else
							rect_held_by_lmb->event_proc(e = rect::event::loutup);
						rect_held_by_lmb = 0;
					}
				} 
				
				if(gl.msg == event::rup) {
					if(rect_held_by_rmb) {
						if(rect_held_by_rmb->get_clipped_rect().hover(gl.mouse.pos)) {
							rect_held_by_rmb->event_proc(e = rect::event::rup);
							rect_held_by_rmb->event_proc(e = rect::event::rclick);
							pass = false;
						}
						else
							rect_held_by_rmb->event_proc(e = rect::event::routup);
						rect_held_by_rmb = 0;
					}
				} 

				if(rect_in_focus && rect_in_focus->fetch_wheel && gl.msg == event::wheel) {
					   if(rect_in_focus->enable_drawing) rect_in_focus->poll_message(in);
					   pass = false;
				}
/*
				if(gl.msg == down && gl.key == event::keys::TAB) {
					rect* f;
					if(f = seek_focusable(focus ? focus : &root, gl.keys[event::keys::LSHIFT])) 
						set_focus(f);

					pass = false;
				}*/
				if(rect_in_focus) {
					switch(gl.msg) {
					case event::keydown:   if(rect_in_focus->enable_drawing) rect_in_focus->event_proc(e = rect::event::keydown); pass = false; break;
					case event::keyup:	   if(rect_in_focus->enable_drawing) rect_in_focus->event_proc(e = rect::event::keyup); pass = false; break;
					case event::character: if(rect_in_focus->enable_drawing) rect_in_focus->event_proc(e = rect::event::character); pass = false; break;
					default: break; 
					}
				}
				
				if(gl.msg == event::clipboard_change) {
					owner.change_clipboard();
					pass = false;
				}

				if(pass) root.poll_message(in);
			}
				

		}
	}
}