#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include <GL/glx.h>
#include <GL/gl.h>

#include "augs/window_framework/translate_x_enums.h"
#include "augs/window_framework/window.h"

template <class T>
auto freed_unique(T* const ptr) {
	return std::unique_ptr<T, decltype(free)*>(ptr, free);
}


namespace augs {
	window::window(const window_settings& settings) {
		last_mouse_pos = settings.get_screen_size() / 2;

		int default_screen = 0xdeadbeef;

		/* Open Xlib Display */ 
		display = XOpenDisplay(0);

		default_screen = DefaultScreen(display);

		/* Get the XCB connection from the display */
		connection = XGetXCBConnection(display);

		if(!connection) {
			XCloseDisplay(display);
			throw window_error("Can't get xcb connection from display");
		}

		syms = xcb_key_symbols_alloc(connection);

		/* Acquire event queue ownership */
		XSetEventQueueOwner(display, XCBOwnsEventQueue);

		/* Find XCB screen */
		xcb_screen_t *screen = 0;
		xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(connection));
		for(int screen_num = default_screen;
						screen_iter.rem && screen_num > 0;
						--screen_num, xcb_screen_next(&screen_iter));
		screen = screen_iter.data;

		auto setup_and_run = [this, settings](int default_screen, xcb_screen_t *screen) {
			int visualID = 0;

			/* Query framebuffer configurations */
			GLXFBConfig *fb_configs = 0;
			int num_fb_configs = 0;
			fb_configs = glXGetFBConfigs(display, default_screen, &num_fb_configs);

			if (!fb_configs || num_fb_configs == 0) {
				throw window_error("glXGetFBConfigs failed");
			}

			/* Select first framebuffer config and query visualID */
			GLXFBConfig fb_config = fb_configs[0];
			glXGetFBConfigAttrib(display, fb_config, GLX_VISUAL_ID , &visualID);

			/* Create OpenGL context */
			context = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, True);

			if (!context) {
				throw window_error("glXCreateNewContext failed");
			}

			/* Create XID's for colormap and window */
			xcb_colormap_t colormap = xcb_generate_id(connection);
			window_id = xcb_generate_id(connection);

			/* Create colormap */
			xcb_create_colormap(
				connection,
				XCB_COLORMAP_ALLOC_NONE,
				colormap,
				screen->root,
				visualID
			);

			/* Create window */
			uint32_t eventmask = 
				XCB_EVENT_MASK_EXPOSURE 
				| XCB_EVENT_MASK_KEY_PRESS 
				| XCB_EVENT_MASK_KEY_RELEASE
				| XCB_EVENT_MASK_POINTER_MOTION
				| XCB_EVENT_MASK_BUTTON_PRESS
				| XCB_EVENT_MASK_BUTTON_RELEASE
			;

			uint32_t valuelist[] = { eventmask, colormap, 0 };
			uint32_t valuemask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;

			xcb_create_window(
				connection,
				XCB_COPY_FROM_PARENT,
				window_id,
				screen->root,
				settings.position.x, settings.position.y,
				settings.size.x, settings.size.y,
				settings.border ? 1 : 0,
				XCB_WINDOW_CLASS_INPUT_OUTPUT,
				visualID,
				valuemask,
				valuelist
			);


			// NOTE: window must be mapped before glXMakeContextCurrent
			xcb_map_window(connection, window_id); 

			glxwindow = glXCreateWindow(display, fb_config, window_id, 0);

			if(!window_id) {
				xcb_destroy_window(connection, window_id);
				glXDestroyContext(display, context);

				throw window_error("glXCreateWindow failed");
			}

			drawable = glxwindow;

