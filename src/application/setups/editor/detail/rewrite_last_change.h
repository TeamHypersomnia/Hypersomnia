#pragma once
#include "augs/templates/identity_templates.h"
#include "application/setups/editor/commands/editor_command_traits.h"
#include "augs/readwrite/to_bytes.h"

template <class derived>
template <class T>
void change_property_command<derived>::rewrite_change(
	const T& new_value,
	const editor_command_input in
) {
	augs::assign_bytes(value_after_change, new_value);
	rewrite_change_internal(in);
}

template <class cmd_type, class P, class E, class F = empty_callback>
auto make_rewrite_last_change(
	const P& property_location,
	const E& cmd_in,
	F&& before_rewrite = empty_callback()
) {
	return [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = cmd_in.get_history().last_command();

		if (auto* const cmd = std::get_if<cmd_type>(std::addressof(last))) {
			const auto current_step = cmd_in.get_current_step();

			if (current_step == cmd->common.when_happened) {
				cmd->built_description = description + property_location;
				before_rewrite(*cmd, new_content);
				cmd->rewrite_change(new_content, cmd_in);
			}
			else {
				/* Spawn new command for another step */
				auto cloned_cmd = *cmd;
				cloned_cmd.built_description = description + property_location;

				if constexpr(has_member_sanitize_v<cmd_type, editor_command_input>) {
					cloned_cmd.sanitize(cmd_in);
				}

				before_rewrite(cloned_cmd, new_content);

				augs::assign_bytes(cloned_cmd.value_after_change, new_content);
				cloned_cmd.common.has_parent = true;

				::post_editor_command(cmd_in, cloned_cmd);
			}
		}
	};
}
