#define INCLUDE_TYPES_IN 1

#include "application/setups/debugger/gui/debugger_unpathed_asset_gui.h"

#if BUILD_PROPERTY_DEBUGGER

#include "augs/string/string_templates.h"

#include "augs/templates/introspection_utils/field_name_tracker.h"
#include "augs/templates/introspection_utils/on_dynamic_content.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/path_tree_structs.h"

#include "game/organization/for_each_entity_type.h"

#include "application/setups/debugger/debugger_folder.h"
#include "application/intercosm.h"

#include "application/setups/debugger/detail/read_write_defaults_buttons.h"
#include "application/setups/debugger/detail/other_styles.h"
#include "application/setups/debugger/detail/format_struct_name.h"
#include "application/setups/debugger/property_debugger/general_edit_properties.h"
#include "application/setups/debugger/detail/find_locations_that_use.h"
#include "application/setups/debugger/detail/checkbox_selection.h"
#include "application/setups/debugger/detail/duplicate_delete_buttons.h"

#include "application/setups/debugger/property_debugger/widgets/frames_prologue_widget.h"
#include "application/setups/debugger/property_debugger/widgets/asset_sane_default_provider.h"
#include "application/setups/debugger/property_debugger/widgets/pathed_asset_widget.h"
#include "application/setups/debugger/property_debugger/special_widgets.h"

#include "application/setups/debugger/property_debugger/compare_all_fields_to.h"
#include "augs/readwrite/byte_readwrite.h"

#include "augs/templates/list_utils.h"
#include "application/setups/debugger/detail/rewrite_last_change.h"

template <class id_type>
struct unpathed_asset_entry {
	std::string name;
	id_type id;

	std::vector<std::string> using_locations;

	bool used() const {
		return using_locations.size() > 0;
	}

	auto tie() const {
		return std::tie(name, id.indirection_index, id.version);
	}

	bool operator<(const unpathed_asset_entry& b) const {
		return tie() < b.tie();
	}
};

#endif

