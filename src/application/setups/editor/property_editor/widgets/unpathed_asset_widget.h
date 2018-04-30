#pragma once
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "view/viewables/all_viewables_defs.h"
#include "application/setups/editor/property_editor/tweaker_type.h"

struct unpathed_asset_widget {
	all_viewables_defs& viewables;
	const all_logical_assets& logicals;

private:
	template <class T>
	auto find(const T& id) const {
		if constexpr(is_viewable_asset_v<T>) {
			return get_viewable_pool<T>(viewables).find(id);
		}
		else if constexpr(is_logical_asset_v<T>) {
			return get_logicals_pool<T>(logicals).find(id);
		}
		else {
			static_assert(always_false_v<T>);
		}
	}
public:

	template <class T>
	static constexpr bool handles =
		is_unpathed_asset<T>
	;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T& asset_id
	) const {
		static_assert(handles<T>);

		if (const auto a = find(asset_id)) {
			return typesafe_sprintf("Set %x to %x", formatted_label, a->name);
		}

		return typesafe_sprintf("Unset %x", formatted_label);
	}

	template <class T>
	std::optional<tweaker_type> handle(const std::string& identity_label, T& asset_id) const {
		static_assert(handles<T>);

		using namespace augs::imgui;

		std::optional<tweaker_type> result;

		const auto displayed_str = [&]() {
			if (const auto n = find(asset_id)) {
				return n->name;
			}

			return std::string("None");
		}();

		thread_local ImGuiTextFilter filter;
		thread_local keyboard_acquiring_popup track;

		auto id = scoped_id("Unpathed asset selection");

		if (auto combo = track.standard_combo_facade(filter, identity_label.c_str(), displayed_str.c_str())) {
			if (detail_select_none(asset_id)) {
				return std::make_optional(tweaker_type::DISCRETE);
			}

			auto list_assets_in = [&](const auto& p) {
				p.for_each_object_and_id(
					[&asset_id, &result](const auto& object, const auto& new_id) {
						const auto& name = object.name;
						if (!filter.PassFilter(name.c_str())) {
							return;
						}

						if (ImGui::Selectable(name.c_str())) {
							asset_id = new_id;
							result = tweaker_type::DISCRETE;
						}
					}
				);
			};

			if constexpr(is_viewable_asset_v<T>) {
				list_assets_in(get_viewable_pool<T>(viewables));
			}
			else if constexpr(is_logical_asset_v<T>) {
				list_assets_in(get_logicals_pool<T>(logicals));
			}
			else {
				static_assert(always_false_v<T>);
			}
		}

		return result;
	}
};
