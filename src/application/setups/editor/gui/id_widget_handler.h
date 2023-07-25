#pragma once
#include "application/setups/editor/gui/widgets/resource_chooser.h"
#include "application/setups/editor/gui/widgets/node_chooser.h"
#include "application/setups/editor/gui/widgets/layer_chooser.h"
#include "application/setups/editor/detail/is_editor_typed_resource.h"
#include "application/setups/editor/detail/is_editor_typed_node.h"

struct id_widget_handler {
	editor_setup& setup;
	const editor_icon_info_in icon_in;
	bool allow_none = true;

	template <class T>
	static constexpr bool handles = 
		std::is_same_v<T, editor_layer_id>
		|| is_editor_typed_resource_id_v<T>
		|| is_editor_typed_node_id_v<T>
	;

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
		bool modified = false;
		const auto none_label = std::string(get_none_label(label));

		static_assert(handles<T>);

		if constexpr(std::is_same_v<T, editor_layer_id>) {
			auto layer = setup.find_layer(property);
			const auto displayed_resource_name = layer ? layer->get_display_name() : none_label;

			thread_local layer_chooser chooser;

			chooser.perform(
				label,
				displayed_resource_name,
				property,
				setup,
				allow_none,
				[&](const editor_layer_id& chosen_id, const auto& chosen_name) {
					result = typesafe_sprintf("Changed layer to %x", chosen_name);
					modified = true;
					property = chosen_id;
				},
				[](auto&&...) { return true; },
				none_label
			);
		}
		else if constexpr(is_editor_typed_resource_id_v<T>) {
			auto resource = setup.find_resource(property);
			const auto displayed_resource_name = resource ? resource->get_display_name() : none_label;

			using R = typename T::target_type;
			thread_local resource_chooser<R> chooser;

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
		}
		else if constexpr(is_editor_typed_node_id_v<T>) {
			auto node = setup.find_node(property);
			const auto displayed_node_name = node ? node->get_display_name() : none_label;

			using R = typename T::target_type;
			thread_local node_chooser<R> chooser;

			chooser.perform(
				label,
				displayed_node_name,
				property,
				setup,
				icon_in,
				allow_none,
				[&](const editor_typed_node_id<R>& chosen_id, const auto& chosen_name) {
					result = typesafe_sprintf("Changed node to %x", chosen_name);
					modified = true;
					property = chosen_id;
				},
				[&](const auto& node) { 
					if (const auto resource = setup.find_resource(node.resource_id)) {
						if (resource->editable.type == area_marker_type::PORTAL) {
							return true;
						}
					}

					return false;
				},
				none_label,
				show_icon
			);
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive if constepxr");
		}

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
