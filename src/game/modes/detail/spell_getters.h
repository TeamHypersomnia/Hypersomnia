#pragma once

template <class F>
decltype(auto) on_spell(const cosmos& cosm, const spell_id& id, F&& callback) {
	return id.dispatch(
		[&](auto s) -> decltype(auto) {
			using S = decltype(s);
			return callback(std::get<S>(cosm.get_common_significant().spells));
		}
	);
};

inline assets::image_id get_spell_image(
	const cosmos& cosm,
	const spell_id& id
) {
	auto getter = [&](const auto& spell_data) {
		return spell_data.appearance.icon;
	};

	return ::on_spell(cosm, id, getter);

}
