#pragma once
#include "view/viewables/all_viewables_defs.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "application/setups/debugger/property_debugger/tweaker_type.h"
#include "application/setups/debugger/property_debugger/widgets/asset_path_chooser.h"
#include "application/setups/debugger/property_debugger/property_debugger_settings.h"
#include "view/asset_funcs.h"

struct source_path_widget {
	all_viewables_defs& defs;
	const augs::path_type& project_path;
	const property_debugger_settings& settings;
	const bool disabled;

	template <class T>
	static constexpr bool handles = is_maybe_official_path<T>::value;

	template <class T>
	static constexpr bool handles_prologue = false;

	template <class T>
	auto describe_changed(
		const std::string& /* formatted_label */,
		const T& to
	) const {
		return typesafe_sprintf("Changed %x path to %x", assets::get_label<typename T::id_type>(), to.path);
	}

	template <class T>
	std::optional<tweaker_type> handle(const std::string& identity_label, T& object) const {
		using id_type = typename T::id_type;
		auto& definitions = get_viewable_pool<id_type>(defs);

		bool modified = false;

		auto scope = ::maybe_disabled_cols(settings, disabled);

		thread_local asset_path_chooser<id_type> chooser;

		chooser.perform(
			identity_label,
			object,
			project_path,
			[&](const auto& chosen_path) {
				object = chosen_path;
				modified = true;
			},
			[&](const auto& candidate_path) {
				if (const auto asset_id = ::find_asset_id_by_path(candidate_path, definitions)) {
					return false;
				}

				return true;
			},
			"Already tracked paths"
		);

		if (modified) {
			return tweaker_type::DISCRETE;
		}

		return std::nullopt;
	}
};
