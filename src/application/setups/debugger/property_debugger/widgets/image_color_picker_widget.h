#pragma once
#include "view/viewables/all_viewables_declaration.h"
#include "application/setups/debugger/detail/debugger_image_preview.h"
#include "application/setups/debugger/property_debugger/tweaker_type.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_drawers.h"

struct image_color_picker_widget {
	const assets::image_id id;
	const images_in_atlas_map& game_atlas;
	const image_definitions_map& defs;
	debugger_image_preview& current_preview;
	const augs::path_type& project_path;
	const bool nodeize;

	template <class T>
	static constexpr bool handles = is_one_of_v<T, rgba>;

	template <class T>
	static constexpr bool handles_prologue = is_one_of_v<T, std::vector<rgba>>;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T& to
	) const {
		return typesafe_sprintf("Changed %x to %x", formatted_label, to);
	}

	void update_preview() const {
		const auto& p = defs[id].get_source_path();

		if (current_preview.path != p) {
			current_preview.image.from_file(p.resolve(project_path));
			current_preview.path = p;
		}
	}

	template <class T>
	bool handle_prologue(const std::string& identity_label, T& object) const {
		using namespace augs::imgui;

		static_assert(handles<typename T::value_type>);

		update_preview();

		bool result = false;

		auto perform_widget = [&]() {
			const auto zoom = 4;
			const auto& entry = game_atlas.find_or(id).diffuse;

			if (const auto picked = image_color_picker(zoom, entry, current_preview.image)) {
				object.emplace_back(*picked);
				result = true;
			}
		};

		if (nodeize) {
			const auto node_label = "Add multiple...###" + identity_label;

			if (auto node = scoped_tree_node(node_label.c_str())) {
				perform_widget();
			}
		}
		else {
			if (auto combo = scoped_combo(("###AddMultiple" + identity_label).c_str(), "Add multiple...", ImGuiComboFlags_HeightLargest)) {
				perform_widget();
			}
		}

		return result;
	}

	template <class T>
	auto handle(const std::string& identity_label, T& object) const {
		using namespace augs::imgui;

		std::optional<tweaker_type> result;

		update_preview();

		auto iw = scoped_item_width(60);

		auto perform_widget = [&]() {
			const auto zoom = 4;
			const auto& entry = game_atlas.find_or(id).diffuse;

			if (const auto picked = image_color_picker(zoom, entry, current_preview.image)) {
				object = *picked;
				result = tweaker_type::DISCRETE;
			}
		};

		auto perform_standard_widget = [&]() {
			/* Perform the standard widget for manual tweaking */

			ImGui::SameLine();

			auto iw = scoped_item_width(-1);

			if (color_edit(identity_label, object)) {
				result = tweaker_type::CONTINUOUS;
			}
		};

		if (nodeize) {
			const auto node_label = "" + identity_label;

			auto node = scoped_tree_node(node_label.c_str());

			perform_standard_widget();

			if (node) {
				perform_widget();
			}
		}
		else {
			if (auto combo = scoped_combo((identity_label + "Picker").c_str(), "Pick", ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoPreview)) {
				perform_widget();
			}

			perform_standard_widget();
		}

		return result;
	}
};

