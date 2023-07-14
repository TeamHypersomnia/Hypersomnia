#pragma once
#include "3rdparty/imgui/imgui.h"
#include "3rdparty/imgui/imgui_internal.h"

#include "augs/misc/imgui/imgui_controls.h"

#include "augs/graphics/rgba.h"
#include "augs/misc/scope_guard.h"
#include "augs/misc/pool/pooled_object_id.h"

namespace augs {
	namespace imgui {
		extern std::optional<ImGuiID> next_window_to_close;

		template <class T>
		struct window_scope {
			T guard;
			bool is_open = false;

			window_scope(T&& g, const bool is_open) 
				: guard(std::move(g)), is_open(is_open) 
			{}

			explicit operator bool() {
				return is_open;
			}
		};

		template <class Pre, class Post>
		decltype(auto) cond_scoped_op(bool cond, Pre pre, Post post) {
			if (cond) {
				if constexpr(std::is_same_v<bool, decltype(pre())>) {
					if (!pre()) {
						cond = false;
					}
				}
				else {
					pre();
				}
			}

			return scope_guard([post, cond]() { if (cond) { post(); } });
		}

		template <class... T>
		auto scoped_child(T&&... args) {
			ImGui::BeginChild(std::forward<T>(args)...);
		
			return scope_guard([]() { ImGui::EndChild(); });
		}
		
		template <class... T>
		auto scoped_window(T&&... args) {
			const bool is_open = ImGui::Begin(std::forward<T>(args)...);
			return window_scope(scope_guard([](){ ImGui::End(); }), is_open);
		}

		template <class... T>
		auto cond_scoped_window(const bool do_it, const auto label, bool* show, T&&... args) {
			auto guard = scope_guard([](){ ImGui::End(); });

			if (!do_it) {
				guard.release();
				return window_scope(std::move(guard), false);
			}
			else {
				const bool is_open = ImGui::Begin(label, show, std::forward<T>(args)...);

				ImGuiWindow* window = ImGui::GetCurrentWindow();

				if (next_window_to_close.has_value()) {
					if (next_window_to_close == window->ID) {
						ImGui::End();
						if (show) {
							*show = false;
						}
						next_window_to_close = std::nullopt;

						guard.release();
						return window_scope(std::move(guard), false);
					}
				}

				return window_scope(std::move(guard), is_open);
			}
		}

		template <class... T>
		auto cond_scoped_style_color(const bool do_it, T&&... args) {
			return cond_scoped_op(
				do_it,
			  	[&](){ ImGui::PushStyleColor(std::forward<T>(args)...); },
				[]() { ImGui::PopStyleColor(); }
			);
		}

		template <class... T>
		auto scoped_item_flag(T&&... args) {
			ImGui::PushItemFlag(std::forward<T>(args)...);

			return scope_guard([]() { ImGui::PopItemFlag(); });
		}

		template <class... T>
		auto cond_scoped_item_flag(const bool do_it, T&&... args) {
			return cond_scoped_op(
				do_it,
				[&](){ ImGui::PushItemFlag(std::forward<T>(args)...); },
				[]() { ImGui::PopItemFlag(); }
			);
		}

		template <class... T>
		auto scoped_style_color(T&&... args) {
			return cond_scoped_style_color(true, std::forward<T>(args)...);
		}

		template <class... T>
		auto scoped_combo(T&&... args) {
			const auto result = ImGui::BeginCombo(std::forward<T>(args)...);

			auto opt = scope_guard([]() { ImGui::EndCombo(); });

			if (!result) {
				opt.release();
			}

			return opt;
		}

		inline auto scoped_text_color(const rgba& col) {
			return scoped_style_color(ImGuiCol_Text, col);
		}

		template <class... T>
		auto scoped_style_var(T&&... args) {
			ImGui::PushStyleVar(std::forward<T>(args)...);
		
			return scope_guard([]() { ImGui::PopStyleVar(); });
		}

		template <class... T>
		auto cond_scoped_modal_popup(const bool should_be_open, const std::string& label, T&&... args) {
			if (should_be_open) {
				if (!ImGui::IsPopupOpen(label.c_str())) {
					ImGui::OpenPopup(label.c_str());
				}
			}

			const auto is_open = ImGui::BeginPopupModal(label.c_str(), std::forward<T>(args)...);

			if (is_open) {
				if (!should_be_open) {
					ImGui::CloseCurrentPopup();
				}
			}

			auto opt = scope_guard([]() { { ImGui::EndPopup(); }});

			if (!is_open) {
				opt.release();
			}

			return opt;
		}

