#pragma once
#include "game/transcendental/cosmos.h"
#include "application/setups/editor/property_editor/widgets/keyboard_acquiring_popup.h"
#include "application/setups/editor/detail/maybe_different_colors.h"

struct flavour_widget {
	const cosmos& cosm;

	template <class T>
	static constexpr bool handles =
		std::is_convertible_v<T, entity_flavour_id>
	;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T& to
	) const {
		static_assert(handles<T>);

		if (to == T()) {
			return typesafe_sprintf("Unset %x", formatted_label);
		}

		return typesafe_sprintf("Set %x to %x", formatted_label, cosm.find_flavour_name(to));
	}

	template <class T>
	auto handle(const std::string& identity_label, T& flavour_id) {
		static_assert(handles<T>);

		using namespace augs::imgui;

		std::optional<tweaker_type> result;

		const auto displayed_str = [&]() {
			if (const auto n = cosm.find_flavour_name(flavour_id)) {
				return *n;
			}

			return std::string("No flavour selected");
		}();

		thread_local keyboard_acquiring_popup track;

		if (auto combo = scoped_combo(identity_label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
			auto id = scoped_id("Flavour selection");

			thread_local ImGuiTextFilter filter;
			filter.Draw();

			track.check_opened_first_time();

			if (const bool acquire_keyboard = track.pop_acquire_keyboard()) {
				ImGui::SetKeyboardFocusHere();
			}

			auto list_flavours_of_type = [&](auto e) {
				using E = decltype(e);

				const auto entity_type_label = format_field_name(get_type_name<E>());

				if (cosm.get_flavours_count<E>()) {
					text(entity_type_label);
				}
				else {
					text_disabled(entity_type_label);
				}

				auto scope = scoped_indent();

				cosm.for_each_id_and_flavour<E>(
					[&flavour_id, &result](const auto& new_id, const auto& flavour) {
						const auto& name = flavour.get_name();

						if (!filter.PassFilter(name.c_str())) {
							return;
						}

						if (ImGui::Selectable(name.c_str())) {
							flavour_id = new_id;
							result = tweaker_type::DISCRETE;
						}
					}
				);
			};

			if constexpr(is_typed_flavour_id_v<T>) {
				list_flavours_of_type(entity_type_of<T>());
			}
			else if constexpr(is_constrained_flavour_id_v<T>) {
				for_each_type_in_list<typename T::matching_types>([&](auto e) {
					list_flavours_of_type(e);
				});
			}
		}
		else {
			track.mark_not_opened();
		}

		return result;
	}
};
