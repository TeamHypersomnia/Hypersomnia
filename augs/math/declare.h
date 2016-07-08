#pragma once
template <class type> struct vec2t;

namespace augs {
	namespace rects {
		template <class type> struct ltrb;
		template <class type> struct xywh;
		template <class type> struct wh;
	}
}

typedef vec2t<int> vec2i;
typedef vec2t<float> vec2;
typedef vec2t<double> vec2d;

typedef augs::rects::ltrb<int> ltrbi;
typedef augs::rects::ltrb<float> ltrb;
typedef augs::rects::ltrb<double> ltrbd;

typedef augs::rects::xywh<int> xywhi;
typedef augs::rects::xywh<float> xywh;
typedef augs::rects::xywh<double> xywhd;