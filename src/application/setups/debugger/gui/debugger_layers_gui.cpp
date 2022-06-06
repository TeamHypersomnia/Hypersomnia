#include "augs/misc/enum/enum_boolset.h"
#include "augs/string/string_templates.h"

#include "application/setups/debugger/gui/debugger_layers_gui.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/debugger/detail/checkbox_selection.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"

#include "augs/templates/enum_introspect.h"

void debugger_layers_gui::perform(
	const property_debugger_settings& settings,
	augs::maybe<render_layer_filter>& viewing,
	augs::maybe<render_layer_filter>& selecting
) {
	using render_layer_filters = decltype(render_layer_filter::layers);
	using namespace augs::imgui;

	auto make_for_each_item = [&](render_layer_filters& v) {
		return [&](auto callback) {
			augs::for_each_enum_except_bounds([&v, &callback](const render_layer e) {
				(void)v;
				callback(e);
			});
		};
	};

	auto window = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!window) {
		return;
	}

	acquire_keyboard_once();

	const auto max_text_w = [&]() {
		float max_w = -1;

		augs::for_each_enum_except_bounds([&](const render_layer e) {
			const auto& f = format_enum(e);
			max_w = std::max(max_w, ImGui::CalcTextSize(f.c_str(), nullptr, true).x);
		});

		return max_w;
	}();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	ImGui::Columns(3);
	ImGui::SetColumnWidth(0, max_text_w + 12);

	text_disabled("Name");
	ImGui::NextColumn();
	text_disabled("V");
	ImGui::NextColumn();
	text_disabled("S");
	ImGui::NextColumn();

	ImGui::Separator();

	text_disabled("Enabled");
	ImGui::NextColumn();

	const auto now_pos = ImGui::GetCursorPosX();
	checkbox("##EnabledViewing", viewing.is_enabled);
	ImGui::SameLine();
	const auto checkbox_w = ImGui::GetCursorPosX() - now_pos;

	ImGui::SetColumnWidth(1, checkbox_w + 4);
	ImGui::SetColumnWidth(2, checkbox_w + 4);

	ImGui::NextColumn();
	checkbox("##EnabledSelecting", selecting.is_enabled);
	ImGui::NextColumn();

	ImGui::Separator();

	text_disabled("All");

	ImGui::NextColumn();

	{
		auto disabled = ::maybe_disabled_cols(settings, !viewing.is_enabled);

		::do_tick_all_checkbox(
			settings,
			viewing.value.layers,
			make_for_each_item(viewing.value.layers),
			"##Viewing-all"
		);
	}

	ImGui::NextColumn();

	{
		auto disabled = ::maybe_disabled_cols(settings, !selecting.is_enabled);

		::do_tick_all_checkbox(
			settings,
			selecting.value.layers,
			make_for_each_item(selecting.value.layers),
			"##Selecting-all"
		);
	}

	ImGui::NextColumn();

	ImGui::Separator();

	augs::for_each_enum_except_bounds([&](const render_layer e) {
		const auto label = format_enum(e);

		if (!filter.PassFilter(label.c_str())) {
			return;
		}

		text(label);

		ImGui::NextColumn();

		{
			auto disabled = ::maybe_disabled_cols(settings, !viewing.is_enabled);

			::do_selection_checkbox(
				viewing.value.layers,
				e,
				viewing.value.layers[e],
				static_cast<int>(e)
			);
		}

		ImGui::NextColumn();

		{
			auto disabled = ::maybe_disabled_cols(settings, !selecting.is_enabled);

			::do_selection_checkbox(
				selecting.value.layers,
				e,
				selecting.value.layers[e],
				-static_cast<int>(e)
			);
		}

		ImGui::NextColumn();
	});
}
