#include "../augmentations.h"

#include "../utility/stream.h"
#include "../window_framework/window.h"
#include "../config/config.h"
#include "../texture_baker/texture_baker.h"

#include <cassert>
#include <gl\GL.h>
#include <random>

using namespace std;

int main() {
	augmentations::global_log.open(L"logfile.txt");
	augmentations::init();

	using namespace augmentations;
	
	config::input_file cfg("window_config.txt");

	window::glwindow gl;
	gl.create(cfg, rects::wh(100, 100), window::glwindow::RESIZABLE);
	
	gl.set_minimum_size(rects::wh(10, 10));
	gl.set_maximum_size(rects::wh(1680, 1050));

	window::event::message msg;

	bool quit = false;
	gl.set_show(gl.SHOW);
	unsigned id = 0, in = 0;

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, viewport[2], viewport[3], 0, 0, 1 );
	glMatrixMode(GL_MODELVIEW);
	glDisable( GL_DEPTH_TEST ); 
	
	default_random_engine generator;
	uniform_int_distribution<int> color_distribution;
	uniform_int_distribution<int> size_distribution;
	color_distribution = uniform_int_distribution<int>(0, 200);
	size_distribution = uniform_int_distribution<int>(3, 27);

	const int MAX_SIDE = 100;
	const int RECTS = 3000;

	rects::xywhf debug_case [] = { rects::xywhf(0,0,70, 67), rects::xywhf(0,0,76, 47), rects::xywhf(0,0,78, 59) };

	vector<rects::xywhf*> rs(RECTS);
	
	for(int i = 0; i < RECTS; ++i)
		rs[i] = new rects::xywhf;

	unsigned char cols[RECTS][3];
	for(int i = 0; i < RECTS; ++i) {
		cols[i][0] = color_distribution(generator);
		cols[i][1] = color_distribution(generator);
		cols[i][2] = color_distribution(generator);
	}

	std::vector<packing::bin> bins;

	gl.resize = [](window::glwindow& gl) {
		gl.current();
		glViewport(0, 0, gl.get_window_rect().w, gl.get_window_rect().h);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, gl.get_window_rect().w, gl.get_window_rect().h, 0, 0, 1);
		glMatrixMode(GL_MODELVIEW);
	};

	util::fpstimer fps;

	while(true) {
		if(gl.poll_events(msg)) {
			using namespace window::event;

			if(msg == close) {
				break;
			}

			if(msg == window::event::key::down) {
				if(gl.events.key == keys::ESC) break;
				if(gl.events.key == keys::G) {
					for(int i = 0; i < RECTS; ++i) 
						*rs[i] = rects::xywhf(0, 0, size_distribution(generator),  size_distribution(generator));
/*
					for(int i = 0; i < sizeof(debug_case)/sizeof(rects::xywhf); ++i)
								*rs[i] = debug_case[i];*/
					bins.clear();
					rect2D(rs.data(), RECTS, MAX_SIDE, bins);
				}
			}
		}

		if(fps.render()) { 
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			gl.current();
			glClearColor(0.0f, 0.0f, 0.0f, 0.7f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();

			vec2<int> pos;
			for(int i = 0; i < bins.size(); ++i) {
				if(pos.x + bins[i].size.w >= gl.get_window_rect().w) {
					pos.y += MAX_SIDE+10;
					pos.x = 0;
				}

				glColor4ub(255, 255, 255, 255);
				glBegin(GL_QUADS);
				glVertex2i(pos.x, pos.y);
				glVertex2i(pos.x+bins[i].size.w, pos.y);
				glVertex2i(pos.x+bins[i].size.w, pos.y+bins[i].size.h);
				glVertex2i(pos.x, pos.y+bins[i].size.h);
				glEnd();
				
				rects::xywhf* r;
				for(int j = 0; j < bins[i].rects.size(); ++j) {
					r = bins[i].rects[j];
					glColor4ub(cols[j][0], cols[j][1], 0, 255);
					
					glBegin( GL_QUADS );
					glTexCoord2f(0,0);
					glVertex2f(pos.x+r->x, pos.y+r->y);
					glTexCoord2f(1,0);
					glVertex2f(pos.x+r->x + r->w, pos.y+r->y);
					glTexCoord2f(1,1);
					glVertex2f(pos.x+r->x + r->w, pos.y+r->y + r->h);
					glTexCoord2f(0,1);
					glVertex2f(pos.x+r->x, pos.y+r->y + r->h);
					glEnd();
				}
				pos.x+=bins[i].size.w+10;

			}

			if(!gl.swap_buffers()) break;

			gl.set_caption(util::wstr(fps.get_frame_rate()).c_str());
			fps.reset();
		}
		fps.loop();
	}

	augmentations::deinit();

	return 0;
}



