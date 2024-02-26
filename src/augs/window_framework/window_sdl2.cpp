#include <SDL.h>

#include "augs/window_framework/window.h"
#include "augs/log.h"
#include "augs/filesystem/file.h"

augs::event::keys::key translate_sdl2_key(int);
augs::event::keys::key translate_sdl2_mouse_key(int);

namespace augs {
    struct window::platform_data {
		vec2d last_mouse_pos_for_dt;

        SDL_Window* window = nullptr;
        SDL_GLContext gl_context;
    };

    SDL_Window* get_sdl_window(const window::platform_data& d) {
        return d.window;
    }
}

namespace augs {
    window::window(const window_settings& settings)
        : platform(std::make_unique<window::platform_data>()) {
        
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw window_error("SDL_Init failed: " + std::string(SDL_GetError()));
        }

		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        #ifdef __EMSCRIPTEN__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); // Use WebGL 2.0
        #else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        #endif

        platform->window = SDL_CreateWindow(settings.name.c_str(), 
                                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                            settings.size.x, settings.size.y, 
                                            SDL_WINDOW_OPENGL | (settings.border ? 0 : SDL_WINDOW_BORDERLESS) | 
                                            (settings.fullscreen ? SDL_WINDOW_FULLSCREEN : 0));

        if (!platform->window) {
            SDL_Quit();
            throw window_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
        }

        platform->gl_context = SDL_GL_CreateContext(platform->window);

        if (!platform->gl_context) {
            SDL_DestroyWindow(platform->window);
            SDL_Quit();
            throw window_error("SDL_GL_CreateContext failed: " + std::string(SDL_GetError()));
        }

		set_as_current();

        SDL_GL_SetSwapInterval(settings.vsync_mode == vsync_type::ON ? 1 : 0);

