#pragma once
#include "augs/templates/maybe_const.h"

#include "test_scenes/test_scene_settings.h"
#include "view/game_drawing_settings.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "augs/log.h"

class test_mode;
struct intercosm;

struct arena_paths;
struct game_drawing_settings;
struct synced_dynamic_vars;

template <bool C, class ModeVariant, class RulesVariant>
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
				
				const auto vars = std::get_if<typename M::ruleset_type>(std::addressof(self.ruleset));

				if (vars == nullptr) {
					LOG_NVPS(self.current_mode_state.index(), self.ruleset.index());
				}

				ensure(vars != nullptr);

				if constexpr(M::needs_clean_round_state) {
					const auto in = I { self.dynamic_vars, *vars, self.clean_round_state, self.advanced_cosm };

					return callback(typed_mode, in);
				}
				else {
					const auto in = I { self.dynamic_vars, *vars, self.advanced_cosm };
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
				
				const auto vars = std::get_if<typename M::ruleset_type>(std::addressof(self.ruleset));

				if (vars == nullptr) {
					LOG_NVPS(self.current_mode_state.index(), self.ruleset.index());
				}

				ensure(vars != nullptr);

				return callback(typed_mode, *vars);
			}
		);
	}

public:
	maybe_const_ref_t<C, ModeVariant> current_mode_state;
	maybe_const_ref_t<C, intercosm> scene;
	maybe_const_ref_t<C, cosmos> advanced_cosm;
	maybe_const_ref_t<C, RulesVariant> ruleset;
	const cosmos_solvable_significant& clean_round_state;
	const synced_dynamic_vars& dynamic_vars;

	void verify_mode_hasnt_changed() {
		on_mode(
			[&](auto& typed_mode) -> decltype(auto) {
				using M = remove_cref<decltype(typed_mode)>;
				using R = typename M::ruleset_type;

				const auto vars = std::get_if<R>(std::addressof(ruleset));

				if (vars == nullptr) {
					LOG_NVPS(current_mode_state.index(), ruleset.index());
					LOG("WARNING!!! RULESET DOES NO LONGER MATCH THE MODE!!! Setting a default ruleset.");

					ruleset = R();
				}
			}
		);
	};

	template <class T>
	void transfer_all_solvables(T& from) {
		advanced_cosm.assign_solvable(from.advanced_cosm);
		current_mode_state = from.current_mode_state;

		verify_mode_hasnt_changed();
	}

	template <class... Args>
	decltype(auto) on_mode_with_input(Args&&... args) const {
		return this->on_mode_with_input_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_mode_with_rules(Args&&... args) const {
		return this->on_mode_with_rules_impl(*this, std::forward<Args>(args)...);
	}

	auto choose_mode(const RulesVariant& v) {
		ruleset = v;

		std::visit(
			[&]<typename T>(const T&) {
				current_mode_state = typename T::mode_type();
			}, 
			ruleset
		);
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
		return std::visit(std::forward<F>(f), current_mode_state);
	}

	void make_default(
		cosmos_solvable_significant& target_clean_round_state
	) const {
		{
			scene.clear();

			test_scene_settings settings;
			settings.scene_tickrate = 60;

			scene.make_test_scene(settings);
		}

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

				settings.teammates_are_enemies = in.rules.is_ffa();
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

	game_mode_name_type get_current_game_mode_name() const {
		return on_mode_with_input(
			[&](const auto& mode, const auto& input) {
				return mode.get_name(input);
			}
		);
	}

	auto solvable_hash() const {
		return get_cosmos().template calculate_solvable_signi_hash<uint32_t>();
	}
};
