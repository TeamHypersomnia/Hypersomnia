#pragma once

template <class C>
void sort_flavours_by_name(const cosmos& cosm, C& ids) {
	sort_range_by(
		ids,
		[&](const auto id) -> const std::string* { 
			return std::addressof(cosm.get_flavour(id).template get<invariants::text_details>().name);
		},
		[](const auto& a, const auto& b) {
			return *a.compared < *b.compared;
		}
	);
}

