#include "imgui_utils.h"
#include "augs/log.h"

#include <imgui/imgui_internal.h>

#include "augs/filesystem/file.h"

#include "augs/image/image.h"
#include "augs/image/font.h"
#include "augs/graphics/texture.h"
#include "augs/window_framework/window.h"

#include "augs/window_framework/platform_utils.h"
#include "augs/templates/main_thread_queue.h"

using namespace ImGui;

#if PLATFORM_WEB

#include <emscripten_browser_clipboard.h>
static std::string web_clipboard;

/* 
	Idk it deadlocked for some reason randomly so better safe than sorry and use recursive
*/

static std::recursive_mutex clipboard_lk;

static const char* web_GetClipboardText(void*) {
	thread_local std::string clip_storage;

	LOG("web_GetClipboardText");
	{
		std::scoped_lock lk(clipboard_lk);
		clip_storage = web_clipboard;
	}

    return clip_storage.c_str();
}

static void web_SetClipboardText(void*, const char* text) {
	LOG("web_SetClipboardText");

	{
		std::scoped_lock lk(clipboard_lk);
		web_clipboard = text;
	}

	const auto clip = std::string(text);

	main_thread_queue::execute_async(
		[clip]() {
			emscripten_browser_clipboard::copy(clip);
		}
	);
}

#elif PLATFORM_LINUX
#if USE_GLFW
#include <GLFW/glfw3.h>
#include <mutex>

namespace augs {
	GLFWwindow* get_glfw_window(const window& d);
}

extern std::mutex clipboard_mutex;
extern std::optional<std::string> set_clipboard_op;
extern std::string clipboard_snapshot;

static const char* augs_GetClipboardText(void*) {
	thread_local std::string sss;

	{
		std::lock_guard<std::mutex> lock(clipboard_mutex);
		sss = clipboard_snapshot;
	}

	return sss.c_str();
}

static void augs_SetClipboardText(void*, const char* text) {
    std::lock_guard<std::mutex> lock(clipboard_mutex);
    set_clipboard_op = text;
}

#else
#include "augs/window_framework/shell.h"
#include "augs/window_framework/exec.h"

void unix_set_clipboard_data(const std::string& abc) {
	const auto p = augs::path_type("/tmp/augs_clipboard_data.txt"); 
	augs::save_as_text(p, abc);
	augs::shell("xclip -selection CLIPBOARD -in " + p.string());
}

std::string unix_get_clipboard_data() {
	return augs::exec("xclip -selection CLIPBOARD -out");
}

static const char* augs_GetClipboardText(void*) {
	thread_local std::string sss;
	sss = unix_get_clipboard_data();
	return sss.c_str();
}

static void augs_SetClipboardText(void*, const char* text) {
	unix_set_clipboard_data(text);
}
#endif
#endif

namespace augs {
	namespace imgui {
		/* Definition */
		std::optional<ImGuiID> next_window_to_close;

		context_raii::~context_raii() {
			ImGui::DestroyContext();
		}

