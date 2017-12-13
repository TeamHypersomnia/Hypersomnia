#pragma once
#include <optional>
#include <imgui/imgui.h>

#include "augs/misc/scope_guard.h"

namespace augs {
	namespace imgui {
		template <class... T>
		auto scoped_child(T&&... args) {
			ImGui::BeginChild(std::forward<T>(args)...);
		
			return make_scope_guard([]() { ImGui::EndChild(); });
		}
		
		template <class... T>
		auto scoped_window(T&&... args) {
			ImGui::Begin(std::forward<T>(args)...);
		
			return make_scope_guard([]() { ImGui::End(); });
		}
		
		template <class... T>
		auto scoped_style_color(T&&... args) {
			ImGui::PushStyleColor(std::forward<T>(args)...);
		
			return make_scope_guard([]() { ImGui::PopStyleColor(); });
		}
		
		template <class... T>
		auto scoped_style_var(T&&... args) {
			ImGui::PushStyleVar(std::forward<T>(args)...);
		
			return make_scope_guard([]() { ImGui::PopStyleVar(); });
		}

		template <class... T>
		auto scoped_modal_popup(T&&... args) {
			const auto result = ImGui::BeginPopupModal(std::forward<T>(args)...);

			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::EndPopup(); }}));

			if (!result) {
				opt = std::nullopt;
			}

			return opt;
		}

		inline auto scoped_indent() {
			ImGui::Indent();
		
			return make_scope_guard([]() { ImGui::Unindent(); });
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

		inline auto scoped_id(void* const v) {
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