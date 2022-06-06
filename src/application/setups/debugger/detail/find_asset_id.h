#pragma once

template <class P, class I>
auto find_asset_id_by_name(
	const std::string& searched_name,
	const P& definitions,
	const I& image_defs
) {
	using K = typename P::key_type;

	std::optional<K> result_id;

	for_each_id_and_object(definitions,
		[&](const auto id, const auto& l) {
			if (searched_name == ::get_displayed_name(l, image_defs)) {
				if (result_id == std::nullopt) {
					result_id = id;
				}
			}
		}
	);

	return result_id;
}
