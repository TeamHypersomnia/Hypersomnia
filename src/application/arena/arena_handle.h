#pragma once
#include "augs/templates/maybe_const.h"
#include "application/predefined_rulesets.h"
#include "application/arena/mode_and_rules.h"

#include "test_scenes/test_scene_settings.h"
#include "view/game_drawing_settings.h"
#include "application/setups/editor/project/editor_project_paths.h"

struct arena_paths;
struct game_drawing_settings;

template <bool C, class ModeAndRulesType>
class basic_arena_handle {
	template <class E, class F>
	static decltype(auto) on_mode_with_input_impl(
		E& self,
		F callback
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

				if constexpr(M::needs_clean_round_state) {
					const auto in = I { *vars, self.clean_round_state, self.advanced_cosm };

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
		F callback
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
	const cosmos_solvable_significant& clean_round_state;

	template <class T>
	void transfer_all_solvables(T& from) {
		advanced_cosm.assign_solvable(from.advanced_cosm);
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

	auto get_current_round_number() const {
		return this->on_mode(
			[&](const auto& typed_mode) {
				using M = remove_cref<decltype(typed_mode)>;

				if constexpr(std::is_same_v<test_mode, M>) {
					return 0u;
				}
				else {
					return typed_mode.get_current_round_number();
				}
			}
		);
	}

	double get_inv_tickrate() const {
		return this->on_mode(
			[&](const auto& typed_mode) {
				using M = remove_cref<decltype(typed_mode)>;

				if constexpr(std::is_same_v<test_mode, M>) {
					return advanced_cosm.get_fixed_delta().template in_seconds<double>();
				}
				else {
					return typed_mode.get_round_speeds().calc_inv_tickrate();
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
				const auto current_logic_speed = static_cast<double>(m.get_round_speeds().logic_speed_mult);
				const auto chosen_audiovisual_speed = rules.view.audiovisual_speed;

				return std::max(current_logic_speed, chosen_audiovisual_speed);
			}
		});
	}

	template <class F>
	decltype(auto) on_mode(F&& f) const {
		return std::visit(std::forward<F>(f), current_mode.state);
	}

	template <class S>
	void make_default(
		S& lua,
		cosmos_solvable_significant& target_clean_round_state
	) const {
		scene.clear();

		rulesets = {};

		bomb_defusal_ruleset bomb_ruleset;

		test_scene_settings settings;
		settings.scene_tickrate = 60;

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
		bomb_ruleset.warmup_secs = 0;

		const auto bomb_ruleset_id = raw_ruleset_id(0);
		rulesets.all.template get_for<bomb_defusal>().try_emplace(bomb_ruleset_id, std::move(bomb_ruleset));

		current_mode.rules_id = bomb_ruleset_id;
		current_mode.state = bomb_defusal();

		ruleset_id id;
		id.raw = bomb_ruleset_id;
		id.type_id.set<bomb_defusal>();

		rulesets.meta.server_default = id;
		rulesets.meta.playtest_default = id;

		target_clean_round_state = advanced_cosm.get_solvable().significant;
	}

	template <class... Args>
	decltype(auto) advance(Args&&... args) const {
		return on_mode_with_input(
			[&](auto& typed_mode, const auto& in) -> decltype(auto) {
				return typed_mode.advance(in, std::forward<Args>(args)...);
			}
		);
	}

	auto& get_cosmos() const {
		return advanced_cosm;
	}

	void adjust(game_drawing_settings& settings) const {
		on_mode_with_input(
			[&](const auto&, const auto& in) {
				const auto& r = in.rules.view;

				settings.enemy_hud_mode = r.enemy_hud_mode;
				settings.fog_of_war = r.fog_of_war;

				if (!r.enable_danger_indicators) {
					settings.draw_danger_indicators.is_enabled = false;
				}

				if (!r.enable_teammate_indicators) {
					settings.draw_teammate_indicators.is_enabled = false;
				}

				if (!r.enable_tactical_indicators) {
					settings.draw_tactical_indicators.is_enabled = false;
				}
			}
		);
	}

	auto emigrate_mode_session() const {
		return this->on_mode(
			[](const auto& typed_mode) { return typed_mode.emigrate(); }
		);
	}


	template <class A>
	void migrate_mode_session(const A& session) const {
		this->on_mode_with_input(
			[&session](auto& typed_mode, const auto& input) {
				typed_mode.migrate(input, session);
			}
		);
	}
};
