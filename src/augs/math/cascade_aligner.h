#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"

template <class P, class S>
struct cascade_aligner_node {
	P p;
	S s;

	P ap;
	S as;

	cascade_aligner_node(P p, S s, S as) :
	p(p),
	s(s),

	ap(p),
	as(as)
	{}

	operator P() const {
		return ap;
	}

	operator transformr() const {
		return transformr(ap, 0);
	}

	auto result() const {
		return transformr(ap, 0);
	}
};

template <class P, class S>
class cascade_aligner : cascade_aligner_node<P, S> {
public:
	using base = cascade_aligner_node<P, S>;
	using base::base;
	using base::operator P;
	using base::operator transformr;

	using PV = std::vector<base>;
	using base::p;
	using base::s;
	using base::ap;
	using base::as;

private:
	PV written;
public:

	auto clone() const {
		return *this;
	}

	auto& pop(int n = 1) {
		while (n--) {
			written.pop_back();
		}

		base::operator=(written.back());
		return *this;
	}

	auto& stretch_r(int limit = 0) {
		const auto r = p.x + s.x / 2;
		const auto target_r = r + (limit * as.x);

		const auto al = ap.x;
		const auto ar = al + as.x / 2;

		if (const auto dt = target_r - ar; dt > 0) {
			as.x = target_r - al;
			ap.x = target_r - as.x / 2;
		}

		return *this;
	}

	auto& fill_ri() {
		const auto r = p.x + s.x / 2;
		const auto ar = ap.x + as.x / 2;
		auto times = int((r - ar) / as.x);

		while (times-- > 0) {
			nr().push();
		}

		return *this;
	}

	// nudge right
	auto& nr() {
		ap += P(as.x, 0);
		return *this;
	}

	// nudge left
	auto& nl() {
		ap -= P(as.x, 0);
		return *this;
	}

	// nudge down
	auto& nd() {
		ap += P(0, as.y);
		return *this;
	}

	// nudge up
	auto& nu() {
		ap -= P(0, as.y);
		return *this;
	}

	auto& bo() {
		ap += P(0, s.y / 2 + as.y / 2);
		return *this;
	}

	auto& bi() {
		ap += P(0, s.y / 2 - as.y / 2);
		return *this;
	}

	auto& ro() {
		ap += P(s.x / 2 + as.x / 2, 0);
		return *this;
	}

	auto& ri() {
		ap += P(s.x / 2 - as.x / 2, 0);
		return *this;
	}

	auto& to() {
		ap -= P(0, s.y / 2 + as.y / 2);
		return *this;
	}

	auto& ti() {
		ap -= P(0, s.y / 2 - as.y / 2);
		return *this;
	}

	auto& lo() {
		ap -= P(s.x / 2 + as.x / 2, 0);
		return *this;
	}

	auto& li() {
		ap -= P(s.x / 2 - as.x / 2, 0);
		return *this;
	}

	auto& push() {
		written.push_back(*this);
		return *this;
	}

	auto& next(const S next_size) {
		push();

		p = ap;
		s = as;
		ap = p;
		as = next_size;

		return *this;
	}

	auto all() {
		return written;
	}

	auto begin() {
		return written.begin();
	}

	auto end() {
		return written.end();
	}

	auto result() const {
		return operator transformr();
	}
};

template <class P, class S>
auto make_cascade_aligner(const P p, const S s, const S as) {
	return cascade_aligner<P, S>(p, s, as);
}
