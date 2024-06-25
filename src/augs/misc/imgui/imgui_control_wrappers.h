#pragma once
#include "3rdparty/imgui/imgui.h"
#include "augs/string/typesafe_sprintf.h"

#include "augs/templates/folded_finders.h"
#include "augs/templates/identity_templates.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/traits/function_traits.h"
#include "augs/templates/corresponding_field.h"
#include "augs/string/string_templates.h"
#include "augs/string/format_enum.h"

#include "augs/misc/bound.h"

#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "augs/graphics/rgba.h"

#include "augs/misc/constant_size_string.h"
#include "augs/misc/imgui/imgui_controls.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/filesystem/path.h"

namespace augs {
	namespace imgui {
		namespace detail {
			template <class T, class F>
			decltype(auto) direct_or_convert(T& into, F callback) {
				using Target = remove_cref<argument_t<F, 0>>;

				if constexpr(std::is_same_v<T, Target>) {
					return callback(into);
				}
				else if constexpr(has_first_and_second_types_v<T>){
					auto input = Target(into.first, into.second);
					auto result = callback(input);
					into = T(input.first, input.second);
					return result;
				}
				else {
					Target input = static_cast<Target>(into);
					auto result = callback(input);
					into = static_cast<T>(input);
					return result;
				}
			}
		}

		inline void next_columns(int times) {
			while (times--) {
				ImGui::NextColumn();
			}
		}

		template <std::size_t buffer_size, class S, class... Args>
		bool input_text(std::array<char, buffer_size>& buf, const std::string& label, S& value, Args&&... args) {
			static_assert(buffer_size > 1, "Array length must be at least 2 to hold the null character");

			const auto considered_n = std::min(buffer_size - 1, value.size());
			std::copy(value.data(), value.data() + considered_n + 1, buf.data());

			if (ImGui::InputText(label.c_str(), buf.data(), buffer_size, std::forward<Args>(args)...)) {
				value = std::string(buf.data());
				return true;
			}

			return false;
		}

		template <std::size_t buffer_size, class S, class... Args>
		bool input_text_with_hint(std::array<char, buffer_size>& buf, const std::string& label, const char* hint, S& value, Args&&... args) {
			static_assert(buffer_size > 1, "Array length must be at least 2 to hold the null character");

			const auto considered_n = std::min(buffer_size - 1, value.size());
			std::copy(value.data(), value.data() + considered_n + 1, buf.data());

			if (ImGui::InputTextWithHint(label.c_str(), hint, buf.data(), buffer_size, std::forward<Args>(args)...)) {
				value = std::string(buf.data());
				return true;
			}

			return false;
		}

		template <unsigned buffer_size, class... Args>
		bool input_text_with_hint(const std::string& label, const char* hint, constant_size_string<buffer_size>& value, Args&&... args) {
			typename remove_cref<decltype(value)>::array_type buf;
			return input_text_with_hint(buf, label, hint, value, std::forward<Args>(args)...);
		}

		template <std::size_t buffer_size = 1000, class... Args>
		bool input_text(const std::string& label, std::string& value, Args&&... args) {
			std::array<char, buffer_size> buf;
			return input_text(buf, label, value, std::forward<Args>(args)...);
		}

		template <std::size_t buffer_size = 1000, class... Args>
		bool input_text(const std::string& label, path_type& value, Args&&... args) {
			std::array<char, buffer_size> buf;

			std::string s = string_windows_friendly(value);

			if (input_text(buf, label, s, std::forward<Args>(args)...)) {
				value = path_windows_friendly(s);
				return true;
			}

			return false;
		}

		template <unsigned buffer_size, class... Args>
		bool input_text(const std::string& label, constant_size_string<buffer_size>& value, Args&&... args) {
			typename remove_cref<decltype(value)>::array_type buf;
			return input_text(buf, label, value, std::forward<Args>(args)...);
		}

