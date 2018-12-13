#pragma once

template <class V, class A, class C, class F>
decltype(auto) general_on_mode_with_input(
	V& mode_variant,
	const A& all_vars,
	const raw_ruleset_id rules_id,
	const cosmos_solvable_significant& initial_signi,
	C& cosm,
	F&& callback
) {
	return std::visit(
		[&](auto& typed_mode) -> decltype(auto) {
			using M = remove_cref<decltype(typed_mode)>;
			using I = typename M::input;
			
			const auto vars = mapped_or_nullptr(all_vars.template get_for<M>(), rules_id);
			ensure(vars != nullptr);

			if constexpr(M::needs_initial_signi) {
				const auto in = I { *vars, initial_signi, cosm };

				return callback(typed_mode, in);
			}
			else {
				const auto in = I { *vars, cosm };
				return callback(typed_mode, in);
			}
		},
		mode_variant
	);
}


