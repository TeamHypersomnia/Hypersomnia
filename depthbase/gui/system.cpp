#pragma once
#include "system.h"
#include "rect.h"
#include "window_framework/window.h"
#include <GL/GL.h>
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
		using namespace augs::texture_baker;
		namespace gui {
			augs::texture_baker::texture* null_texture = 0;

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
				void formatted_char::set(wchar_t ch, font* f, const pixel_32& p) {
					font_used = f;
					c = ch;
					memcpy(&r, &p, sizeof(pixel_32));
				}

				void formatted_char::set(font* f, const pixel_32& p) {
					font_used = f;
					memcpy(&r, &p, sizeof(pixel_32));
				}

				style::style(font* f, pixel_32 c) : f(f), color(c) {}

				style::style(const formatted_char& c) : f(c.font_used), color(pixel_32(c.r, c.g, c.b, c.a)) {}

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
				
			group::group(system& owner) : owner(owner), focus(&root), lholded(nullptr), rholded(nullptr) {
			/* default middlescroll setup */
				middlescroll.subject = 0;
				middlescroll.speed_mult = 1.f;
				middlescroll.size = rects::wh<float>(25, 25);
				root.clip = false;
				root.focusable = false;
				root.scrollable = false;
			}
			
			void group::set_focus(rect* f) {
				if(f == focus) return;
				update_rectangles();

				if(focus) 
					focus->event_proc(rect::event_info(*this, rect::event::blur));
				
				focus = f;
				if(f) {
					f->event_proc(rect::event_info(*this, rect::event::focus));
					//f->scroll_to_view();
				}
			}
			
			rect* group::get_focus() const {
				return focus;
			}

			void group::draw_gl_fixed() {
				glVertexPointer(2, GL_FLOAT, sizeof(resources::vertex_triangle), quad_array.data());
				glTexCoordPointer(2, GL_FLOAT, sizeof(resources::vertex_triangle), (char*) (quad_array.data()) + sizeof(int) * 2);
				glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(resources::vertex_triangle), (char*) (quad_array.data()) + sizeof(int) * 2 + sizeof(float) * 2);
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
					rect::add_quad(middlescroll.mat, scroller, &root, quad_array); 
				}
			}
			
			void group::update_rectangles() {
				root.update_rectangles();
			}

			void group::call_updaters() {
				root.update_proc(*this);

				if(middlescroll.subject) {
					vec2<int> tempp = middlescroll.subject->scroll; 
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
					ch.set(0, 0, pixel_32(0, 0, 0, 255));
					paste_clipboard(clipboard, ch);
					own_clip = false;
				}

				own_copy = false;
			}

			void group::poll_events() {
				augs::window::event::state& gl = owner.events;
				using namespace augs::window::event::key;
				using namespace augs::window;
				rect::poll_info in(*this, gl.msg);
				bool pass = true;

				if(middlescroll.subject) {
					if (gl.msg == event::mouse::mdown || gl.msg == event::mouse::mdoubleclick) {
						pass = false;
						middlescroll.subject = 0;
					}
					return;
				}

				rect::event_info e(*this, rect::event::unknown);

				if(gl.msg == event::mouse::lup) {
					if(lholded) {
						if(lholded->get_clipped_rect().hover(gl.mouse.pos)) {
							lholded->event_proc(e = rect::event::lup);
							lholded->event_proc(e = rect::event::lclick);
							pass = false;
						}
						else
							lholded->event_proc(e = rect::event::loutup);
						lholded = 0;
					}
				} 
				
				if(gl.msg == event::mouse::rup) {
					if(rholded) {
						if(rholded->get_clipped_rect().hover(gl.mouse.pos)) {
							rholded->event_proc(e = rect::event::rup);
							rholded->event_proc(e = rect::event::rclick);
							pass = false;
						}
						else
							rholded->event_proc(e = rect::event::routup);
						rholded = 0;
					}
				} 

				if(focus && focus->fetch_wheel && gl.msg == event::mouse::wheel) {
					   if(focus->draw) focus->poll_message(in);
					   pass = false;
				}
/*
				if(gl.msg == down && gl.key == event::keys::TAB) {
					rect* f;
					if(f = seek_focusable(focus ? focus : &root, gl.keys[event::keys::LSHIFT])) 
						set_focus(f);

					pass = false;
				}*/
				if(focus) {
					switch(gl.msg) {
					case down:		if(focus->draw) focus->event_proc(e = rect::event::keydown); pass = false; break;
					case up:		if(focus->draw) focus->event_proc(e = rect::event::keyup); pass = false; break;
					case character: if(focus->draw) focus->event_proc(e = rect::event::character); pass = false; break;
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