#pragma once
#include <string>
#include <vector>
#include <optional>
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/string/typesafe_sprintf.h"
#include "application/setups/editor/resources/editor_star_field.h"

template <class E, class EditResourceId>
void edit_star_field_stars(
	std::string& result,
	std::vector<editor_star_field_star>& stars,
	E& edited_resources,
	EditResourceId edit_resource_id
) {
	bool changed = false;
	std::optional<std::size_t> removed_i;

	for (std::size_t i = 0; i < stars.size(); ++i) {
		auto& star = stars[i];
		const auto remove_label = typesafe_sprintf("-##Star%x", i);

		if (ImGui::Button(remove_label.c_str())) {
			removed_i = i;
		}

		ImGui::SameLine();

		if (edit_resource_id(typesafe_sprintf("Star %x", i + 1), star.sprite)) {
			changed = true;
		}

		ImGui::SameLine();

		{
			const auto weight_label = typesafe_sprintf("Weight##Star%x", i);
			auto width = augs::imgui::scoped_item_width(80);

			if (ImGui::DragFloat(weight_label.c_str(), &star.weight, 0.01f, 0.0f, 100.0f)) {
				star.weight = std::max(0.0f, star.weight);
				result = typesafe_sprintf("Changed weight of Star %x in %%x", i + 1);
				changed = true;
			}
		}
	}

	if (removed_i.has_value()) {
		stars.erase(stars.begin() + *removed_i);
		result = typesafe_sprintf("Removed Star %x in %%x", *removed_i + 1);
		changed = true;
	}

	if (stars.size() > MAX_STAR_FIELD_STAR_TYPES) {
		augs::imgui::text_color(typesafe_sprintf("Only the first %x stars are drawn.", MAX_STAR_FIELD_STAR_TYPES), orange);
	}

	if (ImGui::Button("+##AddStar")) {
		stars.push_back(stars.empty() ? editor_star_field_star() : stars.back());
		result = "Added a star in %x";
		changed = true;
	}

	if (changed) {
		for (auto& e : edited_resources) {
			e.after.star_field.value.stars = stars;
		}
	}
}
