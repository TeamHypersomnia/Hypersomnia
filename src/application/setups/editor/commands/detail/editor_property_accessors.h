#pragma once
#include "application/setups/editor/property_editor/on_field_address.h"
#include "game/cosmos/change_common_significant.hpp"
#include "game/cosmos/cosmic_functions.h"

template <class T>
static constexpr bool should_reinfer_after_change(const T&) {
	return 
		should_reinfer_when_tweaking_v<T>
		|| is_synchronized_v<T>
	;
}

struct editor_property_accessors {
	template <class C, class V>
	static void force_set_clock(C& cosm, V value) {
		cosm.get_solvable({}).significant.clk.now.step = value;
	}

	template <class C, class F>
	static bool access(
		const flavour_property_id& self,
		C& cosm,
		const entity_type_id type_id,
		const std::vector<raw_entity_flavour_id>& flavour_ids,
		F callback
	) {
		bool reinfer = false;

		get_by_dynamic_id(
			all_entity_types(),
			type_id,
			[&](auto e) {
				using E = decltype(e);

				get_by_dynamic_index(
					invariants_of<E> {},
					self.invariant_id,
					[&](const auto& i) {
						using Invariant = remove_cref<decltype(i)>;

						for (const auto& f : flavour_ids) {
							const auto result = on_field_address(
								std::get<Invariant>(cosm.get_flavour({}, typed_entity_flavour_id<E>(f)).invariants),
								self.field,

								[&](auto& resolved_field) -> callback_result {
									return callback(resolved_field);
								}
							);

							if (callback_result::ABORT == result) {
								break;
							}
						}

						if (should_reinfer_after_change(i)) {
							reinfer = true;
						}
					}
				);
			}
		);

		return reinfer;
	}

	template <class C, class F>
	static bool access(
		const entity_property_id& self,
		C& cosm,
		const entity_type_id type_id,
		const std::vector<raw_entity_flavour_id>& flavour_ids,
		F callback
	) {
		get_by_dynamic_id(
			all_entity_types(),
			type_id,
			[&](auto e) {
				using E = decltype(e);

				get_by_dynamic_index(
					components_of<E> {},
					self.component_id,
					[&](const auto& c) {
						using Component = remove_cref<decltype(c)>;

						for (const auto& f : flavour_ids) {
							const auto result = on_field_address(
								std::get<Component>(cosm.get_flavour({}, typed_entity_flavour_id<E>(f)).initial_components),
								self.field,

								[&](auto& resolved_field) -> callback_result {
									return callback(resolved_field);
								}
							);

							if (callback_result::ABORT == result) {
								break;
							}
						}
					}
				);
			}
		);

		return false;
	}

	template <class C, class F>
	static bool access(
		const entity_property_id& self,
		C& cosm,
		const entity_type_id type_id,
		const std::vector<entity_id_base>& entity_ids,
		F callback
	) {
		bool reinfer = false;

		get_by_dynamic_id(
			all_entity_types(),
			type_id,
			[&](auto e) {
				using E = decltype(e);

				get_by_dynamic_index(
					components_of<E> {},
					self.component_id,
					[&](const auto& c) {
						using Component = remove_cref<decltype(c)>;

						for (const auto& e : entity_ids) {
							auto specific_handle = cosm[typed_entity_id<E>(e)];

							const auto result = on_field_address(
								std::get<Component>(specific_handle.get({}).components),
								self.field,
								[&](auto& resolved_field) -> callback_result {
									return callback(resolved_field);
								}
							);

							if (callback_result::ABORT == result) {
								break;
							}
						}

						if (should_reinfer_after_change(c)) {
							reinfer = true;
						}
					}
				);
			}
		);

		return reinfer;
	}

	template <class id_type, class C, class Container, class F>
	static void access(
		const asset_property_id<id_type>& self,
		C& in,
		const Container& asset_ids,
		F callback
	) {
		auto& definitions = access_asset_pool<id_type>(in, {});

		for (const auto& id : asset_ids) {
			const auto result = on_field_address(
				definitions[id],
				self.field,
				[&](auto& resolved_field) -> callback_result {
					return callback(resolved_field);
				}
			);

			if (callback_result::ABORT == result) {
				break;
			}
		}
	}