		template <std::size_t buffer_size = 1000, class... Args>
		bool input_multiline_text(const std::string& label, std::string& value, const unsigned num_lines, Args&&... args) {
			std::array<char, buffer_size> buf;

			std::copy(value.data(), value.data() + value.size() + 1, buf.data());

			if (ImGui::InputTextMultiline(label.c_str(), buf.data(), buffer_size, ImVec2(0.f, ImGui::GetTextLineHeight() * num_lines), std::forward<Args>(args)...)) {
				value = std::string(buf.data());
				return true;
			}

			return false;
		}

		template <unsigned buffer_size, class... Args>
		bool input_multiline_text(const std::string& label, constant_size_string<buffer_size>& value, const unsigned num_lines, Args&&... args) {
			std::array<char, buffer_size> buf;

			std::copy(value.data(), value.data() + value.size() + 1, buf.data());

			if (ImGui::InputTextMultiline(label.c_str(), buf.data(), buffer_size, ImVec2(0.f, ImGui::GetTextLineHeight() * num_lines), std::forward<Args>(args)...)) {
				value = std::string(buf.data());
				return true;
			}

			return false;
		}

		inline bool checkbox(const std::string& label, bool& into) {
			return ImGui::Checkbox(label.c_str(), &into);
		}

		template <class T>
		void fix_integral_bounds(T& v_min, T& v_max) {
			if constexpr(std::is_integral_v<T>) {
				if (v_max == v_min) {
					/* Fix default bounds */

					if constexpr(std::is_same_v<T, int32_t>) {
						/* Correct case, do nothing */
					}
					else if constexpr(std::is_same_v<T, uint32_t>) {
						/* Prevent ImGui from setting negative values */
						v_min = 0;
						v_max = static_cast<T>(std::numeric_limits<int32_t>::max());
					}
					else if constexpr(
						std::is_same_v<T, uint16_t>
						|| std::is_same_v<T, uint8_t>
					) {
						v_min = 0;
						v_max = std::numeric_limits<T>::max();
					}
					else if constexpr(
						std::is_same_v<T, int16_t>
						|| std::is_same_v<T, int8_t>
						|| std::is_same_v<T, char>
					) {
						v_min = std::numeric_limits<T>::min();
						v_max = std::numeric_limits<T>::max();
					}
					else {
						static_assert(always_false_v<T>, "Unsupported integer type.");
					}
				}
			}
		}

		template <class T>
		decltype(auto) drag_vec2(
			const std::string& label,
			basic_vec2<T>& into,
			const float speed = 1.f,
			T v_min = static_cast<T>(0),
			T v_max = static_cast<T>(0),
			const char* const fmt = nullptr
		) {
			using namespace detail;

			fix_integral_bounds(v_min, v_max);

			if constexpr(std::is_integral_v<T>) {
				return direct_or_convert(into, [&](vec2i& input) {
					return ImGui::DragInt2(label.c_str(), &input.x, speed, v_min, v_max, fmt);
				});
			}
			else if constexpr(std::is_floating_point_v<T>) {
				return direct_or_convert(into, [&](vec2& input) {
					return ImGui::DragFloat2(label.c_str(), &input.x, speed, v_min, v_max, fmt);
				});
			}
			else {
				static_assert(always_false_v<T>, "Unsupported type. Use a suitable wrapper.");
			}
		}

