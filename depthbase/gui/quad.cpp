#pragma once
#include "quad.h"

namespace augs {
	namespace graphics {
		namespace gui {
			void scale_virtual_res(rects::wh<float> vres, rects::wh<float> display, vector<quad>& quads) {
				if(vres == display) return;

				float x_mult = display.w/float(vres.w);
				float y_mult = display.h/float(vres.h);

				for(size_t i = 0; i < quads.size(); ++i) {
					for(int q = 0; q < 4; ++q) {
						quads[i].p[q].x = int(float(quads[i].p[q].x)*x_mult);
						quads[i].p[q].y = int(float(quads[i].p[q].y)*y_mult);
					}
				}
			}

			material::material(augs::texture_baker::texture* tex, const pixel_32& color) : tex(tex), color(color) {}

			material::material(const pixel_32& color) : tex(gui::null_texture), color(color) {}

			quad::quad() {}
			quad::quad(const rects::ltrb<float>& rc, const material& mat, const rect_texture& t) {
				p[0].x = p[3].x = rc.l;
				p[0].y = p[1].y = rc.t;
				p[1].x = p[2].x = rc.r;
				p[2].y = p[3].y = rc.b;

				p[0].col = p[1].col = p[2].col = p[3].col = mat.color;

				mat.tex->get_uv(t.u1, t.v1, p[0].u, p[0].v);
				mat.tex->get_uv(t.u2, t.v1, p[1].u, p[1].v);
				mat.tex->get_uv(t.u2, t.v2, p[2].u, p[2].v);
				mat.tex->get_uv(t.u1, t.v2, p[3].u, p[3].v);
			}

			quad quad::clipped(const rects::ltrb<float>& origin, const rects::ltrb<float>& c, const material& mat) {
				rects::ltrb<float> rc = origin;
				rc.clip(c);

				static rect_texture diff;
				static float tw, th;
				static quad q;

				tw = 1.0f / origin.w();
				th = 1.0f / origin.h();

				diff = rect_texture(((q.p[0].x = q.p[3].x = rc.l) - origin.l) * tw,
					((q.p[0].y = q.p[1].y = rc.t) - origin.t) * th,
					((q.p[1].x = q.p[2].x = rc.r) - origin.r) * tw + 1.0f,
					((q.p[2].y = q.p[3].y = rc.b) - origin.b) * th + 1.0f);

				q.p[0].col = q.p[1].col = q.p[2].col = q.p[3].col = mat.color;

				mat.tex->get_uv(diff.u1, diff.v1, q.p[0].u,  q.p[0].v);
				mat.tex->get_uv(diff.u2, diff.v1, q.p[1].u,  q.p[1].v);
				mat.tex->get_uv(diff.u2, diff.v2, q.p[2].u,  q.p[2].v);
				mat.tex->get_uv(diff.u1, diff.v2, q.p[3].u,  q.p[3].v);

				return q;
			}

			rects::ltrb<float> gui::add_quad(const material& mat, const rects::ltrb<float>& origin, const rects::ltrb<float>* p, std::vector<quad>& v) {
				rects::ltrb<float> rc = origin;
				if((p && !rc.clip(*p)) || !rc.good()) return rc;

				quad q;

				float tw = 1.f / origin.w();
				float th = 1.f / origin.h();

				rect_texture diff  (((q.p[0].x = q.p[3].x = rc.l) - origin.l) * tw,
					((q.p[0].y = q.p[1].y = rc.t) - origin.t) * th,
					((q.p[1].x = q.p[2].x = rc.r) - origin.r) * tw + 1.0f,
					((q.p[2].y = q.p[3].y = rc.b) - origin.b) * th + 1.0f);

				q.p[0].col = q.p[1].col = q.p[2].col = q.p[3].col = mat.color;

				mat.tex->get_uv(diff.u1, diff.v1, q.p[0].u,  q.p[0].v);
				mat.tex->get_uv(diff.u2, diff.v1, q.p[1].u,  q.p[1].v);
				mat.tex->get_uv(diff.u2, diff.v2, q.p[2].u,  q.p[2].v);
				mat.tex->get_uv(diff.u1, diff.v2, q.p[3].u,  q.p[3].v);

				v.push_back(q);
				return rc;
			}

			void quad::move(const vec2<int>& v) {
				for(int i = 0; i < 4; ++i) {
					p[i].x += v.x;
					p[i].y += v.y;
				}
			}

			void quad::set(const rects::ltrb<float>& rc) {
				p[0].x = p[3].x = rc.l;
				p[0].y = p[1].y = rc.t;
				p[1].x = p[2].x = rc.r;
				p[2].y = p[3].y = rc.b;
			}

			void quad::rotate90(int times) {
				for(int i = 0; i < times; ++i) {
					auto tmp = p[3];
					p[3].u = p[2].u;
					p[3].v = p[2].v;
					p[2].u = p[1].u;
					p[2].v = p[1].v;
					p[1].u = p[0].u;
					p[1].v = p[0].v;
					p[0].u = tmp.u;
					p[0].v = tmp.v;
				}
			}

			rects::ltrb<float> quad::get_rect() const {
				return rects::ltrb<float>(p[0].x, p[0].y, p[2].x, p[2].y);
			}
		}
	}
}