	template <class T, class F>
	static void access_each_property(
		const change_flavour_property_command& self,
		T in,
		F&& callback
	) {
		auto& cosm = in.get_cosmos();

		if (access(self.property_id, cosm, self.type_id, self.affected_flavours, continue_if_nullopt(std::forward<F>(callback)))) {
			cosm.change_common_significant([&](auto&) { return changer_callback_result::REFRESH; });
		}
	}

	template <class T, class F>
	static void access_each_property(
		const change_initial_component_property_command& self,
		T in,
		F&& callback
	) {
		auto& cosm = in.get_cosmos();

		access(self.property_id, cosm, self.type_id, self.affected_flavours, continue_if_nullopt(std::forward<F>(callback)));
	}

	template <class T, class F>
	static void access_each_property(
		const change_entity_property_command& self,
		T in,
		F&& callback
	) {
		auto& cosm = in.get_cosmos();

		if (access(self.property_id, cosm, self.type_id, self.affected_entities, continue_if_nullopt(std::forward<F>(callback)))) {
			cosmic::reinfer_all_entities(cosm);
		}
	}

	template <class T, class F>
	static void access_each_property(
		const change_common_state_command& self,
		T in,
		F callback
	) {
		auto& cosm = in.get_cosmos();

		cosm.change_common_significant([&](auto& common_signi) {
			on_field_address(
				common_signi,
				self.field,
				continue_if_nullopt([&](auto& resolved_field) {
					return callback(resolved_field);
				})
			);

			return changer_callback_result::DONT_REFRESH;
		});
	}

	template <class T, class F>
	static void access_each_property(
		const change_current_mode_property_command& self,
		T in,
		F callback
	) {
		std::visit(
			[&](auto& typed_mode) {
				on_field_address(
					typed_mode,
					self.field,
					continue_if_nullopt([&](auto& resolved_field) {
						return callback(resolved_field);
					})
				);
			},
			in.folder.player.current_mode.state
		);
	}

	template <class T, class F>
	static void access_each_property(
		const change_rulesets_meta_property& self,
		T in,
		F callback
	) {
		on_field_address(
			in.folder.commanded->rulesets.meta,
			self.field,
			continue_if_nullopt([&](auto& resolved_field) {
				return callback(resolved_field);
			})
		);
	}

	template <class T, class F>
	static void access_each_property(
		const change_mode_player_property_command& self,
		T in,
		F callback
	) {
		std::visit(
			[&](auto& typed_mode) {
				if (const auto player = mapped_or_nullptr(typed_mode.players, self.player_id)) {
					on_field_address(
						*player,
						self.field,
						continue_if_nullopt([&](auto& resolved_field) {
							return callback(resolved_field);
						})
					);
				}
			},
			in.folder.player.current_mode.state
		);
	}

	template <class T, class F>
	static void access_each_property(
		const change_ruleset_property_command& self,
		T in,
		F callback
	) {
		in.folder.commanded->rulesets.all.visit(
			self.type_id,
			[&](auto& typed_mode_rules) {
				if (auto* const found_mode_rules = mapped_or_nullptr(typed_mode_rules, self.id)) {
					on_field_address(
						*found_mode_rules,
						self.field,
						continue_if_nullopt([&](auto& resolved_field) {
							return callback(resolved_field);
						})
					);
				}
			}
		);
	}

	template <class id_type, class T, class F>
	static void access_each_property(
		const change_asset_property_command<id_type>& self,
		T in,
		F&& callback
	) {
		access(
			self.property_id,
			in,
			self.affected_assets,
			continue_if_nullopt(std::forward<F>(callback))
		);
	}

	template <class T, class F>
	static void access_each_property(
		const change_group_property_command& self,
		T in,
		F callback
	) {
		auto& groups = in.folder.commanded->view_ids.selection_groups.groups;

		if (auto mapped_group = mapped_or_nullptr(groups, self.group_index)) {
			callback(mapped_group->name);
		}
	}
};
