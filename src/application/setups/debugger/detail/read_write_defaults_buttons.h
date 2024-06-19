#pragma once
#include "augs/filesystem/file.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

#include "view/load_meta_json.h"

#include "application/setups/debugger/property_debugger/property_debugger_settings.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "augs/readwrite/to_bytes.h"

template <class Defs, class I, class T>
void read_write_defaults_buttons(
	const property_debugger_settings& settings,
	const debugger_command_input cmd_in,
	const Defs& definitions,
	const I id,
	const bool is_current_ticked,
	const T& ticked_in_range
) {
	using namespace augs::imgui;
	using cmd_type = change_asset_property_command<I>;
	using field_type_id = property_field_type_id_t<cmd_type>;

	using D = remove_cref<decltype(definitions[id])>;
	const D& definition_object = definitions[id];

	const auto& project_path = cmd_in.folder.current_path;
	const auto& view = asset_definition_view<const D>(project_path, definition_object);
	const auto resolved = view.get_resolved_source_path();

	{
		const auto meta_json_path = get_meta_json_path(resolved);
		auto cols = maybe_disabled_cols(settings, !augs::exists(meta_json_path));

		if (ImGui::Button("Read defaults")) {
			try {
				decltype(definition_object.meta) new_meta;
				load_meta_json_if_exists(new_meta, resolved);

				cmd_type cmd;

				cmd.affected_assets = { id };
				cmd.property_id.field = make_field_address<field_type_id>(definition_object, definition_object.meta);
				augs::assign_bytes(cmd.value_after_change, new_meta);
				cmd.built_description = "Read defaults from " + augs::filename_first(meta_json_path);

				post_debugger_command(cmd_in, std::move(cmd));
			}
			catch (...) {

			}
		}

		if (ImGui::IsItemHovered()) {
			text_tooltip("Read defaults from:\n%x", meta_json_path);
		}

		ImGui::SameLine();
	}

	{
		if (ImGui::Button("Write defaults")) {
			if (!is_current_ticked) {
				save_meta_json(definition_object.meta, resolved);
			}
			else {
				for (const auto& t : ticked_in_range) {
					const auto& ticked_definition_object = definitions[t.id];
					const auto& ticked_view = asset_definition_view<const D>(project_path, ticked_definition_object);
					const auto ticked_resolved = ticked_view.get_resolved_source_path();

					save_meta_json(ticked_definition_object.meta, ticked_resolved);
				}
			}
		}

		if (ImGui::IsItemHovered()) {
			auto get_lwt = [](const augs::path_type& meta_path) {
				try {
					const auto lwt = augs::last_write_time(meta_path);
					return typesafe_sprintf("(LM: %x)", augs::date_time(lwt).how_long_ago_tell_seconds());
				}
				catch (...) {
					return std::string("(Doesn't exist)");
				}
			};

			auto print_path = [&](const augs::path_type& ticked_resolved) {
				const auto meta_path = get_meta_json_path(ticked_resolved);
				return meta_path.string() + " " + get_lwt(meta_path);
			};

			if (!is_current_ticked) {
				text_tooltip("Write defaults to:\n%x", print_path(resolved));
			}
			else {
				std::string all_resolved;

				for (const auto& t : ticked_in_range) {
					const auto& ticked_definition_object = definitions[t.id];
					const auto& ticked_view = asset_definition_view<const D>(project_path, ticked_definition_object);
					const auto ticked_resolved = ticked_view.get_resolved_source_path();

					all_resolved += print_path(ticked_resolved) + "\n";
				}

				text_tooltip("Write defaults to:\n%x", all_resolved);
			}
		}
	}
}