		template <class... T>
		auto scoped_modal_popup(const std::string& label, T&&... args) {
			if (!ImGui::IsPopupOpen(label.c_str())) {
				ImGui::OpenPopup(label.c_str());
			}

			const auto result = ImGui::BeginPopupModal(label.c_str(), std::forward<T>(args)...);

			auto opt = scope_guard([]() { { ImGui::EndPopup(); }});

			if (!result) {
				opt.release();
			}

			return opt;
		}

		template <class... T>
		auto scoped_tree_node_ex(const std::string& label, T&&... args) {
			const auto result = ImGui::TreeNodeEx(label.c_str(), std::forward<T>(args)...);

			auto opt = scope_guard([]() { ImGui::TreePop(); });

			if (!result) {
				opt.release();
			}

			return opt;
		}

		inline auto cond_scoped_indent(const bool do_it) {
			return cond_scoped_op(
				do_it,
				[&](){ ImGui::Indent(); },
				[]() { ImGui::Unindent(); }
			);
		}

		inline auto scoped_indent() {
			return cond_scoped_indent(true);
		}
		
		inline auto scoped_tooltip() {
			ImGui::BeginTooltip();
		
			return scope_guard([]() { ImGui::EndTooltip(); });
		}
		
		inline auto scoped_group() {
			ImGui::BeginGroup();
		
			return scope_guard([]() { ImGui::EndGroup(); });
		}

		inline auto scoped_item_width(const float v) {
			ImGui::PushItemWidth(v);
		
			return scope_guard([]() { ImGui::PopItemWidth(); });
		}

		template <class size_type, class... keys>
		auto scoped_id(const pooled_object_id<size_type, keys...> id) {
			ImGui::PushID(id.indirection_index);
			ImGui::PushID(id.version);

			return scope_guard([]() { ImGui::PopID(); ImGui::PopID(); });
		}

		inline auto scoped_id(const int v) {
			ImGui::PushID(v);

			return scope_guard([]() { ImGui::PopID(); });
		}

		/*
			Note we cannot accept a const std::string&,
			because we have no guarantee that it wasn't created from a temporary.
			This would cause the subsequent ImGui calls to read pushed ID from an already freed memory.

			It already caused a bug once where buttons didn't react to clicks, but somehow only for Windows.

			In case c_str() is manually passed to this function, the std::string's lifetime will usually exceed the scope of the scoped_id call.
		*/

		inline auto scoped_id(const char* v) {
			ImGui::PushID(v);

			return scope_guard([]() { ImGui::PopID(); });
		}

		inline auto scoped_id(const void* const v) {
			ImGui::PushID(v);

			return scope_guard([]() { ImGui::PopID(); });
		}
		
		inline auto scoped_tree_node(const char* label) {
			const auto result = ImGui::TreeNode(label);
		
			auto opt = scope_guard([]() { { ImGui::TreePop(); }});
		
			if (!result) {
				opt.release();
			}
		
			return opt;
		}

		inline auto scoped_menu(const char* label, const bool enabled = true) {
			const auto result = ImGui::BeginMenu(label, enabled);

			auto opt = scope_guard([]() { { ImGui::EndMenu(); }});

			if (!result) {
				opt.release();
			}

			return opt;
		}

		inline auto scoped_menu_bar() {
			const auto result = ImGui::BeginMenuBar();

			auto opt = scope_guard([]() { { ImGui::EndMenuBar(); }});

			if (!result) {
				opt.release();
			}

			return opt;
		}

		inline auto scoped_main_menu_bar() {
			const auto result = ImGui::BeginMainMenuBar();

			auto opt = scope_guard([]() { { ImGui::EndMainMenuBar(); }});

			if (!result) {
				opt.release();
			}

			return opt;
		}

		inline auto scoped_tab_menu_bar(const float y) {
			const auto result = ImGui::BeginTabMenuBar(y);

			auto opt = scope_guard([]() { { ImGui::EndTabMenuBar(); }});

			if (!result) {
				opt.release();
			}

			return opt;
		}

		inline auto scoped_selectable_colors(const std::array<rgba, 3>& colors) {
			return std::make_tuple(
				scoped_style_color(ImGuiCol_Header, colors[0]),
				scoped_style_color(ImGuiCol_HeaderHovered, colors[1]),
				scoped_style_color(ImGuiCol_HeaderActive, colors[2])
			);
		}

		inline auto scoped_button_colors(const std::array<rgba, 3>& colors) {
			return std::make_tuple(
				scoped_style_color(ImGuiCol_Button, colors[0]),
				scoped_style_color(ImGuiCol_ButtonHovered, colors[1]),
				scoped_style_color(ImGuiCol_ButtonActive, colors[2])
			);
		}

		inline auto scoped_preserve_cursor() {
			const auto before_pos = ImGui::GetCursorPos();

			return scope_guard([before_pos]() { ImGui::SetCursorPos(before_pos); });
		};
	}
}