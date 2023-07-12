#include <queue>
#include <GLFW/glfw3.h>

#include "augs/window_framework/window.h"
#include "augs/window_framework/translate_glfw_enums.h"
#include "augs/log.h"
#include "augs/window_framework/shell.h"
#include "augs/filesystem/file.h"

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
		vec2d last_mouse_pos;

		std::vector<unhandled_key> unhandled_keys;
		std::vector<unhandled_char> unhandled_characters;
		std::vector<unhandled_mouse_button> unhandled_mouse_buttons;
		std::vector<unhandled_scroll> unhandled_scrolls;
		std::vector<unhandled_cursor> unhandled_cursors;

		std::vector<unhandled_window_position> unhandled_window_positions;
		std::vector<unhandled_window_size> unhandled_window_sizes;
		std::vector<unhandled_focus> unhandled_focuses;
	};
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

	static void window_size_callback(GLFWwindow* window, int xsize, int ysize) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_window_sizes.push_back({ xsize, ysize });
	}

	static void focus_callback(GLFWwindow* window, int focused) {
		auto wnd = reinterpret_cast<augs::window*>(glfwGetWindowUserPointer(window));
		wnd->platform->unhandled_focuses.push_back({ focused });
	}
};

static void error_callback(int error, const char* description) {
	LOG("Error %x: %x\n", error, description);
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
		
		std::filesystem::current_path(previous_path);

		auto& window = platform->window;

		LOG("GLFW: setting version hints via glfwWindowHint.");

		glfwWindowHint(GLFW_SAMPLES, 0);
		glfwWindowHint(GLFW_DEPTH_BITS, 0);
		glfwWindowHint(GLFW_DECORATED, settings.border);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		LOG("GLFW: calling glfwCreateWindow.");

		if (settings.fullscreen) {
			GLFWmonitor* primary = glfwGetPrimaryMonitor();

			if (primary) {
				const GLFWvidmode* mode = glfwGetVideoMode(primary);
				window = glfwCreateWindow(mode->width, mode->height, settings.name.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
			}
		}
		else {
			window = glfwCreateWindow(settings.size.x, settings.size.y, settings.name.c_str(), NULL, NULL);
		}


		if (window == nullptr) {
			destroy();
			throw window_error("glfwCreateWindow failed.");
		}

		glfwSetWindowUserPointer(window, this);

		glfwSetKeyCallback(window, glfw_callbacks::key_callback);
		glfwSetCharCallback(window, glfw_callbacks::character_callback);
		glfwSetMouseButtonCallback(window, glfw_callbacks::mouse_button_callback);
		glfwSetScrollCallback(window, glfw_callbacks::scroll_callback);
		glfwSetCursorPosCallback(window, glfw_callbacks::cursor_callback);

		glfwSetWindowPosCallback(window, glfw_callbacks::window_pos_callback);
		glfwSetWindowSizeCallback(window, glfw_callbacks::window_size_callback);

		glfwSetWindowFocusCallback(window, glfw_callbacks::focus_callback);

		if (glfwRawMouseMotionSupported()) {
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		glfwFocusWindow(window);
		active = glfwGetWindowAttrib(window, GLFW_FOCUSED);

		set_as_current();
		set(settings.vsync_mode);

		current_settings = settings;
		current_rect = get_window_rect_impl();
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
			common_event_handler(ch, output);
			output.push_back(ch);
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
			auto new_mouse_pos = vec2d(cursor.xpos, cursor.ypos);
			auto dt = new_mouse_pos - platform->last_mouse_pos;
			platform->last_mouse_pos = new_mouse_pos;

			if (is_active() && (current_settings.draws_own_cursor() || mouse_pos_paused)) {
				LOG_NVPS(dt.x, dt.y);
				auto ch = do_raw_motion({
					static_cast<short>(dt.x),
					static_cast<short>(dt.y) 
				});

				handle_event(ch);
			}
			else {
				const auto new_pos = basic_vec2<short>{ 
					static_cast<short>(cursor.xpos),
					static_cast<short>(cursor.ypos)
			   	};

				auto ch = handle_mousemove(new_pos);
				handle_event(*ch);
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
	}

	void window::set_window_rect(const xywhi r) {
		auto& window = platform->window;
					LOG("SETTING WINDOW RECT: %x", r);

		glfwSetWindowPos(window, r.x, r.y);
		glfwSetWindowSize(window, r.w, r.h);
	}
	
	void window::set_fullscreen_hint(const bool hint) {
		GLFWmonitor* primary = glfwGetPrimaryMonitor();

		if (primary) {
			const GLFWvidmode* mode = glfwGetVideoMode(primary);

			if (mode) {
				if (hint) {
					LOG("SETTING FULLSCREEN MODE: %xx%x@%x Hz", mode->width, mode->height, mode->refreshRate);
					glfwSetWindowMonitor(platform->window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
				}
				else {
					const auto pos = current_settings.position;
					const auto size = current_settings.size;
					LOG("SETTING WINDOWED MODE.");
					glfwSetWindowMonitor(platform->window, NULL, pos.x, pos.y, size.x, size.y, 0);
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
		GLFWmonitor* primary = glfwGetPrimaryMonitor();

		if (primary) {
			const GLFWvidmode* mode = glfwGetVideoMode(primary);

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
		last_mouse_pos = pos;
	
	}

#if PLATFORM_LINUX
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
#endif

	std::optional<std::string> window::open_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& /* custom_title */
	) {
#if PLATFORM_LINUX
		return choose_path(DETAIL_DIR "/unix/open_file.local");
#else
		return std::nullopt;
#endif
	}

	std::optional<std::string> window::save_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& /* custom_title */
	) {
#if PLATFORM_LINUX
		return choose_path(DETAIL_DIR "/unix/save_file.local");
#else
		return std::nullopt;
#endif
	}

	std::optional<std::string> window::choose_directory_dialog(
		const std::string& /* custom_title */
	) {
#if PLATFORM_LINUX
		return choose_path(DETAIL_DIR "/unix/choose_directory.local");
#else
		return std::nullopt;
#endif
	}

	void window::reveal_in_explorer(const augs::path_type& full_path) {
		const auto command = augs::path_type(typesafe_sprintf("open -a Finder \"%x\"", full_path));
		shell(command);
	}

	void window::set_cursor_visible_impl(bool) {
		/* Implemented with GLFW_CURSOR_DISABLED */
	}

	bool window::set_cursor_clipping_impl(bool clip) {
		LOG_NVPS(clip);
		glfwSetInputMode(platform->window, GLFW_CURSOR, clip ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		return true;
	}

	void window::set(const vsync_type mode) {
		switch (mode) {
			case vsync_type::OFF: glfwSwapInterval(0); break;
			case vsync_type::ON: glfwSwapInterval(1); break;
			case vsync_type::ADAPTIVE: glfwSwapInterval(-1); break;

			default: glfwSwapInterval(0); break;
		}
	}

	message_box_button window::retry_cancel(
		const std::string& caption,
		const std::string& text
	) {
		LOG("RETRY CANCEL!!");
		LOG_NVPS(caption, text);
		return message_box_button::CANCEL;
	}

	int window::get_refresh_rate() {
		GLFWmonitor* primary = glfwGetPrimaryMonitor();

		if (primary) {
			const GLFWvidmode* mode = glfwGetVideoMode(primary);

			if (mode) {
				return mode->refreshRate;
			}
		}

		return -1;
	}

	window::~window() {
		destroy();
	}
}


