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

	cascade_aligner_node() = default;
	cascade_aligner_node(const cascade_aligner_node&) = default;

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

	auto fill_times_ti() const {
		const auto t = p.y - s.y / 2;
		const auto at = ap.y - as.y / 2;
		return int((at - t) / as.y);
	}
};

template <class P, class S, class M>
class cascade_aligner {
public:
	using node_type = cascade_aligner_node<P, S, M>;
	using PV = std::vector<node_type>;

private:
	PV written;
	node_type last_emplacement;
public:

	cascade_aligner(const node_type& n) {
		last_emplacement = n;
		written = { n };
	}

	~cascade_aligner() {
		create_all();
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

	auto& fill_ti(int limit = 0) {
		auto times = top().fill_times_ti() + limit;
		while (times-- > 0) { dup().nu(); }
		return *this;
	}

	auto& rot_90() {
		top().as.flip();

		auto& r = meta().rotation;

		if (r == 0.f) {
			r = 90.f;
		}
		else if (r == 90.f) {
			r = 180.f;
		}
		else if (r == 180.f) {
			r = 270.f;
		}
		else if (r == 270.f) {
			r = 0.f;
		}

		return *this;
	}

	auto& rot_minus_90() {
		rot_90();
		rot_90();
		rot_90();

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

	template <class X, class Y>
	auto& mv(X&& x, Y&& y) { 
		top().ap.x += std::forward<X>(x);
		top().ap.y += std::forward<Y>(y);

		return *this;
	}

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

	auto& again() {
		written.emplace_back(last_emplacement);
		return *this;
	}

	template <class... Args>
	auto& again(Args&&... args) {
		auto cloned_meta = meta().next(std::forward<Args>(args)...);
		const auto top_p = top().p;
		const auto top_s = top().s;

		written.emplace_back(
			top_p,
			top_s,
			std::move(cloned_meta)
		);

		return *this;
	}

	template <class... Args>
	auto& next(Args&&... args) {
		auto cloned_meta = meta().next(std::forward<Args>(args)...);
		const auto top_ap = top().ap;
		const auto top_as = top().as;

		last_emplacement = node_type(
			top_ap,
			top_as,
			std::move(cloned_meta)
		);

		written.emplace_back(last_emplacement);

		return *this;
	}
};

template <class P, class S, class M>
auto make_cascade_aligner(const P p, const S s, const M meta) {
	using C = cascade_aligner<P, S, M>;
	using N = typename C::node_type;

	return cascade_aligner<P, S, M>(N(p, s, meta));
}
