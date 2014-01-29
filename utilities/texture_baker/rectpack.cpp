#pragma once
#include "rectpack.h"
#include <vector>
#include <algorithm>
#include <array>

using namespace std;

namespace augmentations {
	namespace packing {
		struct node {
			struct child_node {
				node* ptr;
				bool in_use;

				child_node() : in_use(false), ptr(nullptr) {}

				void set(int l, int t, int r, int b) {
					if(!ptr) ptr = new node(rects::ltrb(l, t, r, b));
					else {
						ptr->rc = rects::ltrb(l, t, r, b);
						ptr->dividable = true;
					}
					in_use = true;
				}
			};

			child_node child[2];
			rects::ltrb rc;
			bool dividable;
			node(rects::ltrb rc = rects::ltrb()) : dividable(true), rc(rc) {}

			void reset(const rects::wh& r) {
				dividable = true;
				rc = rects::ltrb(0, 0, r.w, r.h);
				delcheck();
			}

			node* insert(rects::xywhf& img) {
				if(child[0].ptr && child[0].in_use) {
					node* newn;
					if(newn = child[0].ptr->insert(img)) return newn;
					return    child[1].ptr->insert(img);
				}

				if(!dividable) return 0;
				rects::wh::fit_status f = img.fits(rects::xywh(rc));

				switch(f) {
				case rects::wh::fit_status::DOESNT_FIT: return 0;
				case rects::wh::fit_status::FITS_INSIDE:			    img.flipped = false; break;
				case rects::wh::fit_status::FITS_INSIDE_FLIPPED:		img.flipped = true; break;
				case rects::wh::fit_status::FITS_PERFECTLY:			dividable = false; img.flipped = false; return this;
				case rects::wh::fit_status::FITS_PERFECTLY_FLIPPED:	dividable = false; img.flipped = true;  return this;
				}

				int iw = (img.flipped ? img.h : img.w), 
					ih = (img.flipped ? img.w : img.h);

				if(rc.w() - iw > rc.h() - ih) {
					child[0].set(rc.l, rc.t, rc.l+iw, rc.b);
					child[1].set(rc.l+iw, rc.t, rc.r, rc.b);
				}
				else {
					child[0].set(rc.l, rc.t, rc.r, rc.t + ih);
					child[1].set(rc.l, rc.t + ih, rc.r, rc.b);
				}

				return child[0].ptr->insert(img);
			}

			void delcheck() {
				if(child[0].ptr) { child[0].in_use = false; child[0].ptr->delcheck(); }
				if(child[1].ptr) { child[1].in_use = false; child[1].ptr->delcheck(); }
			}

			~node() {
				if(child[0].ptr) delete child[0].ptr;
				if(child[1].ptr) delete child[1].ptr;
			}
		};

