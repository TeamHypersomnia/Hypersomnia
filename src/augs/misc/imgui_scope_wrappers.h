#pragma once
#include <optional>
#include <imgui/imgui.h>

#include "augs/misc/scope_guard.h"

namespace augs {
	namespace imgui {
		auto scoped_indent() {
			ImGui::Indent();
		
			return make_scope_guard([]() { ImGui::Unindent(); });
		}
		
		auto scoped_tooltip() {
			ImGui::BeginTooltip();
		
			return make_scope_guard([]() { ImGui::EndTooltip(); });
		}
		
		auto scoped_item_width(const float v) {
			ImGui::PushItemWidth(v);
		
			return make_scope_guard([]() { ImGui::PopItemWidth(); });
		}

		auto scoped_id(const int v) {
			ImGui::PushID(v);

			return make_scope_guard([]() { ImGui::PopID(); });
		}

		auto scoped_id(void* const v) {
			ImGui::PushID(v);

			return make_scope_guard([]() { ImGui::PopID(); });
		}
		
		auto scoped_tree_node(const char* label) {
			const auto result = ImGui::TreeNode(label);
		
			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::TreePop(); }}));
		
			if (!result) {
				opt = std::nullopt;
			}
		
			return opt;
		}

		auto scoped_menu(const char* label) {
			const auto result = ImGui::BeginMenu(label);

			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::EndMenu(); }}));

			if (!result) {
				opt = std::nullopt;
			}

			return opt;
		}

		auto scoped_main_menu_bar() {
			const auto result = ImGui::BeginMainMenuBar();

			auto opt = std::make_optional(make_scope_guard([result]() { if (result) { ImGui::EndMainMenuBar(); }}));

			if (!result) {
				opt = std::nullopt;
			}

			return opt;
		}

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
	}
}