#pragma once

using real32 = float;
using real64 = double;

template <class type> struct basic_vec2;
template <class type> struct basic_ltrb;
template <class type> struct basic_xywh;
template <class type> struct basic_transform;

using vec2i = basic_vec2<int>;
using vec2u = basic_vec2<unsigned>;
using vec2 = basic_vec2<real32>;
using vec2d = basic_vec2<real64>;

using ltrbi = basic_ltrb<int>;
using ltrbu = basic_ltrb<unsigned>;
using ltrb = basic_ltrb<float>;
using ltrbd = basic_ltrb<double>;

using xywhi = basic_xywh<int>;
using xywhu = basic_xywh<unsigned>;
using xywh = basic_xywh<real32>;
using xywhd = basic_xywh<real64>;

using transformr = basic_transform<real32>;
using transformi = basic_transform<int>;