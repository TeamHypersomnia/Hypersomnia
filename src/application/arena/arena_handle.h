#pragma once
#include "augs/templates/maybe_const.h"
#include "application/predefined_rulesets.h"
#include "application/arena/mode_and_rules.h"

template <bool C, class ModeAndRulesType>
class basic_arena_handle {
	template <class E, class F>
	static decltype(auto) on_mode_with_input_impl(
		E& self,
		F&& callback
	) {
		return self.on_mode(
			[&](auto& typed_mode) -> decltype(auto) {
				using M = remove_cref<decltype(typed_mode)>;
				using I = typename M::template basic_input<C>;
				
				auto& cosm = self.scene.world;

				const auto vars = mapped_or_nullptr(
					self.rulesets.all.template get_for<M>(), 
					self.current_mode.rules_id
				);

				ensure(vars != nullptr);

				if constexpr(M::needs_initial_signi) {
					const auto in = I { *vars, self.initial_signi, cosm };

					return callback(typed_mode, in);
				}
				else {
					const auto in = I { *vars, cosm };
					return callback(typed_mode, in);
				}
			}
		);
	}

	template <class E, class F>
	static decltype(auto) on_mode_with_rules_impl(
		E& self,
		F&& callback
	) {
		return self.on_mode(
			[&](auto& typed_mode) -> decltype(auto) {
				using M = remove_cref<decltype(typed_mode)>;
				
				const auto vars = mapped_or_nullptr(self.rulesets.all.template get_for<M>(), self.current_mode.rules_id);
				ensure(vars != nullptr);

				return callback(typed_mode, *vars);
			}
		);
	}

public:
	maybe_const_ref_t<C, ModeAndRulesType> current_mode;
	maybe_const_ref_t<C, intercosm> scene;
	maybe_const_ref_t<C, predefined_rulesets> rulesets;
	const cosmos_solvable_significant& initial_signi;

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) {
		return this->on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) const {
		return this->on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_mode_with_rules(Args&&... args) {
		return this->on_mode_with_rules_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_mode_with_rules(Args&&... args) const {
		return this->on_mode_with_rules_impl(*this, std::forward<Args>(args)...);
	}

	double get_inv_tickrate() const {
		return this->on_mode(
			[&](const auto& typed_mode) {
				using M = remove_cref<decltype(typed_mode)>;

				if constexpr(std::is_same_v<test_mode, M>) {
					return scene.world.get_fixed_delta().template in_seconds<double>();
				}
				else {
					return typed_mode.round_speeds.calc_inv_tickrate();
				}
			}
		);
	}

	double get_audiovisual_speed() const {
		return this->on_mode_with_rules([](const auto& m, const auto& rules) -> double {
			using M = remove_cref<decltype(m)>;

			if constexpr(std::is_same_v<test_mode, M>) {
				return 1.0;
			}
			else {
				const auto current_logic_speed = m.round_speeds.logic_speed_mult;
				const auto chosen_audiovisual_speed = rules.view.audiovisual_speed;

				return std::max(current_logic_speed, chosen_audiovisual_speed);
			}
		});
	}

	template <class F>
	decltype(auto) on_mode(F&& f) {
		return std::visit(std::forward<F>(f), current_mode.state);
	}

	template <class F>
	decltype(auto) on_mode(F&& f) const {
		return std::visit(std::forward<F>(f), current_mode.state);
	}

};
