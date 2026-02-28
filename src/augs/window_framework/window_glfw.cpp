#include <GLFW/glfw3.h>

#if PLATFORM_LINUX
#include <dlfcn.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <X11/Xlib.h>
#endif

#include "augs/window_framework/event.h"
#include "augs/window_framework/window.h"
#include "augs/log.h"
#include "augs/filesystem/file.h"

#if PLATFORM_LINUX
#include <mutex>

std::mutex clipboard_mutex;
std::optional<std::string> set_clipboard_op;
std::string clipboard_snapshot;
#endif

augs::event::keys::key translate_glfw_key(int);
augs::event::keys::key translate_glfw_mouse_key(int);

struct unhandled_key {
	int key, scancode, action, mods;
};

struct unhandled_char {
	unsigned int codepoint;
};

struct unhandled_mouse_button {
	int button, action, mods;
};

struct unhandled_scroll {
	double xoffset, yoffset;
};

struct unhandled_cursor {
	double xpos, ypos;
};

struct unhandled_window_position {
	int xpos, ypos;
};

struct unhandled_window_size {
	int xsize, ysize;
};

struct unhandled_focus {
	int focused;
};


namespace augs {
	struct window::platform_data {
		GLFWwindow* window = nullptr;
		vec2d last_mouse_pos_for_dt;
		bool mouse_pos_initialized = false;
		int clips_called = 0;
		float content_scale_x = 1.0f;
		float content_scale_y = 1.0f;

		std::vector<unhandled_key> unhandled_keys;
		std::vector<unhandled_char> unhandled_characters;
		std::vector<unhandled_mouse_button> unhandled_mouse_buttons;
		std::vector<unhandled_scroll> unhandled_scrolls;
		std::vector<unhandled_cursor> unhandled_cursors;

		std::vector<unhandled_window_position> unhandled_window_positions;
		std::vector<unhandled_window_size> unhandled_window_sizes;
		std::vector<unhandled_focus> unhandled_focuses;

#if PLATFORM_LINUX
		bool has_x11 = false;

		int (*XGrabPointer_ptr)(Display*, Window, Bool, unsigned int, int, int, Window, Cursor, Time) = nullptr;
		int (*XUngrabPointer_ptr)(Display*, Time) = nullptr;
		int (*XSync_ptr)(Display*, Bool) = nullptr;
		int (*XUndefineCursor_ptr)(Display*, Window) = nullptr;
		int (*XDefineCursor_ptr)(Display*, Window, Cursor) = nullptr;
		Pixmap (*XCreateBitmapFromData_ptr)(Display*, Drawable, const char*, unsigned int, unsigned int) = nullptr;
		Cursor (*XCreatePixmapCursor_ptr)(Display*, Pixmap, Pixmap, XColor*, XColor*, unsigned int, unsigned int) = nullptr;
		void (*XFreePixmap_ptr)(Display*, Pixmap) = nullptr;

		bool init_x11_functions() {
			void* handle = nullptr;

#if defined(__CYGWIN__)
			handle = dlopen("libX11-6.so", RTLD_LAZY | RTLD_LOCAL);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
			handle = dlopen("libX11.so", RTLD_LAZY | RTLD_LOCAL);
#else
			handle = dlopen("libX11.so.6", RTLD_LAZY | RTLD_LOCAL);
#endif

			if (!handle) {
				LOG("X11 library not found.");
				return false;
			}

			XGrabPointer_ptr = (decltype(XGrabPointer_ptr))dlsym(handle, "XGrabPointer");
			XUngrabPointer_ptr = (decltype(XUngrabPointer_ptr))dlsym(handle, "XUngrabPointer");
			XSync_ptr = (decltype(XSync_ptr))dlsym(handle, "XSync");
			XUndefineCursor_ptr = (decltype(XUndefineCursor_ptr))dlsym(handle, "XUndefineCursor");
			XDefineCursor_ptr = (decltype(XDefineCursor_ptr))dlsym(handle, "XDefineCursor");
			XCreateBitmapFromData_ptr = (decltype(XCreateBitmapFromData_ptr))dlsym(handle, "XCreateBitmapFromData");
			XCreatePixmapCursor_ptr = (decltype(XCreatePixmapCursor_ptr))dlsym(handle, "XCreatePixmapCursor");
			XFreePixmap_ptr = (decltype(XFreePixmap_ptr))dlsym(handle, "XFreePixmap");

			// Check that all are loaded
			if (!XGrabPointer_ptr || !XUngrabPointer_ptr || !XSync_ptr ||
				!XUndefineCursor_ptr || !XDefineCursor_ptr ||
				!XCreateBitmapFromData_ptr || !XCreatePixmapCursor_ptr || !XFreePixmap_ptr) {
				LOG("Failed to load one or more X11 functions");
				dlclose(handle);
				return false;
			}

			LOG("X11 library found.");

			return true;
		}
#endif

	};