		context_raii::context_raii(
			const char* const ini_path,
			const char* const log_path,
			const ImGuiStyle& initial_style
		) {
			ImGui::CreateContext();

			auto& io = GetIO();
			
			using namespace augs::event::keys;

			auto map_key = [&io](auto im, auto aug) {
				io.KeyMap[im] = int(aug);
			};

			map_key(ImGuiKey_Tab, key::TAB);
			map_key(ImGuiKey_LeftArrow, key::LEFT);
			map_key(ImGuiKey_RightArrow, key::RIGHT);
			map_key(ImGuiKey_UpArrow, key::UP);
			map_key(ImGuiKey_DownArrow, key::DOWN);
			map_key(ImGuiKey_PageUp, key::PAGEUP);
			map_key(ImGuiKey_PageDown, key::PAGEDOWN);
			map_key(ImGuiKey_Home, key::HOME);
			map_key(ImGuiKey_End, key::END);
			map_key(ImGuiKey_Delete, key::DEL);
			map_key(ImGuiKey_Backspace, key::BACKSPACE);
			map_key(ImGuiKey_Enter, key::ENTER);
			map_key(ImGuiKey_Escape, key::ESC);
			map_key(ImGuiKey_A, key::A);
			map_key(ImGuiKey_C, key::C);
			map_key(ImGuiKey_V, key::V);
			map_key(ImGuiKey_X, key::X);
			map_key(ImGuiKey_Y, key::Y);
			map_key(ImGuiKey_Z, key::Z);

			io.IniFilename = ini_path;
			io.LogFilename = log_path;

#if PLATFORM_WEB
			io.SetClipboardTextFn = web_SetClipboardText;
			io.GetClipboardTextFn = web_GetClipboardText;

			emscripten_browser_clipboard::paste([](const std::string &paste_data, void *callback_data [[maybe_unused]]){
				LOG("emscripten_browser_clipboard::paste");

				{
					std::scoped_lock lk(clipboard_lk);
					web_clipboard = paste_data;
				}
			});
#elif PLATFORM_LINUX
			io.SetClipboardTextFn = augs_SetClipboardText;
			io.GetClipboardTextFn = augs_GetClipboardText;
#endif

			//io.MouseDoubleClickMaxDist = 100.f;
			GetStyle() = initial_style;

			// ImGui::GetStyle().ScaleAllSizes(2.0f);
		}

		void setup_io_settings(
			const decltype(ImGuiIO::DeltaTime) delta_seconds,
			const vec2i screen_size
		) {
			using namespace event;
			using namespace event::keys;

			auto& io = GetIO();

			io.MouseDrawCursor = false;
			io.DeltaTime = delta_seconds;
			io.DisplaySize = ImVec2(screen_size);

			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
		}

		void pass_inputs(const local_entropy& window_inputs) {
			using namespace event;
			using namespace event::keys;

			auto& io = GetIO();

			for (const auto& in : window_inputs) {
				if (in.msg == message::mousemotion) {
					io.MousePos = ImVec2(in.data.mouse.pos);
				}
				else if (in.was_pressed(key::LMOUSE)) {
					io.MouseDown[0] = true;
				}
				else if (in.was_released(key::LMOUSE)) {
					io.MouseDown[0] = false;
				}
				else if (in.was_pressed(key::RMOUSE)) {
					io.MouseDown[1] = true;
				}
				else if (in.was_released(key::RMOUSE)) {
					io.MouseDown[1] = false;
				}
				else if (in.msg == message::wheel) {
					io.MouseWheel = static_cast<float>(in.data.scroll.amount);
				}
				else if (in.msg == message::keydown) {
					io.KeysDown[static_cast<int>(in.data.key.key)] = true;
				}
				else if (in.msg == message::keyup) {
					io.KeysDown[static_cast<int>(in.data.key.key)] = false;
				}
				else if (in.msg == message::syskeydown) {
					io.KeysDown[static_cast<int>(in.data.key.key)] = true;
				}
				else if (in.msg == message::syskeyup) {
					io.KeysDown[static_cast<int>(in.data.key.key)] = false;
				}
				else if (in.msg == message::character) {
					const auto c = in.data.character.code_point;

					// TODO:
					// FIXME: Losing characters that don't fit in 2 bytes (imgui issue)
					if (c < 0x10000) {
						io.AddInputCharacter(c);
					}
				}

				io.KeyCtrl = io.KeysDown[static_cast<int>(keys::key::LCTRL)] || io.KeysDown[static_cast<int>(keys::key::RCTRL)];
				io.KeyShift = io.KeysDown[static_cast<int>(keys::key::LSHIFT)] || io.KeysDown[static_cast<int>(keys::key::RSHIFT)];
				io.KeyAlt = io.KeysDown[static_cast<int>(keys::key::LALT)] || io.KeysDown[static_cast<int>(keys::key::RALT)];
			}
		}

		void render() {
			Render();
		}

		void release_mouse_buttons() {
			auto& io = GetIO();

			for (auto& m : io.MouseDown) {
				m = false;
			}
		}

		void neutralize_mouse() {
			auto& io = GetIO();

			io.MousePos = { -1, -1 };

			release_mouse_buttons();
		}
		
