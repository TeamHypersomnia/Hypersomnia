#pragma once

#if PLATFORM_UNIX
/*
	Hacky forward declarations to avoid the need for X includes.
	There are errors anyway when the declarations are conflicting,
	so we'll know when something's wrong.
*/
#include <cstdint>

struct __GLXcontextRec;
typedef struct __GLXcontextRec *GLXContext;

typedef unsigned long XID;
typedef XID GLXWindow;
typedef XID GLXDrawable;

typedef uint32_t xcb_window_t;
struct xcb_screen_t;
typedef unsigned long Window;

struct _XDisplay;
typedef struct _XDisplay Display;
struct xcb_connection_t;
typedef uint32_t xcb_timestamp_t;

struct _XCBKeySymbols;
typedef struct _XCBKeySymbols xcb_key_symbols_t;
#endif

#include <optional>

#include "augs/math/rects.h"
#include "augs/math/vec2.h"

#include "augs/templates/settable_as_current_mixin.h"
#include "augs/templates/exception_templates.h"

#include "augs/misc/timing/timer.h"
#include "augs/misc/machine_entropy.h"

#include "augs/window_framework/event.h"
#include "augs/window_framework/window_settings.h"

struct glfw_callbacks;

namespace augs {
	struct window_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	enum class message_box_button {
		RETRY,
		CANCEL
	};

	class window : public settable_as_current_mixin<window> {
#if USE_GLFW
		friend glfw_callbacks;
#endif

#if PLATFORM_WINDOWS || USE_GLFW
		struct platform_data;
		std::unique_ptr<platform_data> platform;

		local_entropy wndproc_queue;

		void show();

#if DECLARE_FRIEND_WNDPROC
	public:
		void handle_wndproc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
	private:
#endif

		template <class H, class U, class W, class L>
		std::optional<event::change> handle_event(
			const H,
			const U, 
			const W, 
			const L
		);

#elif PLATFORM_UNIX
		double smallest_raw_x_unit = 1.0;
		double smallest_raw_y_unit = 1.0;

		xcb_timestamp_t last_ldown_time_ms = 0;

		GLXContext context = 0;
		GLXWindow glxwindow = 0;
		xcb_window_t window_id = 0;
		xcb_screen_t* screen = 0;
		GLXDrawable drawable = 0;
		Display *display = nullptr;
		Window root = 0;
		xcb_connection_t *connection = nullptr;

		xcb_key_symbols_t* syms = nullptr;
		
		int xi_opcode = -1;
		uint32_t wm_delete_window_atom = static_cast<uint32_t>(-1);
		vec2i delta_since_click;
#else
#error "Unsupported platform!"
#endif
		/*
			Some window managers, like i3, do not allow us to set
			the window geometry directly, which is otherwise useful
			when last window pos/size is read from the configuration file.
		*/

		bool can_control_window_geometry = true;

		bool active = true;

		bool cursor_clipping = false;
		bool cursor_visible = true;

		bool mouse_pos_paused = false;
		vec2i last_mouse_pos;
		xywhi current_rect;

		std::optional<event::change> handle_mousemove(
			const basic_vec2<short> new_position
		);
		
		event::change do_raw_motion(const basic_vec2<short>);
		std::optional<event::change> sync_mouse_on_click_activate(const event::change&);
		void common_event_handler(event::change, local_entropy&);

		using settable_as_current_base = settable_as_current_mixin<window>;
		friend settable_as_current_base;

		bool set_as_current_impl();
		static void set_current_to_none_impl();

		void set_window_name(const std::string& name);
		void set_window_border_enabled(const bool);
		void set_window_rect(const xywhi);

		void set_fullscreen_geometry(const bool);
		void set_fullscreen_hint(const bool);

		window_settings current_settings;

		void set_cursor_visible_impl(bool flag); 
		bool set_cursor_clipping_impl(bool flag); 

		void destroy();

		xywhi get_window_rect_impl() const;
	public:
		window(const window_settings&);
		~window();

		window(window&&) = delete;
		window& operator=(window&&) = delete;

		window(const window&) = delete;
		window& operator=(const window&) = delete;

		bool swap_buffers();

		void set_mouse_pos_paused(const bool);
		bool is_mouse_pos_paused() const;

		void apply(const window_settings&, const bool force = false);
		void sync_back_into(window_settings&);
		window_settings get_current_settings() const;

		local_entropy collect_entropy();
		void collect_entropy(local_entropy& into);

		vec2i get_screen_size() const;
		xywhi get_window_rect() const;

		void set_cursor_pos(vec2i);
		void set_cursor_clipping(bool flag); 
		void set_cursor_visible(bool flag); 

		bool is_active() const;
		xywhi get_display() const;

		int get_refresh_rate();

		void set(vsync_type);

		struct file_dialog_filter {
			std::string description;
			std::string extension;
		};

		std::optional<std::string> open_file_dialog(
			const std::vector<file_dialog_filter>& filters,
			const std::string& custom_title = std::string()
		);

		std::optional<std::string> save_file_dialog(
			const std::vector<file_dialog_filter>& filters,
			const std::string& custom_title = std::string()
		);

		std::optional<std::string> choose_directory_dialog(
			const std::string& custom_title = std::string()
		);

		void reveal_in_explorer(const augs::path_type&);
		
		message_box_button retry_cancel(
			const std::string& caption,
			const std::string& text
		);

		auto get_last_mouse_pos() const {
			return last_mouse_pos;
		}
	};
}
