#pragma once
#include "game/cosmos/cosmos_solvable.h"
#include "augs/enums/callback_result.h"

template <template <class> class Predicate, class S, class F>
void cosmos_solvable::for_each_entity_impl(S& self, F callback) {
	self.significant.for_each_entity_pool(
		[&](auto& p) {
			using P = decltype(p);
			using pool_type = remove_cref<P>;

			using Solvable = typename pool_type::mapped_type;
			using E = entity_type_of<Solvable>;

			if constexpr(Predicate<E>::value) {
				using index_type = typename pool_type::used_size_type;

				for (index_type i = 0; i < p.size(); ++i) {
					using R = decltype(callback(p.data()[i], i));
					
					if constexpr(std::is_same_v<R, void>) {
						callback(p.data()[i], i);
					}
					else {
						const auto result = callback(p.data()[i], i);

						if constexpr(std::is_same_v<R, callback_result>) {
							if (result == callback_result::ABORT) {
								break;
							}
						}
						else {
							static_assert(always_false_v<E>, "Wrong return type from a callback to for_each_entity.");
						}
					}
				}
			}
		}
	);
}

template <template <class> class Predicate, class F>
void cosmos_solvable::for_each_entity(F&& callback) {
	for_each_entity_impl<Predicate>(*this, std::forward<F>(callback));
}

template <template <class> class Predicate, class F>
void cosmos_solvable::for_each_entity(F&& callback) const {
	for_each_entity_impl<Predicate>(*this, std::forward<F>(callback));
}
