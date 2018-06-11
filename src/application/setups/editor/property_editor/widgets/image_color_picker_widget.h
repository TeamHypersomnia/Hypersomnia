#pragma once
#include "view/viewables/all_viewables_declarations.h"
#include "application/setups/editor/detail/editor_image_preview.h"
#include "application/setups/editor/property_editor/tweaker_type.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_drawers.h"

struct image_color_picker_widget {
	const assets::image_id id;
	const images_in_atlas_map& game_atlas;
	const image_definitions_map& defs;
	editor_image_preview& current_preview;
	const augs::path_type& project_path;

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

		if (auto combo = scoped_combo(("###AddMultiple" + identity_label).c_str(), "Add multiple...", ImGuiComboFlags_HeightLargest)) {
			const auto zoom = 4;
			const auto& entry = game_atlas.at(id).diffuse;

			if (const auto picked = image_color_picker(zoom, entry, current_preview.image)) {
				object.emplace_back(*picked);
				return true;
			}
		}

		return false;
	}

	template <class T>
	auto handle(const std::string& identity_label, T& object) const {
		using namespace augs::imgui;

		std::optional<tweaker_type> result;

		update_preview();

		auto iw = scoped_item_width(60);

		if (auto combo = scoped_combo((identity_label + "Picker").c_str(), "Pick", ImGuiComboFlags_HeightLargest)) {
			const auto zoom = 4;
			const auto& entry = game_atlas.at(id).diffuse;

			if (const auto picked = image_color_picker(zoom, entry, current_preview.image)) {
				object = *picked;
				result = tweaker_type::DISCRETE;
			}
		}

		if (result) {
			return result;
		}

		/* Perform the standard widget for manual tweaking */

		{
			ImGui::SameLine();

			auto iw = scoped_item_width(-1);

			if (color_edit(identity_label, object)) {
				result = tweaker_type::CONTINUOUS;
			}
		}

		return result;
	}
};

