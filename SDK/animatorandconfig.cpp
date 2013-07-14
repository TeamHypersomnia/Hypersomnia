//#define include_DWM
#include <gl/glew.h>

#include "../augmentations.h"

#include "../config/config.h"
#include "../window_framework/window.h"
#include "../utility/value_animator.h"


int main() {
	augmentations::init();
	using namespace augmentations;
	
	config::input_file cfg("window_config.txt");

	window::glwindow gl;
	gl.create(cfg, rects::wh(100, 100));
	
	//gl.set_minimum_size(rects::wh(cfg[L"min_resw"], cfg[L"min_resh"]));
	//gl.set_maximum_size(rects::wh(cfg[L"max_resw"], cfg[L"max_resh"]));

	window::event::message msg;

	bool quit = false;
	gl.set_show(gl.SHOW);

	util::animator fadein ([](float val){
		glClearColor(val, val, val, val);
	},
		0.f, 1.f, 500, util::animator::EXPONENTIAL);

	util::animator mover ([&](float val){ 
		rects::xywh rc = gl.get_window_rect();
		rc.x = val;
		gl.set_window_rect(rc); }, 400.0f, 30.0f, 500, util::animator::EXPONENTIAL);

	util::animation anim(1), anim2(1);
	anim.animators.push_back(&fadein);
	anim2.animators.push_back(&mover);
	
	anim.start();
	anim2.start();

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	
	cfg[L"resolution_w"].int_val -= 100;
	cfg.save("window_config.txt");

	while(!quit) {
		while(gl.poll_events(msg)) {
			if(msg == window::event::close) {
				quit = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		anim.animate();
		anim2.animate();

		gl.swap_buffers();
	}
	
	augmentations::deinit();
	return 0;
}