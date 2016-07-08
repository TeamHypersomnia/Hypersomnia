#pragma once
namespace augs {
	template <class type = float> struct vec2t;
	namespace rects {
		template <class type> struct ltrb;
		template <class type> struct xywh;
		template <class type> struct wh;
	}
}

typedef augs::vec2t<int> vec2i;
typedef augs::vec2t<float> vec2;
typedef augs::vec2t<double> vec2d;

typedef augs::rects::ltrb<int> ltrbi;
typedef augs::rects::ltrb<float> ltrb;
typedef augs::rects::ltrb<double> ltrbd;

typedef augs::rects::xywh<int> xywhi;
typedef augs::rects::xywh<float> xywh;
typedef augs::rects::xywh<double> xywhd;