template <class asset_id_type>
void debugger_unpathed_asset_gui<asset_id_type>::perform(
	const property_debugger_settings& settings,
	const images_in_atlas_map& game_atlas,
   	const debugger_command_input cmd_in
) {
#if BUILD_PROPERTY_DEBUGGER
	using namespace augs::imgui;

	auto window = base::make_scoped_window();

	if (!window) {
		return;
	}

	auto& folder = cmd_in.folder;

	checkbox("Orphaned", show_orphaned);

	ImGui::SameLine();
	checkbox("Using locations", show_using_locations);

	constexpr bool is_animation_type = std::is_same_v<asset_id_type, assets::plain_animation_id>;

	if constexpr(is_animation_type) {
		ImGui::SameLine();
		checkbox("Preview animations", preview_animations);
	}

	ImGui::Separator();

	{
		const auto button_label = "Create";

		if (ImGui::Button(button_label)) {
			create_unpathed_asset_id_command<asset_id_type> cmd;
			post_debugger_command(cmd_in, std::move(cmd));
			show_orphaned = true;
		}

		ImGui::Separator();
	}

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	base::acquire_keyboard_once();

	using asset_entry_type = unpathed_asset_entry<asset_id_type>;

	thread_local std::vector<asset_entry_type> orphaned_assets;
	thread_local std::vector<asset_entry_type> used_assets;

	orphaned_assets.clear();
	used_assets.clear();

	auto is_ticked = [this](const auto& p) {
		return found_in(ticked_assets, p.id);
	};

	auto get_all_ticked_and_existing = [&](auto in_range) {
		erase_if(in_range, [&] (const auto& candidate) { return !is_ticked(candidate); } );
		return in_range;
	};

	auto& definitions = get_asset_pool<asset_id_type>(cmd_in);
	using def_type = typename remove_cref<decltype(definitions)>::mapped_type;

	for_each_id_and_object(definitions,
		[&](const auto id, const def_type& object) mutable {
			asset_entry_type new_entry;
			new_entry.id = id;
			new_entry.name = get_displayed_name(object, get_asset_pool<assets::image_id>(cmd_in));

			auto& commanded = *folder.commanded;

			const auto in_locations = candidate_id_locations {
				commanded.work.world, 
				commanded.work.viewables,
				commanded.rulesets
			};

			find_locations_that_use(id, in_locations, [&](const auto& location) {
				new_entry.using_locations.push_back(location);
			});

			(new_entry.used() ? used_assets : orphaned_assets).emplace_back(std::move(new_entry));
		}
	);

	{
		auto for_each_range = [](auto callback) {
			callback(orphaned_assets);
			callback(used_assets);
		};

		auto prepare = [&](auto& range) {
			erase_if(range, [&](const auto& entry) {
				const auto& displayed_name = entry.name;

				if (!filter.PassFilter(displayed_name.c_str())) {
					return true;
				}

				return false;
			});

			sort_range(range);
		};

		for_each_range(prepare);
	}

	auto files_view = scoped_child("Assets view");

	const auto num_cols = 2 + (show_using_locations ? 1 : 0);

	ImGui::Columns(num_cols);

	auto do_asset = [&](const auto& asset_entry, const auto& ticked_in_range, const auto& ticked_ids) {
		(void)ticked_in_range;

		const auto id = asset_entry.id;
		auto scope = scoped_id(id);
		const auto& displayed_name = asset_entry.name;

		const auto is_current_ticked = is_ticked(asset_entry);

		const auto flags = do_selection_checkbox(ticked_assets, id, is_current_ticked, id);

		const auto result = duplicate_delete_buttons<
			duplicate_asset_command<asset_id_type>,
			forget_asset_id_command<asset_id_type>
		> (cmd_in, id, settings, "", !asset_entry.used());

		if (result == dd_buttons_result::DUPLICATE) {
			/* 
				Show orphaned assets if they aren't already. 
				Otherwise the author would have no feedback for pressing duplicate button.
			*/
			show_orphaned = true;
		}
		else if (result == dd_buttons_result::DELETE) {
			/* It has just been deleted. */
			return;
		}

		const auto node = scoped_tree_node_ex(displayed_name + "###Node", flags);

		constexpr bool is_animation_type = std::is_same_v<asset_id_type, assets::plain_animation_id>;

		if constexpr(is_animation_type) {
			ImGui::SameLine();
			text_disabled(typesafe_sprintf("(%x frames)", definitions[id].frames.size()));
		}

		next_columns(2);

		if (show_using_locations) {
			if (asset_entry.used()) {
				const auto& using_locations = asset_entry.using_locations;

				if (auto node = scoped_tree_node(typesafe_sprintf("%x locations###locations", using_locations.size()).c_str())) {
					for (const auto& l : using_locations) {
						text(l);
					}
				}
			}
			else {
				text_disabled("(Nowhere)");
			}

			ImGui::NextColumn();
		}

		if (node) {
			auto sc = scoped_indent();

			const auto property_location = typesafe_sprintf(" (in %x)", displayed_name);

			using cmd_type = change_asset_property_command<asset_id_type>;
			using field_type_id = property_field_type_id_t<cmd_type>;

			auto post_new_change = [&](
				const auto& description,
				const auto field_id,
				const auto& new_content
			) {
				cmd_type cmd;

				if (is_current_ticked) {
					cmd.affected_assets = ticked_ids;
				}
				else {
					cmd.affected_assets = { id };
				}

				cmd.property_id.field = field_id;
				augs::assign_bytes(cmd.value_after_change, new_content);
				cmd.built_description = description + property_location;

				post_debugger_command(cmd_in, std::move(cmd));
			};

			auto rewrite_last_change = make_rewrite_last_change<cmd_type>(
				property_location,
				cmd_in
			);

			auto prop_in = property_debugger_input { settings, property_debugger_data };
			const auto& project_path = cmd_in.folder.current_path;
			auto& viewables = folder.commanded->work.viewables;

			using frames_widget_type = 
				std::conditional_t<
					is_animation_type,
					frames_prologue_widget,
					default_widget_provider
				>
			;

			general_edit_properties<field_type_id>(
				prop_in,
				definitions[id],
				post_new_change,
				rewrite_last_change,
				[&](const auto& first, const auto& field_id) {
					if (!is_current_ticked) {
						return true;
					}

					return compare_all_fields_to(
						first,
						asset_property_id<asset_id_type> { field_id }, 
						cmd_in, 
						ticked_ids
					);
				},
				special_widgets(
					pathed_asset_widget { viewables, project_path, cmd_in },
					frames_widget_type { prop_in, cmd_in, id, project_path, ticked_ids, is_current_ticked, game_atlas, preview_animations }
				),
				asset_sane_default_provider { viewables },
				num_cols - 2
			);
		}
	};

	int s = 0;
	
	auto do_section = [&](
		const auto& entries,
		const std::array<std::string, 2> labels,
		const std::optional<rgba> color = std::nullopt
	) {
		if (entries.empty()) {
			return;
		}

		{
			do_tick_all_checkbox(
				settings,
				ticked_assets,
				[&entries](auto callback) {
					for (const auto& p : entries) {
						callback(p.id);
					}
				},
				s++
			);

			if (color) {
				text_color(labels[0], *color);
			}
			else {
				text_disabled(labels[0]);
			}
		}

		ImGui::NextColumn();
		text_disabled("Properties");

		ImGui::NextColumn();

		if (show_using_locations) {
			text_disabled(labels[1]);
			ImGui::NextColumn();
		}

		ImGui::Separator();

		const auto ticked_and_existing = get_all_ticked_and_existing(entries);

		thread_local std::vector<asset_id_type> ticked_ids;
		ticked_ids.clear();

		for (const auto& p : ticked_and_existing) {
			ticked_ids.push_back(p.id);
		}

		for (const auto& p : entries) {
			do_asset(p, ticked_and_existing, ticked_ids);
		}

		ImGui::Separator();
	};

	const auto label = uncapitalize_first(format_field_name(get_type_name_strip_namespace<def_type>()));

	if (show_orphaned) {
		do_section(
			orphaned_assets,
			{ typesafe_sprintf("Orphaned %xs", label), "Used at" }
		);
	}

	do_section(
		used_assets,
		{ capitalize_first(std::string(label)) + "s", "Used at" }
	);
#else
	(void)settings;
	(void)game_atlas;
	(void)cmd_in;
#endif
}

template struct debugger_unpathed_asset_gui<assets::plain_animation_id>;
template struct debugger_unpathed_asset_gui<assets::particle_effect_id>;