		rects::wh _rect2D(rects::xywhf* const * v, int n, int max_s, vector<rects::xywhf*>* succ, vector<rects::xywhf*>* unsucc) {
			node root;

			enum {
				SQUARE = 0,
				HEIGHT = 1,
				WIDTH = 2
			};
			int pass = SQUARE;

			// just add a function to execute more heuristics
			const int FUNCS = 5;
			array<bool(*)(rects::xywhf*, rects::xywhf*), FUNCS> functions = {										
				[](rects::xywhf* a, rects::xywhf* b){ return a->area() > b->area(); },							/* area */
				[](rects::xywhf* a, rects::xywhf* b){ return a->perimeter() > b->perimeter(); },				/* perimeter */
				[](rects::xywhf* a, rects::xywhf* b){ return std::max(a->w, a->h) > std::max(b->w, b->h); },	/* maximum side */
				[](rects::xywhf* a, rects::xywhf* b){ return a->w > b->w; },									/* maximum width */
				[](rects::xywhf* a, rects::xywhf* b){ return a->h > b->h; }											/* maximum height */
			};

			vector<rects::xywhf*> order[FUNCS];

			for(size_t f = 0; f < functions.size(); ++f) { 
				order[f] = vector<rects::xywhf*>(v, v+n);
				sort(order[f].begin(), order[f].end(), functions[f]);
			}

			rects::wh min_bin = rects::wh(max_s, max_s);
			int min_func = -1, best_func = 0, best_area = 0, _area = 0, step, step_multiplier, i;

			bool fail = false;

			for(size_t f = 0; f < functions.size(); ++f) {
				v = order[f].data();
				step = min_bin.w / 2;
				root.reset(min_bin);
				pass = SQUARE;

				while(true) {
					/* if we expanded our root too much, we've failed, it's time to exit */
					if(root.rc.area() > min_bin.area() || root.rc.max_side() > max_s) {
						if(min_func > -1) break;
						_area = 0;

						root.reset(min_bin);
						for(i = 0; i < n; ++i)
							if(root.insert(*v[i]))
								_area += v[i]->area();

						fail = true;
						break;
					}

					/* we assume we're going to decrease root's size */
					step_multiplier = -1;

					for(i = 0; i < n; ++i) {
						if(!root.insert(*v[i])) {
							/* our assumption was wrong - one of the rectangles could not fit */
							step_multiplier = 1;
							break;
						}
					}

					if(step_multiplier == -1 && step <= 64) {
						++pass;
						if(pass > WIDTH) break;
						step = (pass == WIDTH) ? root.rc.w() : root.rc.h();
						step /= 2;
					}

					root.reset(rects::wh(
						root.rc.w() + ((pass == SQUARE || pass == WIDTH  )? step_multiplier*step : 0), 
						root.rc.h() + ((pass == SQUARE || pass == HEIGHT )? step_multiplier*step : 0)
						));

					step /= 2;
					if(!step) 
						step = 1;
				}

				if(!fail && (min_bin.area() >= root.rc.area())) {
					min_bin = rects::wh(root.rc);
					min_func = f;
				}

				else if(fail && (_area > best_area)) {
					best_area = _area;
					best_func = f;
				}
				fail = false;
			}

			v = order[min_func == -1 ? best_func : min_func].data();

			float clip_x = 0.f, clip_y = 0.f;
			node* ret;

			root.reset(min_bin);

			for(i = 0; i < n; ++i) {
				if(ret = root.insert(*v[i])) {
					v[i]->x = ret->rc.l;
					v[i]->y = ret->rc.t;

					if(v[i]->flipped) {
						v[i]->flipped = false;
						v[i]->flip();
					}

					clip_x = std::max(clip_x, ret->rc.r);
					clip_y = std::max(clip_y, ret->rc.b); 

					if(succ) succ->push_back(v[i]);
				}
				else {
					if(unsucc) unsucc->push_back(v[i]);
					else return rects::wh(-1, -1);

					v[i]->flipped = false;
				}
			}

			return rects::wh(clip_x, clip_y);
		}


		int rect2D(rects::xywhf* const * v, int n, int max_s, bin& out_bin) {
			out_bin.size = rects::wh(max_s, max_s);

			for(int i = 0; i < n; ++i) 
				if(v[i]->fits(out_bin.size) == rects::wh::fit_status::DOESNT_FIT) return 2;

			out_bin.size = _rect2D(v, n, max_s);

			if(out_bin.size.w == -1) return 1;
			return 0;
		}

		int rect2D(rects::xywhf* const * v, int n, int max_s, std::vector<bin>& bins) {
			return rect2D(v, n, max_s, bins, -1);
		}

		int rect2D(rects::xywhf* const * v, int n, int max_s, vector<bin>& bins, int max_bins) {
			rects::wh bin_rc(max_s, max_s);

			for(int i = 0; i < n; ++i) 
				if(v[i]->fits(bin_rc) == rects::wh::fit_status::DOESNT_FIT) return 2;

			vector<rects::xywhf*> vec[2], *p[2] = { vec, vec+1 };
			vec[0].resize(n);
			vec[1].clear();
			memcpy(&vec[0][0], v, sizeof(rects::xywhf*)*n);

			bin* b = 0;

			while(true) {
				bins.push_back(bin());
				b = &bins[bins.size()-1];

				b->size = _rect2D(&((*p[0])[0]), p[0]->size(), max_s, &b->rects, p[1]);
				b->rects.shrink_to_fit();

				p[0]->clear();

				if(!p[1]->size()) break;
				else if(max_bins > -1 && int(bins.size()) >= max_bins) 
					return 1; 

				std::swap(p[0], p[1]);
			}

			return 0;
		}

	}
}