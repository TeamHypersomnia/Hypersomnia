#pragma once
#include "application/setups/debugger/property_debugger/general_edit_properties.h"
#include "application/setups/debugger/property_debugger/commanding_property_debugger_input.h"
#include "application/setups/debugger/detail/rewrite_last_change.h"

template <
	class Behaviour = default_edit_properties_behaviour,
	class T,
	class cmd_type,
	class S = default_widget_provider,
	class D = default_sane_default_provider
>
void singular_edit_properties(
	const commanding_property_debugger_input& in,
	const T& parent_altered,
	const std::string& property_location,
	const cmd_type& cmd_pattern = cmd_type(),
	S special_widget_provider = {},
	D sane_defaults = {},
	const int extra_columns = 0
) {
	using namespace augs::imgui;

	auto& cmd_in = in.command_in;
	auto& history = cmd_in.get_history();

	using field_type_id = field_type_id_t<cmd_type>;

	auto post_new_change = [&](
		const auto& description,
		const field_address<field_type_id>& field,
		const auto& new_content
	) {
		auto cmd = cmd_pattern;
		cmd.field = field;

		augs::assign_bytes(cmd.value_after_change, new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, cmd_in);
	};

	auto rewrite_last_change = make_rewrite_last_change<cmd_type>(
		property_location,
		cmd_in
	);

	general_edit_properties<field_type_id, Behaviour>(
		in.prop_in, 
		parent_altered,
		post_new_change,
		rewrite_last_change,
		true_returner(),
		special_widget_provider,
		sane_defaults,
		extra_columns
	);
}