		template <class T>
		decltype(auto) drag_transform(
			const std::string& label_s,
			basic_transform<T>& into,
			const float speed = 1.f,
			T v_min = static_cast<T>(0),
			T v_max = static_cast<T>(0),
			const char* const fmt = nullptr
		) {
			using namespace detail;
			using namespace ImGui;

			ImGuiDataType pos_data_type;

			if constexpr(std::is_same_v<T, real32>) {
				pos_data_type = ImGuiDataType_Float;
			}
			else if constexpr(std::is_same_v<T, int>) {
				pos_data_type = ImGuiDataType_S32;
			}
			else {
				static_assert(always_false_v<T>);
			}

			ImGuiContext& g = *GImGui;
			bool value_changed = false;
			BeginGroup();

			const auto* label = label_s.c_str();
			PushID(label);
			PushMultiItemsWidths(3, CalcItemWidth());

			PushID(0);
			value_changed |= DragScalar("##v", pos_data_type, &into.pos.x, speed, &v_min, &v_max, fmt);
			SameLine(0, g.Style.ItemInnerSpacing.x);
			PopID();
			PopItemWidth();

			PushID(1);
			value_changed |= DragScalar("##v", pos_data_type, &into.pos.y, speed, &v_min, &v_max, fmt);
			SameLine(0, g.Style.ItemInnerSpacing.x);
			PopID();
			PopItemWidth();

			PushID(2);

			const float dmin = -180.f;
			const float dmax = 180.f;

			value_changed |= DragScalar("##v", ImGuiDataType_Float, &into.rotation, speed, &dmin, &dmax, fmt);
			SameLine(0, g.Style.ItemInnerSpacing.x);
			PopID();
			PopItemWidth();

			TextUnformatted(label, FindRenderedTextEnd(label));
			PopID();
			EndGroup();

			return value_changed;
		}

		template <class T>
		decltype(auto) drag_bound(
			const std::string& label,
			bound<T>& into,
			const float speed = 1.f,
			T v_min = static_cast<T>(0),
			T v_max = static_cast<T>(0),
			const char* const fmt = nullptr
		) {
			using namespace detail;

			fix_integral_bounds(v_min, v_max);

			if constexpr(std::is_integral_v<T>) {
				return direct_or_convert(into, [&](bound<int>& input) {
					return ImGui::DragIntRange2(label.c_str(), std::addressof(input.first), std::addressof(input.second), speed, v_min, v_max, fmt);
				});
			}
			else if constexpr(std::is_floating_point_v<T>) {
				return direct_or_convert(into, [&](bound<float>& input) {
					return ImGui::DragFloatRange2(label.c_str(), std::addressof(input.first), std::addressof(input.second), speed, v_min, v_max, fmt);
				});
			}
			else {
				static_assert(always_false_v<T>, "Unsupported type. Use a suitable wrapper.");
			}
		}

		template <class T>
		decltype(auto) drag(
			const std::string& label,
			T& into,
			const float speed = 1.f,
			T v_min = static_cast<T>(0),
			T v_max = static_cast<T>(0),
			const char* const fmt = nullptr
		) {
			using namespace detail;

			fix_integral_bounds(v_min, v_max);

			if constexpr(std::is_integral_v<T>) {
				return direct_or_convert(into, [&](int& input) {
					return ImGui::DragInt(label.c_str(), &input, speed, v_min, v_max, fmt);
				});
			}
			else if constexpr(std::is_floating_point_v<T>) {
				return direct_or_convert(into, [&](float& input) {
					return ImGui::DragFloat(label.c_str(), &input, speed, v_min, v_max, fmt);
				});
			}
			else {
				static_assert(always_false_v<T>, "Unsupported type. Use a suitable wrapper.");
			}
		}

		inline bool drag_rect_bounded_vec2i(
			const std::string& label, 
			vec2i& into, 
			const float speed, 
			vec2i lower_bound, 
			vec2i upper_bound, 
			const std::string& display_format = "%.0f"
		) {
			return 
				ImGui::DragIntN(
					label.c_str(), 
					&into.x, 
					2, 
					speed, 
					&lower_bound.x,
					&upper_bound.x,
					display_format.c_str()
				)
			;
		}

		inline bool color_edit(
			const std::string& label,
			rgba& into,
			const ImGuiColorEditFlags flags = 0
		) {
			ImVec4 input = into;
			const bool result = { ImGui::ColorEdit4(label.c_str(), reinterpret_cast<float*>(&input), flags) };
			into = rgba(input);
			return result;
		}

		inline bool color_picker(
			const std::string& label,
			rgba& into,
			const ImGuiColorEditFlags flags = 0
		) {
			ImVec4 input = into;
			const bool result = { ImGui::ColorPicker4(label.c_str(), reinterpret_cast<float*>(&input), flags) };
			into = rgba(input);
			return result;
		}