        current_settings = settings;
        last_windowed_rect = current_settings.make_window_rect();
        current_rect = get_window_rect_impl();
        last_mouse_pos = current_rect.get_size() / 2;
        set_cursor_pos(last_mouse_pos);
    }

    void window::set_window_name(const std::string& name) {
        SDL_SetWindowTitle(platform->window, name.c_str());
    }

    void window::set_window_border_enabled(const bool decorated) {
        SDL_SetWindowBordered(platform->window, decorated ? SDL_TRUE : SDL_FALSE);
    }

    bool window::swap_buffers() {
        SDL_GL_SwapWindow(platform->window);
        return true;
    }

    void window::collect_entropy(local_entropy& output) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    {
                        event::change ch;
                        ch.msg = event.type == SDL_KEYDOWN ? event::message::keydown : event::message::keyup;
                        ch.data.key.key = translate_sdl2_key(event.key.keysym.sym);
                        common_event_handler(ch, output);
                        output.push_back(ch);
                    }
                    break;
                case SDL_TEXTINPUT:
                    {
                        event::change ch;
                        ch.msg = event::message::character;
                        ch.data.character.code_point = static_cast<unsigned int>(*event.text.text);
                        common_event_handler(ch, output);
                        output.push_back(ch);
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    {
                        event::change ch;
                        ch.msg = event.type == SDL_MOUSEBUTTONDOWN ? event::message::keydown : event::message::keyup;
                        ch.data.key.key = translate_sdl2_mouse_key(event.button.button);
                        common_event_handler(ch, output);
                        output.push_back(ch);
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    {
                        event::change ch;
                        ch.msg = event::message::wheel;
                        ch.data.scroll.amount = event.wheel.y;
                        common_event_handler(ch, output);
                        output.push_back(ch);
                    }
                    break;
                case SDL_MOUSEMOTION:
					if (SDL_GetRelativeMouseMode()) {
						auto dx = event.motion.xrel;
						auto dy = event.motion.yrel;

						if (is_active() && (current_settings.draws_own_cursor() || mouse_pos_paused)) {
							auto ch = do_raw_motion({
								static_cast<short>(dx),
								static_cast<short>(dy)
							});
							common_event_handler(ch, output);
							output.push_back(ch);
						}
					} 
					else {
						auto new_mouse_pos = vec2d(event.motion.x, event.motion.y);

						const auto new_pos = basic_vec2<short>{
							static_cast<short>(event.motion.x),
							static_cast<short>(event.motion.y)
						};

						auto ch = handle_mousemove(new_pos);
						if (ch) {
							common_event_handler(*ch, output);
							output.push_back(*ch);
						}
					}

                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        event::change ch;
                        ch.msg = event::message::resize;
                        common_event_handler(ch, output);
                        output.push_back(ch);
                    } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                        event::change ch;
                        ch.msg = event::message::activate;
                        common_event_handler(ch, output);
                        output.push_back(ch);
                    } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                        event::change ch;
                        ch.msg = event::message::deactivate;
                        common_event_handler(ch, output);
                        output.push_back(ch);
                    }
                    break;
                case SDL_QUIT:
                    {
                        event::change ch;
                        ch.msg = event::message::close;
                        common_event_handler(ch, output);
                        output.push_back(ch);
                    }
                    break;
            }
        }
    }

    void window::set_window_rect(const xywhi r) {
        SDL_SetWindowPosition(platform->window, r.x, r.y);
        SDL_SetWindowSize(platform->window, r.w, r.h);
    }

    void window::set_fullscreen_hint(const bool hint) {
        SDL_SetWindowFullscreen(platform->window, hint ? SDL_WINDOW_FULLSCREEN : 0);
    }

    xywhi window::get_window_rect_impl() const {
        int width, height, xpos, ypos;
        SDL_GetWindowSize(platform->window, &width, &height);
        SDL_GetWindowPosition(platform->window, &xpos, &ypos);
        return { xpos, ypos, width, height };
    }

    xywhi window::get_display() const {
        SDL_DisplayMode mode;
        if (SDL_GetDesktopDisplayMode(0, &mode) == 0) {
            return { 0, 0, mode.w, mode.h };
        }
        return {};
    }

    void window::destroy() {
        if (platform->gl_context) {
            SDL_GL_DeleteContext(platform->gl_context);
        }
        if (platform->window != nullptr) {
            SDL_DestroyWindow(platform->window);
            SDL_Quit();
            platform->window = nullptr;
        }
    }

    bool window::set_as_current_impl() {
        SDL_GL_MakeCurrent(platform->window, platform->gl_context);
        return true;
    }

    void window::set_current_to_none_impl() {
        SDL_GL_MakeCurrent(nullptr, nullptr);
    }

    void window::set_cursor_pos(const vec2i pos) {
        SDL_WarpMouseInWindow(platform->window, pos.x, pos.y);
        platform->last_mouse_pos_for_dt = pos;
        last_mouse_pos = pos;
    }

    void window::set_cursor_visible_impl(bool visible) {
        SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
    }

    bool window::set_cursor_clipping_impl(const bool clip) {
		SDL_bool relative = clip ? SDL_TRUE : SDL_FALSE;

		if (SDL_SetRelativeMouseMode(relative) == 0) {
			return true; // Successfully set the relative mouse mode
		}
		else {
			LOG("Error setting relative mouse mode: %s", SDL_GetError());
			return false; // Failed to set the relative mouse mode
		}
	}

    void window::set(const vsync_type mode) {
        SDL_GL_SetSwapInterval(mode == vsync_type::ON ? 1 : 0);
    }

    int window::get_refresh_rate() {
        SDL_DisplayMode mode;
        if (SDL_GetDesktopDisplayMode(0, &mode) == 0) {
            return mode.refresh_rate;
        }
        return -1;
    }

    void window::check_current_context() {
        // This function can be used to check the current OpenGL context, etc.
    }

    window::~window() {
        destroy();
    }
}

#include "augs/window_framework/translate_sdl2_enums.hpp"