	GLFWwindow* get_glfw_window(const window::platform_data& d) {
		return d.window;
	}

	GLFWwindow* get_glfw_window(const window& d) {
		return get_glfw_window(*d.platform);
	}
}

struct glfw_callbacks {
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_keys.push_back({ key, scancode, action, mods });
	}

	static void character_callback(GLFWwindow* window, unsigned codepoint) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_characters.push_back({ codepoint });
	}

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_mouse_buttons.push_back({ button, action, mods });
	}

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_scrolls.push_back({ xoffset, yoffset });
	}

	static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_cursors.push_back({ xpos, ypos });
	}

	static void window_pos_callback(GLFWwindow* window, int xpos, int ypos) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_window_positions.push_back({ xpos, ypos });
	}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_window_sizes.push_back({ width, height });
	}

	static void focus_callback(GLFWwindow* window, int focused) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_focuses.push_back({ focused });
	}

	static void content_scale_callback(GLFWwindow* window, float xscale, float yscale) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->content_scale_x = xscale;
		wnd->platform->content_scale_y = yscale;
		wnd->platform->unhandled_window_sizes.push_back({ 0, 0 });
	}
};

static void error_callback(int error, const char* description) {
	LOG("GLFW Error %x: %x\n", error, description);
}

static GLFWmonitor* get_current_monitor(GLFWwindow* window) {
	int wx, wy, ww, wh;
	glfwGetWindowPos(window, &wx, &wy);
	glfwGetWindowSize(window, &ww, &wh);

	const int cx = wx + ww / 2;
	const int cy = wy + wh / 2;

	int count = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&count);

	for (int i = 0; i < count; i++) {
		int mx, my, mw, mh;
		glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mw, &mh);

		if (cx >= mx && cx < mx + mw && cy >= my && cy < my + mh) {
			return monitors[i];
		}
	}

	return glfwGetPrimaryMonitor();
}

