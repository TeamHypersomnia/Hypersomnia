#pragma once
#include <type_traits>

#include "augs/ensure.h"

template <class Pointer, class F>
void dynamic_cast_dispatch(Pointer*, F&&) {
	ensure(false && "failed to perform dynamic dispatch");
}

template <class Candidate, class... Candidates, class Pointer, class F>
void dynamic_cast_dispatch(Pointer* const o, F&& callback) {
	auto* const cast = dynamic_cast<Candidate*>(o);

	if (cast != nullptr) {
		callback(cast);
	}
	else {
		dynamic_cast_dispatch<Candidates...>(o, std::forward<F>(callback));
	}
}