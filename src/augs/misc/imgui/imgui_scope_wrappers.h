#pragma once
#include "3rdparty/imgui/imgui.h"
#include "3rdparty/imgui/imgui_internal.h"

#include "augs/misc/imgui/imgui_controls.h"

#include "augs/graphics/rgba.h"
#include "augs/misc/scope_guard.h"

namespace augs {
	namespace imgui {
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

			return scope_guard([&post, cond]() { if (cond) { post(); } });
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
		auto cond_scoped_window(const bool do_it, T&&... args) {
			auto guard = scope_guard([](){ ImGui::End(); });

			if (!do_it) {
				guard.release();
				return window_scope(std::move(guard), false);
			}
			else {
				const bool is_open = ImGui::Begin(std::forward<T>(args)...);
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
		auto scoped_modal_popup(const std::string& label, T&&... args) {
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
		
		inline auto scoped_item_width(const float v) {
			ImGui::PushItemWidth(v);
		
			return scope_guard([]() { ImGui::PopItemWidth(); });
		}

		inline auto scoped_id(const int v) {
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

	}
}