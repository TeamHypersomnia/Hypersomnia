//#define include_DWM
#include "db.h"

int main() {
	using namespace db;
	using namespace math;
	using namespace window;

	config cfg("window_config.txt");

	glwindow::init();
	glwindow gl;
	gl.create(cfg, rect_wh(100, 100), 0);
	
	gl.set_minimum_size(rect_wh(cfg[L"min_resw"], cfg[L"min_resh"]));
	gl.set_maximum_size(rect_wh(cfg[L"max_resw"], cfg[L"max_resh"]));

	event::message msg;

	bool quit = false;
	gl.set_show(gl.SHOW);
	

	misc::animator fadein ([](float val){
		glClearColor(val, val, val, val);
	},
		0.f, 1.f, 500, misc::animator::EXPONENTIAL);

	misc::animator mover ([&](float val){ 
		rect_xywh rc = gl.get_window_rect();
		rc.x = val;
		gl.set_window_rect(rc); }, 400.0f, 0.0f, 500, misc::animator::EXPONENTIAL);

	misc::animation anim(1), anim2(1);
	anim.animators.push_back(&fadein);
	anim2.animators.push_back(&mover);
	
	anim.start();
	anim2.start();

	db::graphics::init();
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	
	cfg[L"resolution_w"].int_val -= 100;
	cfg.save("window_config.txt");

	while(!quit) {
		while(gl.poll_events(msg)) {
			if(msg == event::close) {
				quit = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		anim.animate();
		anim2.animate();

		gl.swap_buffers();
	}
	
	return 0;
}