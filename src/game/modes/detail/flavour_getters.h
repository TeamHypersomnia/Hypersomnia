#pragma once

inline bool is_alive(const cosmos& cosm, const item_flavour_id& t) {
	if (!t.is_set()) {
		return false;
	}

	return t.dispatch(
		[&](const auto& typed_id) {
			return nullptr != cosm.find_flavour(typed_id);
		}
	);
};

inline bool is_alive(const cosmos& cosm, const spell_id& t) {
	(void)cosm;
	return t.is_set();
}

inline const inventory_slot* find_slot_def_of(
	const cosmos& cosm, 
	const item_flavour_id& container_flavour_id, 
	const slot_function f
) {
	return cosm.on_flavour(container_flavour_id, [&](const auto& typed_flavour) -> const inventory_slot* {
		if (const auto c = typed_flavour.template find<invariants::container>()) {
			if (const auto slot_def = mapped_or_nullptr(c->slots, f)) {
				return slot_def;
			}
		}

		return nullptr;
	});
}

template <class F>
inline auto get_flavour_name(const cosmos& cosm, const F& flavour) {
	return cosm.get_flavour(flavour).get_name();
}

template <class F>
inline auto get_flavour_image(const cosmos& cosm, const F& flavour) {
	return cosm.get_flavour(flavour).get_image_id();
}

inline item_flavour_id get_allowed_flavour_of(
	const cosmos& cosm, 
	const item_flavour_id& container_flavour_id, 
	const slot_function f
) {
	if (const auto def = find_slot_def_of(cosm, container_flavour_id, f)) {
		return def->only_allow_flavour;
	}

	return item_flavour_id();
}

inline item_flavour_id get_allowed_flavour_of_deposit(const cosmos& cosm, const item_flavour_id& mag_id) {
	return get_allowed_flavour_of(cosm, mag_id, slot_function::ITEM_DEPOSIT);
}

inline assets::image_id image_of(
	const cosmos& cosm,
	const item_flavour_id& of
) {
	if (!::is_alive(cosm, of)) {
		return assets::image_id();
	}

	return cosm.on_flavour(of, [&](const auto& typed_flavour) {
		return typed_flavour.get_image_id();
	});
}

