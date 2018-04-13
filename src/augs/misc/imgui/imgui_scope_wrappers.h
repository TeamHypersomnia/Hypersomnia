#pragma once
#include <optional>

#include "3rdparty/imgui/imgui.h"
#include "3rdparty/imgui/imgui_internal.h"

#include "augs/misc/imgui/imgui_controls.h"

#include "augs/graphics/rgba.h"
#include "augs/misc/scope_guard.h"

namespace augs {
	namespace imgui {
		template <class Pre, class Post>
		decltype(auto) cond_scoped_op(const bool cond, Pre pre, Post post) {
			if (cond) {
				pre();
			}

			return make_scope_guard([&post, cond]() { if (cond) { post(); } });
		}

		template <class... T>
		auto scoped_child(T&&... args) {
			ImGui::BeginChild(std::forward<T>(args)...);
		
			return make_scope_guard([]() { ImGui::EndChild(); });
		}
		
		template <class... T>
		auto cond_scoped_window(const bool do_it, T&&... args) {
			return cond_scoped_op(
				do_it,
				[&](){ ImGui::Begin(std::forward<T>(args)...); },
				[]() { ImGui::End(); }
			);
		}

		template <class... T>
		auto scoped_window(T&&... args) {
			return cond_scoped_window(true, std::forward<T>(args)...);
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

		inline auto scoped_text_color(const rgba& col) {
			return scoped_style_color(ImGuiCol_Text, col);
		}

		template <class... T>
		auto scoped_style_var(T&&... args) {
			ImGui::PushStyleVar(std::forward<T>(args)...);
		
			return make_scope_guard([]() { ImGui::PopStyleVar(); });
		}

		template <class... T>
		auto scoped_modal_popup(const std::string& label, T&&... args) {
			const auto result = ImGui::BeginPopupModal(label.c_str(), std::forward<T>(args)...);

			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::EndPopup(); }}));

			if (!result) {
				opt = std::nullopt;
			}

			return opt;
		}

		template <class... T>
		auto scoped_tree_node_ex(const std::string& label, T&&... args) {
			const auto result = ImGui::TreeNodeEx(label.c_str(), std::forward<T>(args)...);

			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::TreePop(); }}));

			if (!result) {
				opt = std::nullopt;
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
		
			return make_scope_guard([]() { ImGui::EndTooltip(); });
		}
		
		inline auto scoped_item_width(const float v) {
			ImGui::PushItemWidth(v);
		
			return make_scope_guard([]() { ImGui::PopItemWidth(); });
		}

		inline auto scoped_id(const int v) {
			ImGui::PushID(v);

			return make_scope_guard([]() { ImGui::PopID(); });
		}

		inline auto scoped_id(const void* const v) {
			ImGui::PushID(v);

			return make_scope_guard([]() { ImGui::PopID(); });
		}
		
		inline auto scoped_tree_node(const char* label) {
			const auto result = ImGui::TreeNode(label);
		
			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::TreePop(); }}));
		
			if (!result) {
				opt = std::nullopt;
			}
		
			return opt;
		}

		inline auto scoped_menu(const char* label, const bool enabled = true) {
			const auto result = ImGui::BeginMenu(label, enabled);

			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::EndMenu(); }}));

			if (!result) {
				opt = std::nullopt;
			}

			return opt;
		}

		inline auto scoped_main_menu_bar() {
			const auto result = ImGui::BeginMainMenuBar();

			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::EndMainMenuBar(); }}));

			if (!result) {
				opt = std::nullopt;
			}

			return opt;
		}

		inline auto scoped_tab_menu_bar(const float y) {
			const auto result = ImGui::BeginTabMenuBar(y);

			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::EndTabMenuBar(); }}));

			if (!result) {
				opt = std::nullopt;
			}

			return opt;
		}

	}
}