			/* make OpenGL context current */
			if (!set_as_current()) {
				xcb_destroy_window(connection, window_id);
				glXDestroyContext(display, context);

				throw window_error("glXMakeContextCurrent failed");
			}
		};

		setup_and_run(default_screen, screen);
		apply(settings, true);
	}

	void window::destroy() {
		if (display) {
			unset_if_current();

			xcb_key_symbols_free(syms);	

			glXDestroyWindow(display, glxwindow);

			xcb_destroy_window(connection, window_id);

			glXDestroyContext(display, context);

			XCloseDisplay(display);

			context = 0;
			glxwindow = 0;
			drawable = 0;
			display = nullptr;
			connection = nullptr;
		}	
	}

	void window::set_window_name(const std::string& name) {
		xcb_change_property (connection,
			XCB_PROP_MODE_REPLACE,
			window_id,
			XCB_ATOM_WM_NAME,
			XCB_ATOM_STRING,
			8,
			name.length(),
			name.c_str()
		);
	}

	void window::set_window_border_enabled(const bool flag) {
		uint32_t new_width = flag ? 0 : 1;

		xcb_configure_window(
			connection,
			window_id,
			XCB_CONFIG_WINDOW_BORDER_WIDTH,
			&new_width
		);
	}

	bool window::swap_buffers() { 
		glXSwapBuffers(display, drawable);
		return true;
	}

	void window::show() {}

	template <class F, class G>
	std::optional<event::change> handle_event(
		const xcb_generic_event_t* event,
		xcb_timestamp_t& last_ldown_time_ms,
		F mousemotion_handler,
		G keysym_getter
	) {
		using namespace event;
		using namespace keys;

		// TODO: handle WM_DELETE_WINDOW
		// xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(c, 0, 16, "WM_DELETE_WINDOW");
		// xcb_intern_atom_reply_t* reply2 = xcb_intern_atom_reply(c, cookie2, 0);
	
		change ch;

		switch (event->response_type & ~0x80) {
            case XCB_KEY_PRESS: {
				const auto* const press = reinterpret_cast<const xcb_key_press_event_t*>(event);
			
				const auto keycode = press->detail;	
				const auto keysym = keysym_getter(keycode);
#if 0
				LOG("Keycode down: %x", static_cast<int>(keycode));
				LOG("Keysym down: %x", static_cast<int>(keysym));
#endif			
				ch.msg = message::keydown;
				ch.data.key.key = translate_keysym({ keysym });

				return ch;
            }
            case XCB_KEY_RELEASE: {
				const auto* const release = reinterpret_cast<const xcb_key_release_event_t*>(event);
			
				const auto keycode = release->detail;	
				const auto keysym = keysym_getter(keycode);
#if 0
				LOG("Keycode up: %x", static_cast<int>(keycode));
				LOG("Keysym up: %x", static_cast<int>(keysym));
#endif			
				ch.msg = message::keyup;
				ch.data.key.key = translate_keysym({ keysym });

				return ch;
            }
			case XCB_MOTION_NOTIFY: {
           		const auto* const motion = reinterpret_cast<const xcb_motion_notify_event_t*>(event);
				return mousemotion_handler({motion->event_x, motion->event_y});
			} 
			case XCB_BUTTON_PRESS: {
           		const auto* const press = reinterpret_cast<const xcb_button_press_event_t*>(event);

				// LOG("DOWN.Det: %x; time: %x; st: %x", static_cast<int>(press->detail), press->time, static_cast<int>(press->state));
		
				// TODO: support doubleclicks for left and middle
				
				switch (press->detail) {
					case 1:
					   	if(press->time - last_ldown_time_ms <= 500) {
							ch.msg = message::ldoubleclick;
						}
						else {
							ch.msg = message::keydown;
							ch.data.key.key = key::LMOUSE;
						}

						last_ldown_time_ms = press->time;

						return ch;

					case 3:
						ch.msg = message::keydown;
						ch.data.key.key = key::RMOUSE;
						return ch;

					case 2:
						ch.msg = message::keydown;
						ch.data.key.key = key::MMOUSE;
						return ch;
						
					case 8:
						ch.msg = message::keydown;
						ch.data.key.key = key::MOUSE4;
						return ch;

					case 9:
						ch.msg = message::keydown;
						ch.data.key.key = key::MOUSE5;
						return ch;

					default: return std::nullopt;
				}
			}

			case XCB_BUTTON_RELEASE: {
           		const auto* const release = reinterpret_cast<const xcb_button_release_event_t*>(event);

				// LOG("UP.Det: %x; time: %x; st: %x", static_cast<int>(release->detail), release->time, static_cast<int>(release->state));
			
				switch (release->detail) {
					case 1:
						ch.msg = message::keyup;
						ch.data.key.key = key::LMOUSE;
						return ch;

					case 3:
						ch.msg = message::keyup;
						ch.data.key.key = key::RMOUSE;
						return ch;

					case 2:
						ch.msg = message::keyup;
						ch.data.key.key = key::MMOUSE;
						return ch;
						
					case 8:
						ch.msg = message::keyup;
						ch.data.key.key = key::MOUSE4;
						return ch;

					case 9:
						ch.msg = message::keyup;
						ch.data.key.key = key::MOUSE5;
						return ch;

					default: return std::nullopt;
				}
			}
/*
		    case XCB_CLIENT_MESSAGE: {
										 if((*(xcb_client_message_event_t*)event).data.data32[0] == (*reply2).atom) {


										 }
									 }

*/
			default:
				return std::nullopt;
		}
	}

	void window::collect_entropy(local_entropy& into) {
		auto keysym_getter = [this](const xcb_keycode_t keycode){
			return xcb_key_symbols_get_keysym(syms, keycode, 0);
		};

		while (const auto event = freed_unique(xcb_poll_for_event(connection))) {
			if (const auto ch = handle_event(
				event.get(), 
				last_ldown_time_ms,
				[this](const basic_vec2<short> p) { return handle_mousemove(p); },
				keysym_getter
			)) {
				into.push_back(*ch);
			}
		}
	}


	void window::set_window_rect(const xywhi) {}

	xywhi window::get_window_rect() const { 
		xcb_get_geometry_cookie_t  geomCookie = xcb_get_geometry (connection, window_id);
		
		const auto geom = freed_unique(
			xcb_get_geometry_reply (connection, geomCookie, NULL)
		);

		return { geom->x, geom->y, geom->width, geom->height }; 
	}

	bool window::is_active() const { return false; }


	bool window::set_as_current_impl() {
#if BUILD_OPENGL
		return glXMakeContextCurrent(display, drawable, drawable, context);
#else
		return true;
#endif
	}

	void window::set_current_to_none_impl() {
#if BUILD_OPENGL
	//	 For now we only will have one window anyway
	//	 glXMakeContextCurrent(display, None, None, nullptr);
#endif
	}

	std::optional<std::string> window::open_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		std::string custom_title
	) const {
		return std::nullopt;
	}

	std::optional<std::string> window::save_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		std::string custom_title
	) const {
		return std::nullopt;
	}
}
