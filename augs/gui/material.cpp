#pragma once
#include "material.h"
#include "rect.h"
#include "texture_baker/texture_baker.h"

namespace augs {
	namespace gui {
		material::material(assets::texture_id tex, const rgba& color) : tex(tex), color(color) {}

		material::material(const rgba& color) : tex(assets::texture_id::BLANK), color(color) {}

		rects::ltrb<float> draw_clipped_rectangle(material mat, rects::ltrb<float> origin, rect_id p, std::vector<augs::vertex_triangle>& v) {
			/* if p is null, we don't clip at all
			if p is not null and p->clip is true, we take p->rc_clipped as clipper
			if p is not null and p->clip is false, we search for the first clipping parent's rc_clipped
			*/

			return draw_clipped_rectangle(mat, origin, &p->get_clipping_rect(), v);
		}

		rects::ltrb<float> draw_clipped_rectangle(material mat, rects::ltrb<float> origin, const rects::ltrb<float>* clipper, std::vector<augs::vertex_triangle>& v) {
			return draw_clipped_rectangle(*mat.tex, mat.color, origin, clipper, v);
		}

		rects::ltrb<float> draw_clipped_rectangle(augs::texture& tex, rgba color, rects::ltrb<float> origin, const rects::ltrb<float>* parent, std::vector<augs::vertex_triangle>& v) {
			rects::ltrb<float> rc = origin;
			if ((parent && !rc.clip_by(*parent)) || !rc.good()) return rc;

			augs::vertex p[4];

			float tw = 1.f / origin.w();
			float th = 1.f / origin.h();

			rects::texture<float> diff(((p[0].pos.x = p[3].pos.x = rc.l) - origin.l) * tw,
				((p[0].pos.y = p[1].pos.y = rc.t) - origin.t) * th,
				((p[1].pos.x = p[2].pos.x = rc.r) - origin.r) * tw + 1.0f,
				((p[2].pos.y = p[3].pos.y = rc.b) - origin.b) * th + 1.0f);

			p[0].color = p[1].color = p[2].color = p[3].color = color;

			tex.get_uv(diff.u1, diff.v1, p[0].texcoord.x, p[0].texcoord.y);
			tex.get_uv(diff.u2, diff.v1, p[1].texcoord.x, p[1].texcoord.y);
			tex.get_uv(diff.u2, diff.v2, p[2].texcoord.x, p[2].texcoord.y);
			tex.get_uv(diff.u1, diff.v2, p[3].texcoord.x, p[3].texcoord.y);

			augs::vertex_triangle out[2];
			out[0].vertices[0] = p[0];
			out[0].vertices[1] = p[1];
			out[0].vertices[2] = p[2];

			out[1].vertices[0] = p[2];
			out[1].vertices[1] = p[3];
			out[1].vertices[2] = p[0];

			v.push_back(out[0]);
			v.push_back(out[1]);
			return rc;
		}
	}
}