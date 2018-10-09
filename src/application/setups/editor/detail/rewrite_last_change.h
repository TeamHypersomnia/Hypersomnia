#pragma once
#include "augs/templates/identity_templates.h"

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
			cmd->built_description = description + property_location;
			before_rewrite(*cmd, new_content);
			cmd->rewrite_change(augs::to_bytes(new_content), cmd_in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};
}
