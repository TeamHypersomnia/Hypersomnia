#pragma once
#include "application/setups/editor/gui/widgets/resource_chooser.h"

template <class T>
struct is_editor_typed_resource_id : std::false_type {};

template <class T>
struct is_editor_typed_resource_id<editor_typed_resource_id<T>> : std::true_type {};

template <class T>
static constexpr bool is_editor_typed_resource_id_v = is_editor_typed_resource_id<T>::value;

struct id_widget_handler {
	editor_setup& setup;
	const editor_icon_info_in icon_in;
	bool allow_none = true;

	template <class T>
	static constexpr bool handles = is_editor_typed_resource_id_v<T>;

	auto get_none_label(const std::string& label) const {
		if (label == "Footstep sound") {
			return "(Default)";
		}

		return "(None)";
	}

	template <class T>
	bool handle(
		std::string& result,
		const std::string& label,
		T& property,
		const bool show_icon = true
	) {
		static_assert(handles<T>);

		auto resource = setup.find_resource(property);

		const auto none_label = std::string(get_none_label(label));
		const auto displayed_resource_name = resource ? resource->get_display_name() : none_label;

		using R = typename T::target_type;
		thread_local resource_chooser<R> chooser;

		bool modified = false;

		chooser.perform(
			label,
			displayed_resource_name,
			property,
			setup,
			icon_in,
			allow_none,
			[&](const editor_typed_resource_id<R>& chosen_id, const auto& chosen_name) {
				result = typesafe_sprintf("Changed resource to %x", chosen_name);
				modified = true;
				property = chosen_id;
			},
			none_label,
			show_icon
		);

		if (allow_none) {
			if (property.is_set()) {
				ImGui::SameLine();

				auto scope = augs::imgui::scoped_id(label.c_str());

				if (ImGui::Button("Clear")) {
					result = typesafe_sprintf("Cleared %x", label);
					property = {};
					modified = true;
				}
			}
		}

		return modified;
	}
};