		template <class... Args>
		void text(const std::string& format, Args&&... args) {
			if constexpr(sizeof...(Args) == 0) {
				ImGui::TextUnformatted(format.c_str());
			}
			else {
				text(typesafe_sprintf(format, std::forward<Args>(args)...));
			}
		}

		inline void centered_text(const std::string& str) {
			const auto s = ImGui::CalcTextSize(str.c_str(), nullptr, true);
			const auto w = ImGui::GetWindowWidth();
			ImGui::SetCursorPosX(w / 2 - s.x / 2);
			text(str);
		}

		inline void text(const char* const t) {
			ImGui::TextUnformatted(t);
		}

		inline void text_color(const std::string& t, const rgba& r) {
			auto scope = scoped_text_color(r);
			text(t);
		}

		inline void text_color(const std::string& t, const ImVec4& r) {
			return text_color(t, rgba(r));
		}

		template <class... T>
		void text_tooltip(T&&... args) {
			auto scope = scoped_tooltip();
			text(std::forward<T>(args)...);
		}

		template <class... T>
		void tooltip_on_hover(T&&... args) {
			if (ImGui::IsItemHovered()) {
				text_tooltip(std::forward<T>(args)...);
			}
		}

		inline void text_disabled(const std::string& t) {
			text_color(t, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		}

		inline void next_column_text(const std::string& tx = "") {
			ImGui::NextColumn();

			if (tx.size() > 0) {
				augs::imgui::text(tx);
			}

			ImGui::NextColumn();
		};

		inline void next_column_text_disabled(const std::string& tx = "") {
			ImGui::NextColumn();

			if (tx.size() > 0) {
				augs::imgui::text_disabled(tx);
			}

			ImGui::NextColumn();
		};

		template <class T>
		bool slider(
			const std::string& label, 
			T& into, 
			T lower_bound,
			T upper_bound,
			const char* const fmt = nullptr
		) {
			using namespace detail;

			fix_integral_bounds(lower_bound, upper_bound);

			if constexpr(std::is_integral_v<T>) {
				return direct_or_convert(into, [&](int& input) {
					return ImGui::SliderInt(label.c_str(), &input, lower_bound, upper_bound, fmt);
				});
			}
			else if constexpr(std::is_floating_point_v<T>) {
				return direct_or_convert(into, [&](float& input) {
					return ImGui::SliderFloat(label.c_str(), &input, lower_bound, upper_bound, fmt);
				});
			}
			else if constexpr(std::is_same_v<T, ImVec2>) {
				return direct_or_convert(into, [&](ImVec2& input) {
					return ImGui::SliderFloat2(label.c_str(), &input.x, lower_bound, upper_bound, fmt);
				});
			}
			else if constexpr(std::is_same_v<T, vec2> || std::is_same_v<T, vec2d>) {
				return direct_or_convert(into, [&](vec2& input) {
					return ImGui::SliderFloat2(label.c_str(), &input.x, lower_bound, upper_bound, fmt);
				});
			}
			else if constexpr(std::is_same_v<T, vec2i> || std::is_same_v<T, vec2u>) {
				return direct_or_convert(into, [&](vec2i& input) {
					return ImGui::SliderInt2(label.c_str(), &input.x, lower_bound, upper_bound, fmt);
				});
			}
		}

		template <class T>
		auto make_revert_button_lambda(T& edited_config, const T& last_saved_config) {
			return [&](auto& field) {
				const auto& corresponding_last_saved_field = 
					get_corresponding_field(field, edited_config, last_saved_config)
				;

				bool changed = false;

				if (!(field == corresponding_last_saved_field)) {
					auto id = scoped_id(reinterpret_cast<void*>(&field)); 
					ImGui::SameLine(); 

					if (ImGui::Button("Revert")) { 
						field = corresponding_last_saved_field;
						changed = true;
					} 
				}

				return changed;
			};
		}
	}
}