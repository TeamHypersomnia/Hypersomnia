#pragma once
template <class type> struct vec2t;
template <class type> struct ltrbt;
template <class type> struct xywht;

typedef vec2t<int> vec2i;
typedef vec2t<unsigned> vec2u;
typedef vec2t<float> vec2;
typedef vec2t<double> vec2d;

typedef ltrbt<int> ltrbi;
typedef ltrbt<unsigned> ltrbu;
typedef ltrbt<float> ltrb;
typedef ltrbt<double> ltrbd;

typedef xywht<int> xywhi;
typedef xywht<unsigned> xywhu;
typedef xywht<float> xywh;
typedef xywht<double> xywhd;