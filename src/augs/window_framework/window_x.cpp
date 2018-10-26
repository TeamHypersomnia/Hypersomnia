#include <thread>

#include <unistd.h>
#include <fcntl.h>

#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>

#include <xcb/xcb.h>
#if TODO_FULLSCREEN
#include <xcb/xcb_ewmh.h>
#endif
#include <xcb/xcb_keysyms.h>
#include <xcb/xcbext.h>

#include <GL/glx.h>
#include <GL/gl.h>

#include "augs/templates/container_templates.h"
#include "augs/filesystem/file.h"

#include "augs/window_framework/print_x_devices.h"
#include "augs/window_framework/translate_x_enums.h"
#include "augs/window_framework/shell.h"
#include "augs/window_framework/window.h"

#include "augs/window_framework/detail_xinput.h"

int ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);

template <class T>
auto freed_unique(T* const ptr) {
	return std::unique_ptr<T, decltype(free)*>(ptr, free);
}

template <class T>
auto xfreed_unique(T* const ptr) {
	return std::unique_ptr<T, decltype(XFree)*>(ptr, XFree);
}

namespace augs {
	window::window(const window_settings& settings) {
		/* Open Xlib Display */ 
		display = XOpenDisplay(0);

		if (display == nullptr) {
			throw window_error("Could open X Display.");
		}

		/* XInput Extension available? */
		{
			int event;
			int	error;

			if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error)) {
				LOG("X Input extension not available.");
			}
			else {
				LOG("X Input extension available.");
			}
		}

		{
			/* Which version of XI2? We support 2.0 */
			int major = 2;
			int	minor = 0;

			if (XIQueryVersion(display, &major, &minor) == BadRequest) {
				LOG("XI2 not available. Server supports %x.%x\n", major, minor);
			}
			else {
				LOG("XI2 extension available.");
			}
		}

		print_input_devices(display);

		int default_screen = DefaultScreen(display);

		/* Get the XCB connection from the display */
		connection = XGetXCBConnection(display);

		if (!connection) {
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
			auto fb_config = [&]() {
				thread_local auto disp = display; disp = display;

				struct total_config {
					int	depth = -1;
					int visual_id = 0;

					int samp_buf = -1; 
					int samples = -1;
					int	depth_size = -1;
					int	stencil_size = -1;

					GLXFBConfig fb;

					total_config(const GLXFBConfig& fb) : fb(fb) {
						auto vi = xfreed_unique(glXGetVisualFromFBConfig(disp, fb));

						if (vi) {
							depth = vi->depth;
							visual_id = vi->visualid;
						}

						glXGetFBConfigAttrib(disp, fb, GLX_SAMPLE_BUFFERS, &samp_buf);
						glXGetFBConfigAttrib(disp, fb, GLX_SAMPLES       , &samples);
						glXGetFBConfigAttrib(disp, fb, GLX_DEPTH_SIZE    , &depth_size);
						glXGetFBConfigAttrib(disp, fb, GLX_STENCIL_SIZE    , &stencil_size);
					}

					auto make_tup() const {
						return std::make_tuple(samp_buf, samples, depth_size);
					}

					bool preferred_to(const total_config& b) const {
						return make_tup() < b.make_tup();
					}

					bool operator<(const total_config& b) const {
						return preferred_to(b);
					}

					void report(const int i) const {
						LOG("  Matching fbconfig %x, visual ID %h: SAMPLE_BUFFERS = %x, SAMPLES = %x, DEPTH_SIZE = %x, STENCIL_SIZE = %x\n", i, visual_id, samp_buf, samples, depth_size, stencil_size);
					}
				};

				auto fbc = [&]() {
					static int visual_attribs[] =
						{
							GLX_DEPTH_SIZE      , 0,
							GLX_X_RENDERABLE    , True,
							GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
							GLX_RENDER_TYPE     , GLX_RGBA_BIT,
							GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
							GLX_RED_SIZE        , 8,
							GLX_GREEN_SIZE      , 8,
							GLX_BLUE_SIZE       , 8,
							GLX_ALPHA_SIZE      , 8,
							GLX_STENCIL_SIZE    , BUILD_STENCIL_BUFFER ? 8 : 0,
							GLX_DOUBLEBUFFER    , True,
							GLX_SAMPLE_BUFFERS  , 0,
							GLX_SAMPLES         , 0,
							GLX_AUX_BUFFERS, 0,
							GLX_CONFIG_CAVEAT, GLX_NONE,
							GLX_TRANSPARENT_TYPE, GLX_NONE,
							GLX_ACCUM_RED_SIZE, 0,
							GLX_ACCUM_GREEN_SIZE, 0,
							GLX_ACCUM_BLUE_SIZE, 0,
							GLX_ACCUM_ALPHA_SIZE, 0,
							None
						}
					;

					int fbcount;

					auto fbc = xfreed_unique(glXChooseFBConfig(
						display, 
						default_screen, 
						visual_attribs, 
						&fbcount
					));

					if (!fbc) {
						throw window_error( "Failed to retrieve a framebuffer config\n" );
					}

					return std::vector<total_config>(fbc.get(), fbc.get() + fbcount);
				}();

				LOG("Found %x matching FB configs.\n", fbc.size());

				erase_if (fbc, [](const auto& f) {
					return f.depth != 24;
				});

				LOG("Found %x matching FB configs after filtering for depth.\n", fbc.size());

				sort_range(fbc);

				LOG("Sorted configs: ");

				for (const auto& f : fbc) {
					f.report(index_in(fbc, f));
				}

				LOG("Choosing the first fb config in list.");
				return fbc.front();
			}();

			/* Create OpenGL context */
			context = glXCreateNewContext(display, fb_config.fb, GLX_RGBA_TYPE, 0, True);

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
				fb_config.visual_id
			);

			/* Create window */
			uint32_t eventmask = 
				XCB_EVENT_MASK_EXPOSURE 
				| XCB_EVENT_MASK_KEY_PRESS 
				| XCB_EVENT_MASK_KEY_RELEASE
				| XCB_EVENT_MASK_POINTER_MOTION
				| XCB_EVENT_MASK_BUTTON_PRESS
				| XCB_EVENT_MASK_BUTTON_RELEASE
				| XCB_EVENT_MASK_FOCUS_CHANGE
				| XCB_INPUT_XI_EVENT_MASK_RAW_MOTION
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
				fb_config.visual_id,
				valuemask,
				valuelist
			);

			// NOTE: window must be mapped before glXMakeContextCurrent
			xcb_map_window(connection, window_id);

			{
				xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 1, 12,"WM_PROTOCOLS");
				auto reply = freed_unique(xcb_intern_atom_reply(connection, cookie, 0));
				xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
				auto reply2 = freed_unique(xcb_intern_atom_reply(connection, cookie2, 0));

				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window_id, (*reply).atom, 4, 32, 1, &reply2->atom);

				xcb_flush(connection);

				wm_delete_window_atom = { reply2->atom };
			}

			glxwindow = glXCreateWindow(display, fb_config.fb, window_id, 0);

			if (!window_id) {
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

		last_mouse_pos = get_screen_size() / 2;

		{
			auto dpy = display;

			XIEventMask mask;

			mask.deviceid = XIAllMasterDevices;
			mask.mask_len = XIMaskLen(XI_RawMotion);
			mask.mask = reinterpret_cast<unsigned char*>(std::calloc(mask.mask_len, sizeof(unsigned char)));

			mask.deviceid = XIAllMasterDevices;
			memset(mask.mask, 0, mask.mask_len);
			XISetMask(mask.mask, XI_RawMotion);

			XISelectEvents(dpy, DefaultRootWindow(dpy), &mask, 1);

			free(mask.mask);
			XSync(dpy, True);
		}
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
		uint32_t new_width = flag ? 1 : 0;

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
	template <class F, class G, class H>
	std::optional<event::change> handle_event(
		const xcb_generic_event_t* event,
		xcb_timestamp_t& last_ldown_time_ms,
		F mousemotion_handler,
		G keysym_getter,
		H character_event_emitter,
		decltype(xcb_intern_atom_reply_t::atom) wm_delete_window_atom,
		const bool log_keystrokes
	) {
		using namespace event;
		using namespace keys;

		change ch;

		switch (event->response_type & ~0x80) {
			default:
#if 0
			{
				auto i1 = static_cast<int>(event->response_type);
				auto i2 = static_cast<int>(event->response_type & ~0x80);
				LOG("Unknown: %x %x", i1, i2);
			}
#endif
			return std::nullopt;

			case XCB_INPUT_RAW_MOTION:
			return ch;
			case XCB_CLIENT_MESSAGE:
				{
					if (wm_delete_window_atom == reinterpret_cast<const xcb_client_message_event_t*>(event)->data.data32[0]) {
						LOG("WM_DELETE_WINDOW request received");
						ch.msg = message::close;

						return ch;
					}	

					return std::nullopt;
				}
			case XCB_FOCUS_OUT:
			   ch.msg = message::deactivate;
		   	   return ch;

			case XCB_FOCUS_IN:
			   ch.msg = message::activate;
		   	   return ch;

            case XCB_KEY_PRESS: {
				const auto* const press = reinterpret_cast<const xcb_key_press_event_t*>(event);
			
				const auto keycode = press->detail;	
				const auto keysym = keysym_getter(keycode);

				ch.msg = message::keydown;
				ch.data.key.key = translate_keysym({ keysym });
				ch.timestamp = press->time;

				if (log_keystrokes) {
					LOG("Keycode down: %x", static_cast<int>(keycode));
					LOG("Keysym down: %x (%h)", static_cast<int>(keysym), static_cast<int>(keysym));
					LOG("Output enum: %x", int(ch.data.key.key));
				}

				switch (ch.data.key.key) {
					case key::ESC: break;
					case key::DEL: break;
					case key::BACKSPACE: break;
					default: character_event_emitter(press); break;
				}

				return ch;
            }
            case XCB_KEY_RELEASE: {
				const auto* const release = reinterpret_cast<const xcb_key_release_event_t*>(event);
			
				const auto keycode = release->detail;	
				const auto keysym = keysym_getter(keycode);

				ch.msg = message::keyup;
				ch.data.key.key = translate_keysym({ keysym });

				if (log_keystrokes) {
					LOG("Keycode up: %x", static_cast<int>(keycode));
					LOG("Keysym up: %x", static_cast<int>(keysym));
					LOG("Output enum: %x", int(ch.data.key.key));
				}

				ch.timestamp = release->time;

				return ch;
            }
			case XCB_MOTION_NOTIFY: {
           		const auto* const motion = reinterpret_cast<const xcb_motion_notify_event_t*>(event);
				return mousemotion_handler({motion->event_x, motion->event_y});
			} 
			case XCB_BUTTON_PRESS: {
           		const auto* const press = reinterpret_cast<const xcb_button_press_event_t*>(event);
				ch.timestamp = press->time;

				// LOG("DOWN.Det: %x; time: %x; st: %x", static_cast<int>(press->detail), press->time, static_cast<int>(press->state));
		
				// TODO: support doubleclicks for left and middle
				
				switch (press->detail) {
					case 1:
					   	if (press->time - last_ldown_time_ms <= 500) {
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
						
					case 4:
						ch.msg = message::wheel;
						ch.data.scroll.amount = 1;
						return ch;
						
					case 5:
						ch.msg = message::wheel;
						ch.data.scroll.amount = -1;
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
				ch.timestamp = release->time;

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
		}
	}

	void window::collect_entropy(local_entropy& output) {
		auto keysym_getter = [this](const xcb_keycode_t keycode){
			return xcb_key_symbols_get_keysym(syms, keycode, 0);
		};

		auto character_event_emitter = [&](const xcb_key_press_event_t* const press) {
			using namespace event;
			
			change ch;
			ch.msg = message::character;
			ch.timestamp = press->time;

			XKeyEvent keyev;
			keyev.display = display;
			keyev.keycode = press->detail;
			keyev.state = press->state;
			
			std::array<char, 16> buf {};

			if (XLookupString(&keyev, buf.data(), buf.size(), nullptr, nullptr)) {
				if (get_current_settings().log_keystrokes) {
					LOG("Ch press: %x, len: %x", buf.data(), strlen(buf.data()));
				}
				
				unsigned int c = 0;
				ImTextCharFromUtf8(&c, buf.begin(), buf.end());
				ch.data.character.code_point = c;

				output.push_back(ch);
			}
		};
		
		while (const auto event = freed_unique(xcb_poll_for_event(connection))) {
			{
				xcb_ge_generic_event_t *generic_event = (xcb_ge_generic_event_t*)event.get();

				if (generic_event->response_type == XCB_GE_GENERIC 
					&& generic_event->extension == xi_opcode 
					&& generic_event->event_type == XI_RawMotion
				) {
					const auto mot = reinterpret_cast<xcb_input_raw_button_press_event_t*>(generic_event);
					const auto axis_n = xcb_input_raw_button_press_axisvalues_raw_length(mot);

					if (2 == axis_n) {
						const auto axes = xcb_input_raw_button_press_axisvalues(mot);

						const auto x = axes[0].integral;
						const auto y = axes[1].integral;

						if (is_active() && (current_settings.raw_mouse_input || mouse_pos_paused)) {
							auto ch = do_raw_motion({
								static_cast<short>(x),
								static_cast<short>(y) 
							});

							output.push_back(ch);
						}
					}
					else {
						/* LOG("WARNING! axis_n = %x (should be 2)", axis_n); */
					}

					continue;
				}
			}

			auto mousemotion_handler = [this](const basic_vec2<short> p) {
				return handle_mousemove(p); 
			};

			if (const auto ch = handle_event(
				event.get(), 
				last_ldown_time_ms,
				mousemotion_handler,
				keysym_getter,
				character_event_emitter,
				wm_delete_window_atom,
				get_current_settings().log_keystrokes
			)) {
				common_event_handler(*ch, output);
				output.push_back(*ch);
			}
		}

		local_entropy clean_entropy;

		std::size_t i = 0;

		while (i < output.size()) {
			const auto& ch = output[i];

			using k = event::key_change;

			const auto ch_change = ch.get_key_change();

			const bool add_this = [&]() {
				if (ch.msg == event::message::character) {
					return true;
				}

				if (ch_change != k::NO_CHANGE) {
					for (std::size_t n = i + 1; n < output.size(); ++n) {
						const auto& nx = output[n];
						const auto nx_change = nx.get_key_change();

						if (nx_change != k::NO_CHANGE) {
							if (nx.timestamp == ch.timestamp) {
								if (nx.data.key.key == ch.data.key.key && nx_change != ch_change) {
									//LOG("Erase %x and %x", i, n);
								   output.erase(output.begin() + n);
								   return false;
							   }
						   }
						}
					}
				}

				return true;
			}();

			if (add_this) {
				clean_entropy.emplace_back(ch);
			}

			++i;
		}

		output = std::move(clean_entropy);
	}


	void window::set_window_rect(const xywhi r) {
		if (!can_control_window_geometry) {
			return;
		}

		uint32_t values[4] = {
			static_cast<uint32_t>(r.x),
			static_cast<uint32_t>(r.y),
			static_cast<uint32_t>(r.w),
			static_cast<uint32_t>(r.h)
		};

		xcb_configure_window(
			connection,
			window_id,
			XCB_CONFIG_WINDOW_X 
			| XCB_CONFIG_WINDOW_Y
			| XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT
			,	
			values
		);

		if (get_window_rect() != r) {
			can_control_window_geometry = false;
		}
	}

	void window::set_fullscreen_hint(const bool) {
// TODO: actually implement fullscreen
#if 0
xcb_ewmh_connection_t EWMH;
xcb_intern_atom_cookie_t *EWMHCookie = xcb_ewmh_init_atoms(connection, &EWMH);
xcb_ewmh_init_atoms_replies(&EWMH, EWMHCookie, NULL);
#endif
	}

	xywhi window::get_window_rect() const { 
		xcb_get_geometry_cookie_t  geomCookie = xcb_get_geometry (connection, window_id);
		
		const auto geom = freed_unique(
			xcb_get_geometry_reply (connection, geomCookie, NULL)
		);

		return { geom->x, geom->y, geom->width, geom->height }; 
	}

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

	void window::set_cursor_pos(vec2i pos) {
    	XWarpPointer(display, None, window_id, 0, 0, 0, 0, pos.x, pos.y);
		XSync(display, False);
	}

	void window::clip_system_cursor() {
		XGrabPointer(
			display,
		   	window_id,
		   	True,
		   	0,
            GrabModeAsync, 
			GrabModeAsync,
            window_id,
		   	None,
		   	CurrentTime
		);
	}

	void window::disable_cursor_clipping() {
		XUngrabPointer(display, CurrentTime);
	}

	void window::set_cursor_visible(const bool flag) {
		if (!flag) {
			static const auto sharedInvisibleCursor = [this](){
				// Thanks to:
				// https://stackoverflow.com/a/664528/503776

				Cursor invisibleCursor;
				Pixmap bitmapNoData;
				XColor black;
				static char noData[] = { 0,0,0,0,0,0,0,0 };
				black.red = black.green = black.blue = 0;

				bitmapNoData = XCreateBitmapFromData(display, window_id, noData, 8, 8);

				invisibleCursor = XCreatePixmapCursor(
					display,
				   	bitmapNoData, 
					bitmapNoData, 
					&black, 
					&black, 
					0, 
					0
				);
				
				XFreePixmap(display, bitmapNoData);
				return invisibleCursor;
			}();

			XDefineCursor(display,window_id, sharedInvisibleCursor);
		}
		else {
			XUndefineCursor(display,window_id);
		}
	}

	static std::optional<std::string> read_chosen_path(const augs::path_type& script_path) {
		const auto& temp_result = GENERATED_FILES_DIR "/last_file_path.txt";

		try {
			const auto result = file_to_string(temp_result);
			remove_file(temp_result);
			return result;
		}
		catch (const augs::file_open_error&) {
			LOG("Error: %x did not produce %x", script_path, temp_result);
			return std::nullopt;
		}
	}

	static std::optional<std::string> choose_path(const augs::path_type& script_path) {
		if (!augs::exists(script_path)) {
			LOG("WARNING! Could not find the script file: %x.", script_path);
			return std::nullopt;
		}

		augs::shell(script_path);
		return read_chosen_path(script_path);
	}

	std::optional<std::string> window::open_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& /* custom_title */
	) const {
		return choose_path("scripts/unix/open_file.local");
	}

	std::optional<std::string> window::save_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& /* custom_title */
	) const {
		return choose_path("scripts/unix/save_file.local");
	}

	std::optional<std::string> window::choose_directory_dialog(
		const std::string& /* custom_title */
	) const {
		return choose_path("scripts/unix/choose_directory.local");
	}

	void window::reveal_in_explorer(const augs::path_type& p) const {
		const auto script_path = "scripts/unix/reveal_file.local";

		if (!augs::exists(script_path)) {
			LOG("WARNING! Could not find the script file: %x.", script_path);
			return;
		}

		const auto shell_command = typesafe_sprintf("%x %x", script_path, p.string());

		std::thread([shell_command](){ augs::shell(shell_command); }).detach();
	}

	window::~window() {
		destroy();
	}
}
