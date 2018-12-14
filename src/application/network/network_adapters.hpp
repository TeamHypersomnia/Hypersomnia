#pragma once
#include "application/network/network_adapters.h"
#include "augs/enums/callback_result.h"
#include "augs/templates/traits/function_traits.h"
#include "augs/templates/filter_types.h"

template <class T>
using payload_of_t = last_argument_t<decltype(&T::read_payload)>;

namespace detail {
	template <class... Args>
	struct strap_can_create {
		template <class M, class = void>
		struct write_payload : std::false_type {};

		template <class M>
		struct write_payload<
			M, 
			decltype(std::declval<M&>().write_payload(std::declval<Args>()...), void())
		> : std::true_type {};

		template <class M>
		struct can_create : write_payload<std::remove_pointer_t<M>> {};
	};
}
