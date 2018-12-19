#pragma once
#include "augs/templates/maybe_const.h"
#include "application/predefined_rulesets.h"
#include "application/arena/mode_and_rules.h"

#include "application/arena/arena_utils.h"
#include "test_scenes/test_scene_settings.h"

struct arena_paths;

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
				
				const auto vars = mapped_or_nullptr(
					self.rulesets.all.template get_for<M>(), 
					self.current_mode.rules_id
				);

				ensure(vars != nullptr);

				if constexpr(M::needs_initial_signi) {
					const auto in = I { *vars, self.initial_signi, self.advanced_cosm };

					return callback(typed_mode, in);
				}
				else {
					const auto in = I { *vars, self.advanced_cosm };
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
	maybe_const_ref_t<C, cosmos> advanced_cosm;
	maybe_const_ref_t<C, predefined_rulesets> rulesets;
	const cosmos_solvable_significant& initial_signi;

	template <class T>
	void assign_all_solvables(const T& from) const {
		advanced_cosm = from.advanced_cosm;
		current_mode = from.current_mode;
	}

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) const {
		return this->on_mode_with_input_impl(*this, std::forward<Args>(args)...);
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
					return advanced_cosm.get_fixed_delta().template in_seconds<double>();
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
	decltype(auto) on_mode(F&& f) const {
		return std::visit(std::forward<F>(f), current_mode.state);
	}

	void load_from(
		const arena_paths& paths,
		cosmos_solvable_significant& target_initial_signi
	) const {
		load_arena_from(
			paths,
			scene,
			rulesets
		);

		target_initial_signi = advanced_cosm.get_solvable().significant;
	}

	template <class S>
	void make_default(
		S& lua,
		cosmos_solvable_significant& target_initial_signi
	) const {
		scene.clear();

		rulesets = {};

		bomb_mode_ruleset bomb_ruleset;

		test_scene_settings settings;
		settings.scene_tickrate = 128;

		{
			test_mode_ruleset dummy;

			scene.make_test_scene(
				lua,
				settings,
				dummy,
				std::addressof(bomb_ruleset)
			);
		}

		bomb_ruleset.bot_quota = 0;
		bomb_ruleset.bot_names.clear();

		const auto bomb_ruleset_id = raw_ruleset_id(0);
		rulesets.all.template get_for<bomb_mode>().try_emplace(bomb_ruleset_id, std::move(bomb_ruleset));

		current_mode.rules_id = bomb_ruleset_id;
		current_mode.state = bomb_mode();

		ruleset_id id;
		id.raw = bomb_ruleset_id;
		id.type_id.set<bomb_mode>();

		rulesets.meta.server_default = id;
		rulesets.meta.playtest_default = id;

		target_initial_signi = advanced_cosm.get_solvable().significant;
	}

	template <class Entropy, class Callbacks>
	void advance(
		const Entropy& entropy,
		const Callbacks& callbacks
	) const {
		on_mode_with_input(
			[&](auto& typed_mode, const auto& in) {
				typed_mode.advance(in, entropy, callbacks);
			}
		);
	}

	auto& get_cosmos() const {
		return advanced_cosm;
	}
};
