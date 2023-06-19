#pragma once
#include "game/modes/all_mode_includes.h"
#include "augs/templates/folded_finders.h"
#include "game/modes/ruleset_id.h"

template <class VariantType>
struct basic_mode_and_rules {
	// GEN INTROSPECTOR struct basic_mode_and_rules class VariantType
	VariantType state = arena_mode();
	raw_ruleset_id rules_id = raw_ruleset_id();
	// END GEN INTROSPECTOR

	void choose(const ruleset_id& id) {
		id.type_id.dispatch(
			[&](auto typed_default_mode) {
				using M = decltype(typed_default_mode);

				if constexpr(is_one_of_list_v<M, VariantType>) {
					state.template emplace<M>(std::move(typed_default_mode));
					rules_id = id.raw;
				}
			}
		);
	}
};

using mode_and_rules = basic_mode_and_rules<all_modes_variant>;
