#pragma once
#include <optional>
#include <imgui/imgui.h>

#include "augs/templates/function_traits.h"
#include "augs/templates/corresponding_field.h"
#include "augs/templates/string_templates.h"

#include "augs/math/vec2.h"
#include "augs/misc/scope_guard.h"

namespace augs {
	namespace imgui {
		namespace detail {
			template <class T, class F>
			decltype(auto) direct_or_convert(T& into, F callback) {
				using Target = std::decay_t<argument_of_t<F, 0>>;

				if constexpr(std::is_same_v<T, Target>) {
					return callback(into);
				}
				else {
					Target input = static_cast<Target>(into);
					auto result = callback(input);
					into = static_cast<T>(input);
					return result;
				}
			}
		}

		template <std::size_t buffer_size = 1000, class... Args>
		bool input_text(const std::string& label, std::string& value, Args&&... args) {
			std::array<char, buffer_size> buf;

			std::copy(value.data(), value.data() + value.size() + 1, buf.data());

			if (ImGui::InputText(label.c_str(), buf.data(), buffer_size, std::forward<Args>(args)...)) {
				value = std::string(buf.data());
				return true;
			}

			return false;
		}

		inline bool checkbox(const std::string& label, bool& into) {
			return { ImGui::Checkbox(label.c_str(), &into) };
		}

		template <class T, class... Args>
		decltype(auto) drag(const std::string& label, T& into, Args&&... args) {
			using namespace detail;

			if constexpr(std::is_integral_v<T>) {
				return direct_or_convert(into, [&](int& input) {
					return ImGui::DragInt(label.c_str(), &input, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_floating_point_v<T>) {
				return direct_or_convert(into, [&](float& input) {
					return ImGui::DragFloat(label.c_str(), &input, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_same_v<T, vec2> || std::is_same_v<T, vec2d>) {
				return direct_or_convert(into, [&](vec2& input) {
					return ImGui::DragFloat2(label.c_str(), &input.x, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_same_v<T, vec2i> || std::is_same_v<T, vec2u>) {
				return direct_or_convert(into, [&](vec2i& input) {
					return ImGui::DragInt2(label.c_str(), &input.x, std::forward<Args>(args)...);
				});
			}
		}

		inline bool drag_rect_bounded_vec2(
			const std::string& label, 
			vec2i& into, 
			const float speed, 
			vec2i lower_bound, 
			vec2i upper_bound, 
			const std::string& display_format
		) {
			return { 
				ImGui::DragIntN(
					label.c_str(), 
					&into.x, 
					2, 
					speed, 
					&lower_bound.x,
					&upper_bound.x,
					display_format.c_str()
				) 
			};
		}

		inline void text(const std::string& t) {
			ImGui::Text(t.c_str());
		}

		inline void text(const char* const t) {
			ImGui::Text(t);
		}

		inline void text_tooltip(const std::string& t) {
			ImGui::BeginTooltip();
			text(t);
			ImGui::EndTooltip();
		}

		template <class T, class B, class... Args>
		bool slider(
			const std::string& label, 
			T& into, 
			const B lower_bound,
			const B upper_bound,
			Args&&... args
		) {
			using namespace detail;

			if constexpr(std::is_integral_v<T>) {
				return direct_or_convert(into, [&](int& input) {
					return ImGui::SliderInt(label.c_str(), &input, lower_bound, upper_bound, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_floating_point_v<T>) {
				return direct_or_convert(into, [&](float& input) {
					return ImGui::SliderFloat(label.c_str(), &input, lower_bound, upper_bound, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_same_v<T, ImVec2>) {
				return direct_or_convert(into, [&](ImVec2& input) {
					return ImGui::SliderFloat2(label.c_str(), &input.x, lower_bound, upper_bound, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_same_v<T, vec2> || std::is_same_v<T, vec2d>) {
				return direct_or_convert(into, [&](vec2& input) {
					return ImGui::SliderFloat2(label.c_str(), &input.x, lower_bound, upper_bound, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_same_v<T, vec2i> || std::is_same_v<T, vec2u>) {
				return direct_or_convert(into, [&](vec2i& input) {
					return ImGui::SliderInt2(label.c_str(), &input.x, lower_bound, upper_bound, std::forward<Args>(args)...);
				});
			}
		}

		template <class T, class... Args>
		auto enum_combo(const std::string& label, T& into, Args&&... args) {
			thread_local const auto combo_names = []() {
				std::vector<char> names;

				for_each_enum_except_bounds([&names](const T e) {
					concatenate(
						names,
						format_enum(e)
					);

					names.push_back('\0');
				});
				
				names.push_back('\0');

				return names;
			}();

			auto current = static_cast<int>(into);
			const auto result = ImGui::Combo(label.c_str(), &current, combo_names.data());
			into = static_cast<T>(current);

			return result;
		}

		template <class T>
		auto make_revert_button_lambda(T& edited_config, const T& last_saved_config) {
			return [&](auto& field) {
				const auto& corresponding_last_saved_field = 
					get_corresponding_field(field, edited_config, last_saved_config)
				;

				bool changed = false;

				if (!(field == corresponding_last_saved_field)) {
					ImGui::PushID(reinterpret_cast<void*>(&field)); 
					ImGui::SameLine(); 

					if (ImGui::Button("Revert")) { 
						field = corresponding_last_saved_field;
						changed = true;
					} 
					
					ImGui::PopID(); 
				}
				
				return changed;
			};
		}

		auto scoped_indent() {
			ImGui::Indent();

			return make_scope_guard([]() { ImGui::Unindent(); });
		}

		auto scoped_item_width(const float v) {
			ImGui::PushItemWidth(v);

			return make_scope_guard([]() { ImGui::PopItemWidth(); });
		}

		auto scoped_id(const int v) {
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
	}
}

namespace ImGui {
	IMGUI_API bool TabLabels(const char **tabLabels, int tabSize, int &tabIndex, int *tabOrder = NULL);
}