		image create_atlas_image(const font_loading_input& in, float ratio) {
			auto& io = GetIO();

			io.Fonts->Clear();

			unsigned char* pixels = nullptr;
			int width = 0;
			int height = 0;

			std::vector<ImWchar> ranges;
			ranges.push_back(0x25B2);
			ranges.push_back(0x25BC);

			{
				auto unicode_ranges = in.unicode_ranges;

				if (_should(in.add_japanese_ranges)) {
					concat_ranges(unicode_ranges, io.Fonts->GetGlyphRangesJapanese());
				}

				if (_should(in.add_cyrillic_ranges)) {
					concat_ranges(unicode_ranges, io.Fonts->GetGlyphRangesCyrillic());
				}

				for (const auto& r : unicode_ranges) {
					ranges.push_back(r.first);
					ranges.push_back(r.second);
				}

				ranges.push_back(0);
			}

			if (!augs::exists(in.source_font_path)) {
				throw imgui_init_error("Failed to load %x for reading.", augs::filename_first(in.source_font_path));
			}

			const auto str_path = in.source_font_path.string();

			io.Fonts->AddFontFromFileTTF(str_path.c_str(), ratio * in.size_in_pixels, nullptr, ranges.data());
			io.Fonts->AddFontFromFileTTF(str_path.c_str(), ratio * in.size_in_pixels * 1.2f, nullptr, io.Fonts->GetGlyphRangesDefault());
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

			LOG("IMGUI FONT ATLAS SIZE: %xx%x", width, height);

			io.Fonts->TexID = reinterpret_cast<void*>(0);

			return { pixels, vec2i{ width, height } };
		}

		graphics::texture create_atlas(const font_loading_input& in, float ratio) {
			return create_atlas_image(in, ratio);
		}

		bool is_hovered_with_hand_cursor() {
			// TODO: when all else is done, we may want to do this.
			return false;// IsAnyItemHovered() && GetCurrentContext()->HoveredIdHandCursor;
		}

		bool mouse_over_any_window() {
			return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
		}

		void center_next_window(ImGuiCond cond) {
			const auto screen_size = vec2(GetIO().DisplaySize);
			SetNextWindowPos(ImVec2{ screen_size / 2 }, cond, ImVec2(0.5f, 0.5f));
		}

		void center_next_window(const vec2 size_multiplier, const ImGuiCond cond) {
			const auto screen_size = vec2(GetIO().DisplaySize);
			const auto initial_settings_size = screen_size * size_multiplier;

			set_next_window_rect(
				{
					{ screen_size / 2 - initial_settings_size / 2 },
					initial_settings_size,
				},
				cond
			);
		}

		void set_next_window_rect(const xywh r, const ImGuiCond cond) {
			SetNextWindowPos(ImVec2(r.get_position()), cond);
			SetNextWindowSize(ImVec2(r.get_size()), cond);
		}

		augs::local_entropy filter_inputs(augs::local_entropy local) {
			const bool filter_mouse = ImGui::GetIO().WantCaptureMouse;
			const bool filter_keyboard = ImGui::GetIO().WantTextInput;

			using namespace augs::event;
			using namespace keys;

			erase_if(local, [filter_mouse, filter_keyboard](const change ch) {
				if (filter_mouse && ch.msg == message::mousemotion) {
					return true;
				}

				if (filter_mouse && ch.msg == message::wheel) {
					return true;
				}
				
				/* We always let release events propagate */

				if (ch.was_any_key_pressed()) {
					if (filter_mouse && ch.uses_mouse()) {
						return true;
					}

					if (filter_keyboard && ch.uses_keyboard()) {
						if (ch.is_shortcut_key()) {
							return false;
						}

						return true;
					}
				}

				return false;
			});

			return local;
		}

		void filter_with_hint(ImGuiTextFilter& filter, const char* id, const char* hint) {
			ImGui::SetNextItemWidth(-0.0001f);

			if (ImGui::InputTextWithHint(id, hint, filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf))) {
				filter.Build();
			}
		}
	}
}
