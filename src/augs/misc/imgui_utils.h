#pragma once
#include "augs/math/vec2.h"
#include "augs/templates/string_templates.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/corresponding_field.h"
#include <imgui/imgui.h>

namespace augs {
	namespace imgui {
		namespace detail {
			template <class Target, class T, class F>
			decltype(auto) direct_or_convert(T& into, F&& callback) {
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
		decltype(auto) input_text(const std::string& label, std::string& value, Args&&... args) {
			std::array<char, buffer_size> buf;

			std::copy(value.data(), value.data() + value.size() + 1, buf.data());

			if (ImGui::InputText(label.c_str(), buf.data(), buffer_size, std::forward<Args>(args)...)) {
				value = std::string(buf.data());
				return true;
			}

			return false;
		}

		template <class... Args>
		decltype(auto) checkbox(const std::string& label, bool& into, Args&&... args) {
			ImGui::Checkbox(label.c_str(), &into, std::forward<Args>(args)...);
		}

		template <class T, class... Args>
		decltype(auto) drag(const std::string& label, T& into, Args&&... args) {
			using namespace detail;

			if constexpr(std::is_integral_v<T>) {
				return direct_or_convert<int>(into, [&](int& input) {
					return ImGui::DragInt(label.c_str(), &input, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_floating_point_v<T>) {
				return direct_or_convert<float>(into, [&](float& input) {
					return ImGui::DragFloat(label.c_str(), &input, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_same_v<T, vec2> || std::is_same_v<T, vec2d>) {
				return direct_or_convert<vec2>(into, [&](vec2& input) {
					return ImGui::DragFloat2(label.c_str(), &input.x, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_same_v<T, vec2i> || std::is_same_v<T, vec2u>) {
				return direct_or_convert<vec2i>(into, [&](vec2i& input) {
					return ImGui::DragInt2(label.c_str(), &input.x, std::forward<Args>(args)...);
				});
			}
		}

		template <class T, class... Args>
		decltype(auto) slider(const std::string& label, T& into, Args&&... args) {
			using namespace detail;

			if constexpr(std::is_integral_v<T>) {
				return direct_or_convert<int>(into, [&](int& input) {
					return ImGui::SliderInt(label.c_str(), &input, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_floating_point_v<T>) {
				return direct_or_convert<float>(into, [&](float& input) {
					return ImGui::SliderFloat(label.c_str(), &input, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_same_v<T, vec2> || std::is_same_v<T, vec2d>) {
				return direct_or_convert<vec2>(into, [&](vec2& input) {
					return ImGui::SliderFloat2(label.c_str(), &input.x, std::forward<Args>(args)...);
				});
			}
			else if constexpr(std::is_same_v<T, vec2i> || std::is_same_v<T, vec2u>) {
				return direct_or_convert<vec2i>(into, [&](vec2i& input) {
					return ImGui::SliderInt2(label.c_str(), &input.x, std::forward<Args>(args)...);
				});
			}
		}

		template <class T, class... Args>
		auto enum_combo(const std::string& label, T& into, Args&&... args) {
			thread_local std::vector<char> combo_names;

			if (combo_names.empty()) {
				for_each_enum<T>([](const T e) {
					if (e == T::COUNT) {
						return;
					}

					concatenate(
						combo_names, 
						format_field_name(to_lowercase(std::string(enum_to_string(e))))
					);

					combo_names.push_back('\0');
				});
				
				combo_names.push_back('\0');
			}

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
	}
}

namespace ImGui {
	IMGUI_API bool TabLabels(const char **tabLabels, int tabSize, int &tabIndex, int *tabOrder = NULL);
}