namespace augs {
	window::window(const window_settings& settings) 
		: platform(std::make_unique<window::platform_data>())
	{
		LOG("GLFW: calling glfwInit.");

		glfwSetErrorCallback(error_callback);

		const auto previous_path = std::filesystem::current_path();

		if (!glfwInit()) {
			std::filesystem::current_path(previous_path);
			throw window_error("glfwInit failed.");
		}

		LOG("Calling init_x11_functions.");

#if PLATFORM_LINUX
		platform->has_x11 = platform->init_x11_functions();
#endif
		
		std::filesystem::current_path(previous_path);

		auto& window = platform->window;

		LOG("GLFW: setting version hints via glfwWindowHint.");

		glfwWindowHint(GLFW_SAMPLES, 0);
		glfwWindowHint(GLFW_DEPTH_BITS, 0);
		glfwWindowHint(GLFW_STENCIL_BITS, 8);
		glfwWindowHint(GLFW_DECORATED, settings.border);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		LOG("GLFW: calling glfwCreateWindow.");
		LOG("Note: OpenGL 3.2 or newer is required to run Hypersomnia!");

		if (settings.fullscreen) {
			GLFWmonitor* primary = glfwGetPrimaryMonitor();

			if (primary) {
				const GLFWvidmode* mode = glfwGetVideoMode(primary);
				window = glfwCreateWindow(mode->width, mode->height, settings.name.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
			}
		}
		else {
			window = glfwCreateWindow(settings.size.x, settings.size.y, settings.name.c_str(), NULL, NULL);
			glfwSetWindowPos(window, settings.position.x, settings.position.y);
		}

		if (window == nullptr) {
			destroy();
			throw window_error("glfwCreateWindow failed.");
		}

		glfwSetErrorCallback(error_callback);
		glfwSetWindowUserPointer(window, this);

		glfwSetKeyCallback(window, glfw_callbacks::key_callback);
		glfwSetCharCallback(window, glfw_callbacks::character_callback);
		glfwSetMouseButtonCallback(window, glfw_callbacks::mouse_button_callback);
		glfwSetScrollCallback(window, glfw_callbacks::scroll_callback);

		glfwSetWindowPosCallback(window, glfw_callbacks::window_pos_callback);
		glfwSetFramebufferSizeCallback(window, glfw_callbacks::framebuffer_size_callback);

		glfwSetWindowFocusCallback(window, glfw_callbacks::focus_callback);

		glfwGetWindowContentScale(window, &platform->content_scale_x, &platform->content_scale_y);
		glfwSetWindowContentScaleCallback(window, glfw_callbacks::content_scale_callback);

		if (glfwRawMouseMotionSupported()) {
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		glfwFocusWindow(window);
		active = glfwGetWindowAttrib(window, GLFW_FOCUSED);

		set_as_current();
		set(settings.vsync_mode);

		current_settings = settings;
		last_windowed_rect = current_settings.make_window_rect();

		current_rect = get_window_rect_impl();

		if (settings.draws_own_cursor()) {
			last_mouse_pos = current_rect.get_size() / 2;
			platform->last_mouse_pos_for_dt = last_mouse_pos;
			set_cursor_pos(last_mouse_pos);
		}
		else {
			double mx, my;
			glfwGetCursorPos(window, &mx, &my);

			const auto sx = static_cast<double>(platform->content_scale_x);
			const auto sy = static_cast<double>(platform->content_scale_y);
			const auto scaled = vec2d(mx * sx, my * sy);

			last_mouse_pos.set(scaled.x, scaled.y);
			platform->last_mouse_pos_for_dt = scaled;
		}
		
		glfwSetCursorPosCallback(window, glfw_callbacks::cursor_callback);
	}

	void window::set_window_name(const std::string& name) {
		glfwSetWindowTitle(platform->window, name.c_str());
	}

	void window::set_window_border_enabled(const bool decorated) {
		glfwSetWindowAttrib(platform->window, GLFW_DECORATED, decorated ? GLFW_TRUE : GLFW_FALSE);
	}

	bool window::swap_buffers() { 
		glfwSwapBuffers(platform->window);
		return true;
	}

	void window::collect_entropy(local_entropy& output) {
		auto& window = platform->window;

		auto handle_event = [&](const auto& ch) {
			if (common_event_handler(ch, output)) {
				output.push_back(ch);
			}
		};

		if (glfwWindowShouldClose(window)) {
			LOG("GLFW: glfwWindowShouldClose returned true.");

			event::change ch;
			ch.msg = event::message::close;
			handle_event(ch);
		}

		glfwPollEvents();

		using namespace event;

		for (const auto& key : platform->unhandled_keys) {
			change ch;

			if (key.action == GLFW_PRESS) {
				ch.msg = event::message::keydown;
			}
			else if (key.action == GLFW_RELEASE) {
				ch.msg = event::message::keyup;
			}
			else {
				continue;
			}

			ch.data.key.key = translate_glfw_key(key.key);

			if (get_current_settings().log_keystrokes) {
				LOG("Keycode down: %x", static_cast<int>(key.key));
				LOG("Scancode down: %x", static_cast<int>(key.scancode));
				LOG("Output enum: %x", int(ch.data.key.key));
			}

			handle_event(ch);
		}

		for (const auto& character : platform->unhandled_characters) {
			change ch;
			ch.msg = event::message::character;
			ch.data.character.code_point = character.codepoint;

			handle_event(ch);
		}

		for (const auto& mouse_button : platform->unhandled_mouse_buttons) {
			change ch;

			if (mouse_button.action == GLFW_PRESS) {
				ch.msg = event::message::keydown;
			}
			else if (mouse_button.action == GLFW_RELEASE) {
				ch.msg = event::message::keyup;
			}
			else {
				continue;
			}

			ch.data.key.key = translate_glfw_mouse_key(mouse_button.button);

			handle_event(ch);
		}

		for (const auto& scroll : platform->unhandled_scrolls) {
			change ch;
			ch.msg = message::wheel;
			ch.data.scroll.amount = static_cast<int>(scroll.yoffset);

			handle_event(ch);
		}

		for (const auto& cursor : platform->unhandled_cursors) {
			const auto sx = static_cast<double>(platform->content_scale_x);
			const auto sy = static_cast<double>(platform->content_scale_y);

			const auto new_mouse_pos = vec2d(cursor.xpos * sx, cursor.ypos * sy);

			if (!platform->mouse_pos_initialized) {
				platform->last_mouse_pos_for_dt = new_mouse_pos;
				platform->mouse_pos_initialized = true;
			}

			const auto dt = new_mouse_pos - platform->last_mouse_pos_for_dt;
			platform->last_mouse_pos_for_dt = new_mouse_pos;

			if (is_active() && (current_settings.draws_own_cursor() || mouse_pos_paused)) {
				const auto ch = do_raw_motion({
					static_cast<short>(dt.x),
					static_cast<short>(dt.y) 
				});

				handle_event(ch);
			}
			else {
				const auto new_pos = basic_vec2<short>{ 
					static_cast<short>(cursor.xpos * sx),
					static_cast<short>(cursor.ypos * sy)
			   	};

				if (const auto ch = handle_mousemove(new_pos)) {
					handle_event(*ch);
				}
			}
		}

		for (const auto& size : platform->unhandled_window_sizes) {
			(void)size;

			change ch;
			ch.msg = message::resize;
			handle_event(ch);
		}

		for (const auto& pos : platform->unhandled_window_positions) {
			(void)pos;

			change ch;
			ch.msg = message::move;
			handle_event(ch);
		}

		for (const auto& focus : platform->unhandled_focuses) {
			change ch;
			ch.msg = focus.focused == 0 ? message::deactivate : message::activate;
			handle_event(ch);
		}

		platform->unhandled_keys.clear();
		platform->unhandled_characters.clear();
		platform->unhandled_mouse_buttons.clear();
		platform->unhandled_scrolls.clear();
		platform->unhandled_cursors.clear();

		platform->unhandled_window_positions.clear();
		platform->unhandled_window_sizes.clear();

		platform->unhandled_focuses.clear();

#if PLATFORM_LINUX
		std::lock_guard<std::mutex> lock(clipboard_mutex);

		if (set_clipboard_op) {
			clipboard_snapshot = *set_clipboard_op;
			glfwSetClipboardString(platform->window, set_clipboard_op->c_str());
			set_clipboard_op.reset();
		}
#endif
	}

#if PLATFORM_LINUX
	void window::refresh_clipboard_snapshot() {
		if (platform == nullptr || platform->window == nullptr) {
			return;
		}

		std::lock_guard<std::mutex> lock(clipboard_mutex);

		const auto clip = glfwGetClipboardString(platform->window);

		if (clip != nullptr) {
			clipboard_snapshot = std::string(clip);
		}
	}
#endif

	void window::set_window_rect(const xywhi r) {
		auto& window = platform->window;
		LOG("SETTING WINDOW RECT: %x", r);

		glfwSetWindowPos(window, r.x, r.y);
		glfwSetWindowSize(window, r.w, r.h);
	}
	
	void window::set_fullscreen_hint(const bool hint) {
		GLFWmonitor* monitor = get_current_monitor(platform->window);

		if (monitor) {
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			if (mode) {
				if (hint) {
					LOG("SETTING FULLSCREEN MODE: %xx%x@%x Hz", mode->width, mode->height, mode->refreshRate);
					glfwSetWindowMonitor(platform->window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
				}
				else {
					const auto rect = last_windowed_rect;
					LOG("SETTING WINDOWED MODE.");
					glfwSetWindowMonitor(platform->window, NULL, rect.x, rect.y, rect.w, rect.h, 0);
				}
			}
		}
	}

	xywhi window::get_window_rect_impl() const { 
		const auto& window = platform->window;

		int width, height;
		int xpos, ypos;

		glfwGetFramebufferSize(window, &width, &height);
		glfwGetWindowPos(window, &xpos, &ypos);

		return { xpos, ypos, width, height }; 
	}

	xywhi window::get_display() const { 
		GLFWmonitor* monitor = get_current_monitor(platform->window);

		if (monitor) {
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			if (mode) {
				return { 0, 0, mode->width, mode->height };
			}
		}

		return {};
	}

	void window::destroy() {
		if (platform->window != nullptr) {
			glfwDestroyWindow(platform->window);
			glfwTerminate();
			platform->window = nullptr;
		}
	}

	bool window::set_as_current_impl() {
		LOG("GLFW: calling glfwMakeContextCurrent");
		glfwMakeContextCurrent(platform->window);
		return true;
	}

	void window::set_current_to_none_impl() {
		LOG("GLFW: calling glfwMakeContextCurrent with nullptr");

		glfwMakeContextCurrent(nullptr);
	}

	void window::set_cursor_pos(const vec2i pos) {
		platform->last_mouse_pos_for_dt = pos;
		last_mouse_pos = pos;
	
		const auto sx = static_cast<double>(platform->content_scale_x);
		const auto sy = static_cast<double>(platform->content_scale_y);

		glfwSetCursorPos(platform->window, pos.x / sx, pos.y / sy);
		platform->unhandled_cursors.clear();

	}

#if PLATFORM_LINUX
	bool window::set_cursor_clipping_impl(const bool flag) {
		if (flag) {
			platform->mouse_pos_initialized = false;
		}

		Display* display = platform->has_x11 ? glfwGetX11Display() : nullptr;
		Window window_id = display ? glfwGetX11Window(platform->window) : 0;

		// LOG("set_cursor_clipping_impl: %x. has x11: %x", flag, display != nullptr);

		glfwSetInputMode(platform->window, GLFW_CURSOR, flag ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

		if (flag && glfwRawMouseMotionSupported()) {
			glfwSetInputMode(platform->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else {
			glfwSetInputMode(platform->window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
		}

		if (!display) {
			return true;
		}

		/*
			If we're on X11, we need to properly grab the dumbass pointer
			by force
		*/

		if (flag) {
			auto result = platform->XGrabPointer_ptr(
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

			// LOG("Grab result: %x", result);

			if (result != GrabSuccess) {
				return false;
			}

			platform->XSync_ptr(display, False);
		}
		else {
			platform->XUngrabPointer_ptr(display, CurrentTime);

			platform->XSync_ptr(display, False);
		}

		return true;
	}

	void window::set_cursor_visible_impl(const bool) {
		/*
			Handled by glfwSetInputMode
		*/
	}
#else
	void window::set_cursor_visible_impl(bool) {
		/* Implemented with GLFW_CURSOR_DISABLED */
	}

	bool window::set_cursor_clipping_impl(bool clip) {
		if (clip && platform->clips_called == 0 && current_settings.draws_own_cursor()) {
			platform->mouse_pos_initialized = false;
			platform->clips_called = 1;
		}

		glfwSetInputMode(platform->window, GLFW_CURSOR, clip ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		return true;
	}
#endif

	void window::set(const vsync_type mode) {
		switch (mode) {
			case vsync_type::OFF: glfwSwapInterval(0); break;
			case vsync_type::ON: glfwSwapInterval(1); break;
			case vsync_type::ADAPTIVE: glfwSwapInterval(-1); break;

			default: glfwSwapInterval(0); break;
		}
	}

	int window::get_refresh_rate() {
		GLFWmonitor* monitor = get_current_monitor(platform->window);

		if (monitor) {
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			if (mode) {
				return mode->refreshRate;
			}
		}

		return -1;
	}

	void window::check_current_context() {

	}

	window::~window() {
		destroy();
	}
}

#include "augs/window_framework/translate_glfw_enums.hpp"
