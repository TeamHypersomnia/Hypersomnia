#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"

template <class P, class S, class M>
struct cascade_aligner_node {
	P p;
	S s;

	P ap;
	S as;

	M meta;

	cascade_aligner_node(
		const P& p, 
		const S& s, 
	   	const M& meta
	) :
		p(p),
		s(s),

		ap(p),
		as(meta.get_size()),

		meta(meta)
	{}

	auto* operator->() {
		return &meta;
	}

	void create() {
		meta.create(ap);
	}

	operator P() const {
		return ap;
	}

	operator transformr() const {
		return transformr(ap, 0);
	}

	void stretch_r(int limit = 0) {
		const auto r = p.x + s.x / 2;
		const auto target_r = r + (limit * as.x);

		const auto al = ap.x;
		const auto ar = al + as.x / 2;

		if (const auto dt = target_r - ar; dt > 0) {
			as.x = target_r - al;
			ap.x = target_r - as.x / 2;
		}
	}

	auto result() const {
		return transformr(ap, 0);
	}

	void nr() {	ap += P(as.x, 0); }
	void nl() {	ap -= P(as.x, 0); }
	void nd() {	ap += P(0, as.y); }
	void nu() {	ap -= P(0, as.y); }

	void bo() {	ap += P(0, s.y / 2 + as.y / 2); }
	void bi() {	ap += P(0, s.y / 2 - as.y / 2); }
	void ro() {	ap += P(s.x / 2 + as.x / 2, 0); }
	void ri() {	ap += P(s.x / 2 - as.x / 2, 0); }
	void to() {	ap -= P(0, s.y / 2 + as.y / 2); }
	void ti() {	ap -= P(0, s.y / 2 - as.y / 2); }
	void lo() {	ap -= P(s.x / 2 + as.x / 2, 0); }
	void li() {	ap -= P(s.x / 2 - as.x / 2, 0); }

	auto fill_times_ri() const {
		const auto r = p.x + s.x / 2;
		const auto ar = ap.x + as.x / 2;
		return int((r - ar) / as.x);
	}
};

template <class P, class S, class M>
class cascade_aligner {
public:
	using node_type = cascade_aligner_node<P, S, M>;
	using PV = std::vector<node_type>;

private:
	PV written;
public:

	cascade_aligner(const node_type& n) {
		written = { n };
	}

	auto& top() {
		return written.back();
	}

	auto& meta() {
		return top().meta;
	}

	auto* operator->() {
		return &top();
	}

	auto& create() {
		top().create();
		return *this;
	}

	auto& create_pop() {
		top().create();
		return pop();
	}

	void create_all() {
		for (auto& w : written) {
			w.create();
		}
	}

	auto clone() const {
		return *this;
	}

	auto& pop(int n = 1) {
		while (n--) {
			written.pop_back();
		}

		return *this;
	}

	auto& fill_ri(int limit = 0) {
		auto times = top().fill_times_ri() + limit;
		while (times-- > 0) { dup().nr(); }
		return *this;
	}

	template <class... Args>
	auto& stretch_r(Args&&... args) { top().stretch_r(std::forward<Args>(args)...); return *this; }

	auto& nr() { top().nr(); return *this; }
	auto& nl() { top().nl(); return *this; }
	auto& nd() { top().nd(); return *this; }
	auto& nu() { top().nu(); return *this; }

	auto& bo() { top().bo(); return *this; }
	auto& bi() { top().bi(); return *this; }
	auto& ro() { top().ro(); return *this; }
	auto& ri() { top().ri(); return *this; }
	auto& to() { top().to(); return *this; }
	auto& ti() { top().ti(); return *this; }
	auto& lo() { top().lo(); return *this; }
	auto& li() { top().li(); return *this; }

	auto& flip_h() {
		auto& f = meta().flip.horizontally;
		f = !f;
		return *this;
	}

	auto& flip_v() {
		auto& f = meta().flip.vertically;
		f = !f;
		return *this;
	}

	auto& dup() {
		written.emplace_back(written.back());
		return *this;
	}

	template <class... Args>
	auto& next(Args&&... args) {
		auto cloned_meta = meta().next(std::forward<Args>(args)...);
		const auto top_ap = top().ap;
		const auto top_as = top().as;

		written.emplace_back(
			top_ap,
			top_as,
			std::move(cloned_meta)
		);

		return *this;
	}
};

template <class P, class S, class M>
auto make_cascade_aligner(const P p, const S s, const M meta) {
	using C = cascade_aligner<P, S, M>;
	using N = typename C::node_type;

	return cascade_aligner<P, S, M>(N(p, s, meta));
}
