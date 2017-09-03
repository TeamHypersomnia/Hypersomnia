#include "augs/templates/introspect.h"

#include "game/assets/all_assets.h"

#include "generated/introspectors.h"

void all_logical_assets::update_from(const all_viewable_defs& sources) {
	augs::introspect(
		[this](auto label, const auto& from) {
			using T = std::decay_t<decltype(from)>;

			auto& into = get_store_by(typename T::key_type());

			for (const auto& s : from) {
				into.emplace(s.first, s.second);
			}
		}, 
		sources.all
	);
}